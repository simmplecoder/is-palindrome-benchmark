//
// Created by shino on 21.12.21.
//

#ifndef IS_PAL_BENCHMARK_UTILS_H
#define IS_PAL_BENCHMARK_UTILS_H

#include <chrono>
#include <string_view>
#include <string>
#include <atomic>
#include <climits>
#include <random>
#include <stdexcept>
#include <algorithm>

namespace chrono = std::chrono;

namespace shino {
using benchmark_function_type = bool(std::string_view);
using benchmark_function_ptr = benchmark_function_type*;

using nanoseconds = chrono::nanoseconds;

inline nanoseconds sample(benchmark_function_ptr function, std::string_view input) {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto start = chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    volatile auto is_pal = function(input);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto end = chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);

    return chrono::duration_cast<nanoseconds>(end - start);
}

enum class input_generation_type {
    homogenous,
    spread,
    random
};

inline std::string generate_homogenous_input(std::size_t size) {
    std::string result(size, '\0');
    std::fill(result.begin(), result.end(), 'a');
    return result;
}

inline std::string generate_spread_input(std::size_t size) {
    std::string result(size, '\0');

    unsigned char start_char = 0;
    for (auto& c: result) {
        c = static_cast<char>(start_char);
        start_char += sizeof(std::size_t) * CHAR_BIT;
    }

    return result;
}

// the input is under 1MiB
// only request at a time
inline std::string generate_random_input(std::size_t size) {
    std::minstd_rand0 generator;
    std::uniform_int_distribution<char> dist;
    std::string result(size, '\0');
    std::generate(result.begin(), result.end(),
                  [&dist, &generator]() {
                      return dist(generator);
                  });

    return result;
}

inline std::string generate_input(std::size_t input_size, input_generation_type type) {
    std::string input;
    switch (type) {
        case input_generation_type::homogenous:
            input = generate_homogenous_input(input_size);
            break;
        case input_generation_type::spread:
            input = generate_spread_input(input_size);
            break;
        case input_generation_type::random:
            input = generate_random_input(input_size);
            break;
        default:
            throw std::logic_error("unknown input type");
    }

    return input;
}

inline std::vector<nanoseconds> benchmark(benchmark_function_ptr function,
                                   std::size_t run_count,
                                   std::size_t input_size,
                                   input_generation_type type) {
    std::vector<nanoseconds> samples;
    samples.reserve(run_count);
    for (std::size_t i = 0; i < run_count; ++i) {
        auto input = generate_input(input_size, type);
        samples.push_back(sample(function, input));
    }

    return samples;
}

struct metrics_t {
    nanoseconds min_time = nanoseconds(std::numeric_limits<std::int64_t>::max());
    nanoseconds max_time = nanoseconds(std::numeric_limits<std::int64_t>::min());
    nanoseconds avg_time = nanoseconds(0);
    nanoseconds _98th_percentile_time = nanoseconds(0);
};

// expects already sorted samples
inline metrics_t collect_metrics(const std::vector<nanoseconds>& samples) {
    metrics_t metrics;

    auto sum = nanoseconds(0);
    for (auto sample: samples) {
        sum += sample;

        if (metrics.min_time > sample) {
            metrics.min_time = sample;
        }

        if (metrics.max_time < sample) {
            metrics.max_time = sample;
        }

        sum += sample;
    }

    metrics.avg_time = sum / samples.size();
    metrics._98th_percentile_time = samples[static_cast<std::size_t>(static_cast<double>(samples.size()) * 0.98)];

    return metrics;
}

}

#endif //IS_PAL_BENCHMARK_UTILS_H
