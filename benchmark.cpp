//
// Created by martin on 27/09/24.
//

#include <benchmark/benchmark.h>
#include <random>
#include <ranges>

#include "bytell_hash_map.h"
#include "sequential_hashmap.h"

std::vector<size_t> generate_random_sequence(size_t n) {
  std::vector<size_t> seq(n);
  std::default_random_engine engine(0);
  std::uniform_int_distribution<size_t> uniform_dist(
      0, std::numeric_limits<size_t>::max());
  std::iota(seq.begin(), seq.end(), 0);
  std::ranges::generate(seq, [&]() { return uniform_dist(engine); });
  return seq;
}

template <typename HT>
void fill_map_random(HT& map, const std::vector<size_t>& sequence) {
  for (const size_t i :
       std::ranges::iota_view(0, static_cast<int>(sequence.size()))) {
    map.emplace(sequence[i], sequence[i]);
  }
}
template <typename HT>
static void bm_insert(benchmark::State& state) {
  const size_t num_values = state.range(0);
  auto v{generate_random_sequence(num_values)};
  for (auto _ : state) {
    HT map{};
    fill_map_random(map, v);
    benchmark::DoNotOptimize(map);
  }
}

template <typename HT>
static void bm_access(benchmark::State& state) {
  const size_t num_values = state.range(0);
  auto v{generate_random_sequence(num_values)};
  HT map{};
  fill_map_random(map, v);
  for (auto _ : state) {
    for (auto& val : v) {
      map.at(val);
    }
    benchmark::DoNotOptimize(map);
  }
}

BENCHMARK(bm_insert<sequential_hashmap<size_t, size_t>>)->Arg(1000000);
BENCHMARK(bm_insert<std::unordered_map<size_t, size_t>>)->Arg(1000000);
BENCHMARK(bm_insert<ska::bytell_hash_map<size_t, size_t>>)->Arg(1000000);

BENCHMARK(bm_access<sequential_hashmap<size_t, size_t>>)->Arg(1000000);
BENCHMARK(bm_access<std::unordered_map<size_t, size_t>>)->Arg(1000000);
BENCHMARK(bm_access<ska::bytell_hash_map<size_t, size_t>>)->Arg(1000000);

BENCHMARK_MAIN();