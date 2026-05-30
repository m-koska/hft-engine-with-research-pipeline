#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include "../Engine/OrderBook.hpp"

// Easily fitted into CPU L3 cache
constexpr uint64_t BATCH_SIZE = 100000;

// ================================================================
// 1. BENCHMARK: ADD ORDER (random prices around spread)
// ================================================================

static void BM_Engine_AddOrder(benchmark::State& state) {

    // generating prices
    std::vector<uint32_t> random_prices(BATCH_SIZE);
    std::mt19937 gen(42);
    std::uniform_int_distribution<uint32_t> price_dist(49900, 50100);

    for (size_t i = 0; i < BATCH_SIZE; ++i) {
        random_prices[i] = price_dist(gen);
    }

    for (auto _ : state) {
        state.PauseTiming();
        Engine::OrderBook ob(1000000, 100000, 100000);
        state.ResumeTiming();

        // adding orders with random prices
        for (uint64_t i = 0; i < BATCH_SIZE; ++i) {
            ob.addOrder(i, random_prices[i], 100, Engine::Side::BUY);
        }

        benchmark::DoNotOptimize(&ob);
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * BATCH_SIZE);

}
BENCHMARK(BM_Engine_AddOrder);

// ================================================================
// 2. BENCHMARK: REMOVE ORDER (in random order)
// ================================================================

static void BM_Engine_RemoveOrder(benchmark::State& state) {

    // randomised indices so that the CPU cannot predict the process well enough
    // in order to measure true random accse time
    std::vector<uint64_t> shuffled_ids(BATCH_SIZE);

    for (uint64_t i = 0; i < BATCH_SIZE; ++i) shuffled_ids[i] = i;

    std::mt19937 gen(1337);
    std::shuffle(shuffled_ids.begin(), shuffled_ids.end(), gen);

    for (auto _ : state) {

        state.PauseTiming();
        Engine::OrderBook ob(1000000, 100000, 100000);

        // Filling the OrderBook
        for (uint64_t i = 0; i < BATCH_SIZE; ++i) {
            ob.addOrder(i, 50000, 100, Engine::Side::BUY);
        }

        state.ResumeTiming();

        // accessing and removing Orders in random order
        for (uint64_t i = 0; i < BATCH_SIZE; ++i) {
            ob.removeOrder(shuffled_ids[i]);
        }

        benchmark::DoNotOptimize(&ob);
        benchmark::ClobberMemory();

    }

    state.SetItemsProcessed(state.iterations() * BATCH_SIZE);

}
BENCHMARK(BM_Engine_RemoveOrder);

// ================================================================
// 3. BENCHMARK: REDUCE ORDER VOLUME (in random order)
// ================================================================

static void BM_Engine_ReduceVolume(benchmark::State& state) {

    std::vector<uint64_t> shuffled_ids(BATCH_SIZE);

    for (uint64_t i = 0; i < BATCH_SIZE; ++i) shuffled_ids[i] = i;

    std::mt19937 gen(777);
    std::shuffle(shuffled_ids.begin(), shuffled_ids.end(), gen);

    for (auto _ : state) {

        state.PauseTiming();
        Engine::OrderBook ob(1000000, 100000, 100000);

        for (uint64_t i = 0; i < BATCH_SIZE; ++i) {
            ob.addOrder(i, 50000, 1000000, Engine::Side::BUY);
        }

        state.ResumeTiming();

        for (uint64_t i = 0; i < BATCH_SIZE; ++i) {
            ob.reduceOrderVolume(shuffled_ids[i], 10);
        }

        benchmark::DoNotOptimize(&ob);
        benchmark::ClobberMemory();

    }

    state.SetItemsProcessed(state.iterations() * BATCH_SIZE);

}
BENCHMARK(BM_Engine_ReduceVolume);

// ================================================================
// 4. BENCHMARK: REPLACE ORDER (ITCH 'U')
// ================================================================

static void BM_Engine_ReplaceOrder(benchmark::State& state) {

    std::vector<uint64_t> old_ids(BATCH_SIZE);
    std::vector<uint64_t> new_ids(BATCH_SIZE);
    std::vector<uint32_t> new_prices(BATCH_SIZE);

    std::mt19937 gen(999);
    std::uniform_int_distribution<uint32_t> price_dist(49900, 50100);

    for (uint64_t i = 0; i < BATCH_SIZE; ++i) {
        old_ids[i] = i;
        new_ids[i] = BATCH_SIZE + i; // big number, just to make sure this id wasn't used before
        new_prices[i] = price_dist(gen);
    }

    // shuffling order executions
    std::vector<size_t> execution_order(BATCH_SIZE);

    for (size_t i = 0; i < BATCH_SIZE; ++i) execution_order[i] = i;

    std::shuffle(execution_order.begin(), execution_order.end(), gen);

    for (auto _ : state) {

        state.PauseTiming();
        Engine::OrderBook ob(1000000, 100000, 100000);
        for (uint64_t i = 0; i < BATCH_SIZE; ++i) {
            ob.addOrder(i, 50000, 100, Engine::Side::BUY);
        }

        state.ResumeTiming();

        for (size_t i = 0; i < BATCH_SIZE; ++i) {
            size_t idx = execution_order[i];
            ob.replaceOrder(old_ids[idx], new_ids[idx], new_prices[idx], 150, Engine::Side::BUY);
        }

        benchmark::DoNotOptimize(&ob);
        benchmark::ClobberMemory();

    }

    state.SetItemsProcessed(state.iterations() * BATCH_SIZE);

}
BENCHMARK(BM_Engine_ReplaceOrder);

BENCHMARK_MAIN();