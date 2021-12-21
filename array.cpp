//
// Created by shino on 21.12.21.
//

#include <algorithm>
#include <string_view>

#include "benchmark_utils.h"


// based on https://codereview.stackexchange.com/a/272130
// from https://codereview.stackexchange.com/users/42409/deduplicator
bool is_permutation_palindrome_array(std::string_view s) noexcept {
    unsigned char counts[1u + (unsigned char)-1] {};
    for (unsigned char c : s)
        ++counts[c];
    return std::count_if(std::begin(counts), std::end(counts), [](auto a){ return a % 2; }) < 2;
}

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " <input-size> <target-run-count> "
                                             "<generation_type:'random'|'spread'|homogenous>\n";
        return EXIT_FAILURE;
    }

    std::string generation_type_str = argv[3];
    shino::input_generation_type generation_type;
    if (generation_type_str == "random") {
        generation_type = shino::input_generation_type::random;
    } else if (generation_type_str == "spread") {
        generation_type = shino::input_generation_type::spread;
    } else if (generation_type_str == "homogenous") {
        generation_type = shino::input_generation_type::homogenous;
    } else {
        std::cerr << "unknown input generation type\n";
        return EXIT_FAILURE;
    }

    auto samples = shino::benchmark(&is_permutation_palindrome_array,
                                    std::stoull(argv[2]),
                                    std::stoull(argv[1]),
                                    generation_type);

    std::sort(samples.begin(), samples.end());
    auto metrics = shino::collect_metrics(samples);
    std::cout << "metrics for array based version(ns):\n"
              << "\tmin time: " << metrics.min_time.count() << '\n'
              << "\tmax time: " << metrics.max_time.count() << '\n'
              << "\tavg time: " << metrics.avg_time.count() << '\n'
              << "\t98th percentile time: " << metrics._98th_percentile_time.count() << '\n';
}
