#ifndef SKNanoCore_DoubleBufferPipeline_h
#define SKNanoCore_DoubleBufferPipeline_h

#include "AnalysisException.h"

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace SKNano {

// One dedicated reader context produces at most two owned blocks. The caller
// processes blocks on its own thread; no read context or backend buffer is
// shared with processing code.
class DoubleBufferPipeline {
public:
    template <typename Task, typename ReaderFactory, typename Read,
              typename Process>
    static void run(std::vector<Task> tasks, ReaderFactory readerFactory,
                    Read read, Process process) {
        std::sort(tasks.begin(), tasks.end(),
                  [](const Task &left, const Task &right) {
                      return left.taskId < right.taskId;
                  });
        for (std::size_t index = 1; index < tasks.size(); ++index)
            if (tasks[index - 1].taskId == tasks[index].taskId)
                throw ConfigError(
                    "[DoubleBufferPipeline] duplicate task ID");
        if (tasks.empty())
            return;

        using Reader = decltype(readerFactory());
        using Block = decltype(read(std::declval<Reader &>(),
                                    std::declval<const Task &>()));
        struct Item {
            Task task;
            Block block;
        };
        std::mutex mutex;
        std::condition_variable readable;
        std::condition_variable writable;
        std::deque<Item> queue;
        bool finished = false;
        bool cancelled = false;
        std::exception_ptr readerError;

        std::thread reader([&] {
            try {
                Reader context = readerFactory();
                for (const auto &task : tasks) {
                    Block block = read(context, task);
                    std::unique_lock<std::mutex> lock(mutex);
                    writable.wait(lock, [&] {
                        return cancelled || queue.size() < 2;
                    });
                    if (cancelled)
                        break;
                    queue.push_back(Item{task, std::move(block)});
                    readable.notify_one();
                }
            } catch (...) {
                readerError = std::current_exception();
            }
            {
                std::lock_guard<std::mutex> lock(mutex);
                finished = true;
            }
            readable.notify_all();
        });

        std::exception_ptr processError;
        while (true) {
            std::unique_lock<std::mutex> lock(mutex);
            readable.wait(lock, [&] { return finished || !queue.empty(); });
            if (queue.empty()) {
                if (finished)
                    break;
                continue;
            }
            Item item = std::move(queue.front());
            queue.pop_front();
            writable.notify_one();
            lock.unlock();
            try {
                process(item.task, item.block);
            } catch (...) {
                processError = std::current_exception();
                {
                    std::lock_guard<std::mutex> cancelLock(mutex);
                    cancelled = true;
                }
                writable.notify_all();
                break;
            }
        }
        reader.join();
        if (processError)
            std::rethrow_exception(processError);
        if (readerError)
            std::rethrow_exception(readerError);
    }
};

} // namespace SKNano

#endif
