#include <ROOT/RNTupleImporter.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriteOptions.hxx>

#include <Compression.h>
#include <TFile.h>
#include <TTree.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

namespace fs = std::filesystem;

struct Options {
  std::string input;
  std::string output;
  std::string treeName = "Events";
  std::string ntupleName = "Events";
  std::uint64_t maxEntries = std::numeric_limits<std::uint64_t>::max();
  int compressionLevel = 5;
  std::size_t clusterSize = 128ULL * 1024 * 1024;
  std::size_t pageBufferBudget = 256ULL * 1024 * 1024;
  bool force = false;
};

void PrintUsage(std::ostream &stream, const char *program) {
  stream << "usage: " << program << " INPUT.root OUTPUT.rntuple.root [options]\n"
      << "  --tree NAME                 input TTree name (default: Events)\n"
      << "  --ntuple NAME              output RNTuple name (default: Events)\n"
      << "  --max-entries N            import a prefix for smoke/tuning runs\n"
      << "  --compression-level N      zstd level, 1..19 (default: 5)\n"
      << "  --cluster-size-mib N       target zipped cluster size (default: 128)\n"
      << "  --page-buffer-mib N        write page-buffer budget (default: 256)\n"
      << "  --force                    replace an existing output atomically\n"
      << "  -h, --help                 show this help and exit\n";
}

[[noreturn]] void Usage(const char *program, const std::string &message = {}) {
  if (!message.empty())
    std::cerr << "error: " << message << "\n\n";
  PrintUsage(std::cerr, program);
  throw std::runtime_error("invalid command line");
}

std::uint64_t ParseUnsigned(std::string_view text, std::string_view option) {
  std::size_t consumed = 0;
  std::uint64_t value = 0;
  try {
    value = std::stoull(std::string(text), &consumed);
  } catch (const std::exception &) {
    throw std::runtime_error("invalid value for " + std::string(option) +
                             ": " + std::string(text));
  }
  if (consumed != text.size())
    throw std::runtime_error("invalid value for " + std::string(option) +
                             ": " + std::string(text));
  return value;
}

Options ParseOptions(int argc, char **argv) {
  if (argc == 2 &&
      (std::string_view(argv[1]) == "-h" ||
       std::string_view(argv[1]) == "--help")) {
    PrintUsage(std::cout, argv[0]);
    std::exit(EXIT_SUCCESS);
  }
  if (argc < 3)
    Usage(argv[0]);
  Options options;
  options.input = argv[1];
  options.output = argv[2];
  for (int i = 3; i < argc; ++i) {
    const std::string arg = argv[i];
    auto take = [&](std::string_view option) -> std::string {
      if (++i >= argc)
        Usage(argv[0], "missing value for " + std::string(option));
      return argv[i];
    };
    if (arg == "--tree")
      options.treeName = take(arg);
    else if (arg == "--ntuple")
      options.ntupleName = take(arg);
    else if (arg == "--max-entries")
      options.maxEntries = ParseUnsigned(take(arg), arg);
    else if (arg == "--compression-level")
      options.compressionLevel = static_cast<int>(ParseUnsigned(take(arg), arg));
    else if (arg == "--cluster-size-mib")
      options.clusterSize = ParseUnsigned(take(arg), arg) * 1024ULL * 1024ULL;
    else if (arg == "--page-buffer-mib")
      options.pageBufferBudget = ParseUnsigned(take(arg), arg) * 1024ULL * 1024ULL;
    else if (arg == "--force")
      options.force = true;
    else
      Usage(argv[0], "unknown option " + arg);
  }
  if (options.input.empty() || options.output.empty())
    Usage(argv[0], "input and output paths must not be empty");
  if (fs::absolute(options.input).lexically_normal() ==
      fs::absolute(options.output).lexically_normal())
    throw std::runtime_error("input and output paths must differ");
  if (options.compressionLevel < 1 || options.compressionLevel > 19)
    throw std::runtime_error("zstd compression level must be in [1, 19]");
  if (options.clusterSize == 0 || options.pageBufferBudget == 0)
    throw std::runtime_error("cluster and page-buffer sizes must be positive");
  return options;
}

