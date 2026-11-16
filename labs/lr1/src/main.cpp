#include "benchmark.hpp"
#include "tree_console.hpp"

#include <charconv>
#include <cstdint>
#include <iostream>
#include <string>

namespace {
template<class Number>
bool parseNumber(const char* text, Number& value) {
    const std::string input(text);
    const auto result = std::from_chars(input.data(), input.data() + input.size(), value);
    return result.ec == std::errc{} && result.ptr == input.data() + input.size();
}
}

int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        std::size_t limit = 10000000;
        std::uint64_t seed = 20261116;
        if (argc > 2 && !parseNumber(argv[2], limit)) {
            std::cerr << "Limit must be a positive integer\n";
            return 1;
        }
        if (argc > 3 && !parseNumber(argv[3], seed)) {
            std::cerr << "Seed must be an unsigned integer\n";
            return 1;
        }
        return run_benchmark(limit, seed);
    }
    return run_console();
}
