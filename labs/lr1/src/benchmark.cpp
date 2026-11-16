#include "benchmark.hpp"

#include "avl_set.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>

namespace {
using Clock = std::chrono::steady_clock;

double elapsedMilliseconds(Clock::time_point start, Clock::time_point finish) {
    return std::chrono::duration<double, std::milli>(finish - start).count();
}

std::size_t sampleSize(int point) {
    const double ratio = std::pow(10000.0, 1.0 / 19.0);
    return static_cast<std::size_t>(std::llround(10000.0 * std::pow(ratio, point)));
}
}

int run_benchmark(std::size_t limit, std::uint64_t seed) {
    std::ofstream report("benchmark.csv");
    if (!report) {
        std::cerr << "Cannot open benchmark.csv for writing\n";
        return 1;
    }
    report << "elements,insert_ms,find_ms,array_find_ms,erase_ms,seed\n";
    std::size_t previous = 0;
    for (int point = 0; point < 20; ++point) {
        std::size_t number = sampleSize(point);
        if (number <= previous) number = previous + 1;
        if (number > limit) break;
        previous = number;
        try {
            auto values = std::make_unique<std::size_t[]>(number);
            std::iota(values.get(), values.get() + number, std::size_t{0});
            std::mt19937_64 engine(seed + static_cast<std::uint64_t>(point));
            std::shuffle(values.get(), values.get() + number, engine);

            AvlSet<std::size_t> tree;
            const auto insertStart = Clock::now();
            for (std::size_t i = 0; i < number; ++i) tree.add(values[i]);
            const auto insertEnd = Clock::now();

            for (std::size_t i = 0; i < number; ++i) {
                if (!tree.has(values[i])) throw InvalidState("benchmark lookup missed an inserted key");
            }
            const auto findEnd = Clock::now();

            std::sort(values.get(), values.get() + number);
            const auto arrayFindStart = Clock::now();
            std::mt19937_64 arrayQueries(seed ^ (static_cast<std::uint64_t>(point) + 0x9e3779b97f4a7c15ULL));
            std::uniform_int_distribution<std::size_t> query(0, number - 1);
            for (std::size_t i = 0; i < number; ++i) {
                if (!std::binary_search(values.get(), values.get() + number, query(arrayQueries)))
                    throw InvalidState("sorted-array lookup missed an inserted key");
            }
            const auto arrayFindEnd = Clock::now();

            std::shuffle(values.get(), values.get() + number, engine);
            const auto eraseStart = Clock::now();
            for (std::size_t i = 0; i < number; ++i) tree.remove(values[i]);
            const auto eraseEnd = Clock::now();
            if (!tree.empty()) throw InvalidState("benchmark deletion left keys behind");

            report << number << ',' << elapsedMilliseconds(insertStart, insertEnd) << ','
                   << elapsedMilliseconds(insertEnd, findEnd) << ','
                   << elapsedMilliseconds(arrayFindStart, arrayFindEnd) << ','
                   << elapsedMilliseconds(eraseStart, eraseEnd) << ',' << seed << '\n';
            report.flush();
            std::cout << "Completed " << number << " items\n";
        } catch (const std::bad_alloc&) {
            std::cerr << "Memory limit reached at " << number << " items; prior measurements are saved.\n";
            return 2;
        }
    }
    return 0;
}
