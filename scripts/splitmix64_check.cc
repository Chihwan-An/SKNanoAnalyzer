#include <cstdint>
#include <iostream>
#include <cassert>

// ---- rle_bucket 원본 구현 ------------------------------
uint32_t rle_bucket(uint64_t run, uint64_t lumi, uint64_t event,
                    uint32_t nbuckets) {
    auto splitmix64 = [](uint64_t x) {
        uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    };

    uint64_t seed = run * 0x9E3779B97F4A7C15ULL ^ lumi * 0xBF58476D1CE4E5B9ULL ^
                    event * 0x94D049BB133111EBULL;

    uint64_t h = splitmix64(seed);

    __uint128_t prod = (__uint128_t)h * (__uint128_t)nbuckets;
    return (uint32_t)(prod >> 64);
}

uint64_t compute_rle_bucket_checksum(std::size_t n_samples) {
  constexpr uint64_t INIT_ACC = 0x1234567890abcdefULL;

  // 여기: auto 람다로 mix_checksum 정의
  constexpr auto mix_checksum = [](uint64_t acc, uint32_t bucket) -> uint64_t {
    return acc * 0x9e3779b97f4a7c15ULL + bucket;
  };

  uint64_t acc = INIT_ACC;

  for (uint64_t i = 0; i < n_samples; ++i) {
    uint64_t run   = i;
    uint64_t lumi  = i * 0x9e3779b97f4a7c15ULL;
    uint64_t event = i ^ 0xDEADBEEFCAFEBABEULL;

    uint32_t nbuckets = static_cast<uint32_t>((i % 1'000'000ULL) + 1ULL);

    uint32_t b = rle_bucket(run, lumi, event, nbuckets);
    if (b >= nbuckets) {
      std::cerr << "Out-of-range bucket: b=" << b
                << " nbuckets=" << nbuckets << '\n';
      std::abort();
    }

    acc = mix_checksum(acc, b);
  }

  return acc;
}


int main() {
    const std::size_t N = 10'000'000;  // 1천만 샘플 정도 (원하면 더 늘릴 수 있음)
    uint64_t checksum = compute_rle_bucket_checksum(N);

    std::cout << "rle_bucket checksum for N=" << N << " : 0x"
              << std::hex << checksum << std::dec << "\n";

    // ---- 1단계: 처음 한 번만 이 값을 눈으로 보고 기록한다 ----
    // 예를 들어, 처음 실행 결과가
    //   N=10000000 → checksum = 0xDEADBEEFCAFEBABE
    // 라고 나오면, 아래 상수로 박아 둔다.

    const uint64_t GOLDEN = 0x4068d908153fc923;  // <- 처음 한 번 얻은 값으로 업데이트

    if (checksum != GOLDEN) {
        std::cerr << "rle_bucket checksum mismatch!\n"
                  << " expected: 0x" << std::hex << GOLDEN
                  << " got: 0x" << checksum << std::dec << "\n";
        return 1;
    }

    std::cout << "rle_bucket reproducibility test: OK\n";
    return 0;
}