std::uint64_t InputEntries(const Options &options) {
  std::unique_ptr<TFile> file(TFile::Open(options.input.c_str(), "READ"));
  if (!file || file->IsZombie())
    throw std::runtime_error("cannot open input file " + options.input);
  auto *tree = file->Get<TTree>(options.treeName.c_str());
  if (!tree)
    throw std::runtime_error("cannot find TTree '" + options.treeName +
                             "' in " + options.input);
  return static_cast<std::uint64_t>(tree->GetEntries());
}

} // namespace

int main(int argc, char **argv) {
  fs::path partial;
  try {
    const Options options = ParseOptions(argc, argv);
    const auto inputEntries = InputEntries(options);
    const auto expectedEntries =
        std::min(inputEntries, options.maxEntries);

    const fs::path output = fs::absolute(options.output).lexically_normal();
    if (fs::exists(output) && !options.force)
      throw std::runtime_error("output already exists (use --force): " +
                               output.string());
    if (!output.parent_path().empty())
      fs::create_directories(output.parent_path());

    partial = output;
    partial += ".partial";
    if (fs::exists(partial))
      fs::remove(partial);

    std::cout << "[rntuple-convert] input=" << options.input
              << " tree=" << options.treeName
              << " entries=" << inputEntries << '\n'
              << "[rntuple-convert] output=" << output
              << " ntuple=" << options.ntupleName
              << " selected_entries=" << expectedEntries
              << " zstd_level=" << options.compressionLevel
              << " cluster_mib=" << options.clusterSize / (1024 * 1024)
              << " page_buffer_mib="
              << options.pageBufferBudget / (1024 * 1024) << std::endl;

    auto importer = ROOT::Experimental::RNTupleImporter::Create(
        options.input, options.treeName, partial.string());
    importer->SetNTupleName(options.ntupleName);
    importer->SetIsQuiet(true);
    if (options.maxEntries != std::numeric_limits<std::uint64_t>::max())
      importer->SetMaxEntries(options.maxEntries);
    auto writeOptions = importer->GetWriteOptions();
    writeOptions.SetCompression(ROOT::RCompressionSetting::EAlgorithm::kZSTD,
                                options.compressionLevel);
    writeOptions.SetApproxZippedClusterSize(options.clusterSize);
    writeOptions.SetPageBufferBudget(options.pageBufferBudget);
    writeOptions.SetUseBufferedWrite(true);
    writeOptions.SetEnablePageChecksums(true);
    importer->SetWriteOptions(writeOptions);

    const auto start = std::chrono::steady_clock::now();
    importer->Import();
    importer.reset();

    auto reader = ROOT::RNTupleReader::Open(options.ntupleName, partial.string());
    const auto outputEntries = static_cast<std::uint64_t>(reader->GetNEntries());
    const auto fieldCount = reader->GetDescriptor().GetNFields();
    reader.reset();
    if (outputEntries != expectedEntries)
      throw std::runtime_error("entry-count verification failed: expected " +
                               std::to_string(expectedEntries) + ", got " +
                               std::to_string(outputEntries));

    // On POSIX, rename replaces an existing destination in one atomic step.
    // Keeping the temporary file beside the destination also prevents a
    // cross-filesystem rename from silently degrading this guarantee.
    fs::rename(partial, output);
    partial.clear();

    const auto seconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start).count();
    const auto outputBytes = fs::file_size(output);
    std::cout << std::fixed << std::setprecision(3)
              << "[rntuple-convert] verified entries=" << outputEntries
              << " fields=" << fieldCount
              << " bytes=" << outputBytes
              << " seconds=" << seconds
              << " events_per_second="
              << (seconds > 0. ? outputEntries / seconds : 0.) << '\n';
    return 0;
  } catch (const std::exception &error) {
    if (!partial.empty()) {
      std::error_code ignored;
      fs::remove(partial, ignored);
    }
    std::cerr << "sknano-convert-rntuple: " << error.what() << '\n';
    return 1;
  }
}
