//
// Created by martin on 27/09/24.
//

#include <benchmark/benchmark.h>
#include <random>
#include <ranges>

#include "sequential_hashmap.h"

template <typename HT>
void fill_map_random(HT& map, const std::vector<size_t>& sequence) {
  for (const size_t i :
       std::ranges::iota_view(0, static_cast<int>(sequence.size()))) {
    map.emplace(sequence[i], sequence[i]);
  }
}
template <typename HT>
static void bm_insert(benchmark::State& state) {
  size_t values = state.range(0);
  std::default_random_engine e1(0);
  std::uniform_int_distribution<size_t> uniform_dist(0, 1000000);
  std::vector<size_t> v(values);
  std::ranges::generate(v, [&]() { return uniform_dist(e1); });
  for (auto _ : state) {
    HT map{};
    fill_map_random(map, v);
  }
}

template <typename HT>
static void bm_access(benchmark::State& state) {
  size_t values = state.range(0);
  std::default_random_engine e1(0);
  std::uniform_int_distribution<size_t> uniform_dist(0, 1000000);
  std::vector<size_t> v(values);
  std::ranges::generate(v, [&]() { return uniform_dist(e1); });
  HT map{};
  fill_map_random(map, v);
  for (auto _ : state) {
    for (auto& val : v) {
      map.at(val);
    }
  }
}

BENCHMARK(bm_insert<sequential_hashmap<size_t, size_t>>)->Arg(1000);
BENCHMARK(bm_insert<std::unordered_map<size_t, size_t>>)->Arg(1000);

BENCHMARK(bm_access<sequential_hashmap<size_t, size_t>>)->Arg(1000);
BENCHMARK(bm_access<std::unordered_map<size_t, size_t>>)->Arg(1000);
BENCHMARK_MAIN();