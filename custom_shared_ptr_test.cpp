//
// Created by martin on 27/09/24.
//

#include <benchmark/benchmark.h>
#include <iostream>
#include <random>
#include <ranges>
#include "custom_shared_ptr.h"

struct Test {
  int x;
  Test(int v) : x(v) {}
  ~Test() { x = -1;  std::cout << "call\n";}
};

void test_basic_construction() {
  custom_shared_ptr<int> p(new int(42));
  assert(*p == 42);
}

void test_copy_semantics() {
  custom_shared_ptr<int> p1(new int(100));
  custom_shared_ptr<int> p2 = p1;
  assert(p1 == p2);
  assert(*p2 == 100);
}

void test_reset_and_use_count() {
  custom_shared_ptr<int> p(new int(55));
  assert(p.use_count() == 1);
  p.reset();
  assert(!p);
}

void test_custom_deleter() {
  bool deleted = false;
  {
    custom_shared_ptr<int> p(new int(1), [&](int* ptr) {
      delete ptr;
      deleted = true;
    });
  }
  assert(deleted);
}

struct Tracked {
    static inline bool destroyed = false;
    ~Tracked() { destroyed = true; }
};

void test_destruction() {
    Tracked::destroyed = false;
    {
        custom_shared_ptr<Tracked> p(new Tracked);
        assert(!Tracked::destroyed);
    }
    assert(Tracked::destroyed);
}
void test_shared_ptr(benchmark::State& state) {
  for (auto _ : state) {
    // test_basic_construction();
    // test_copy_semantics();
    // test_reset_and_use_count();
    // test_custom_deleter();
    test_destruction();
  }
}
BENCHMARK(test_shared_ptr)->Arg(1000000)->Iterations(1);

BENCHMARK_MAIN();