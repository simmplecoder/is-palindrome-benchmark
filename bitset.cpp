//
// Created by shino on 21.12.21.
//
#include <bitset>
#include <string_view>

#include "benchmark_utils.h"

// https://codereview.stackexchange.com/a/272129
// from https://codereview.stackexchange.com/users/129343/g-sliepen
bool is_permutation_palindrome_bitset(std::string_view text)
{
    std::bitset<256> odd_characters;

    for (const auto ch : text)
        odd_characters.flip(static_cast<std::uint8_t>(ch));

    return odd_characters.count() <= 1;
}

#include <iostream>
#include <algorithm>

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

    auto samples = shino::benchmark(&is_permutation_palindrome_bitset,
                                    std::stoull(argv[2]),
                                    std::stoull(argv[1]),
                                    generation_type);

    std::sort(samples.begin(), samples.end());
    auto metrics = shino::collect_metrics(samples);
    std::cout << "metrics for bitset based version:\n"
              << "\tmin time: " << metrics.min_time.count() << '\n'
              << "\tmax time: " << metrics.max_time.count() << '\n'
              << "\tavg time: " << metrics.avg_time.count() << '\n'
              << "\t98th percentile time: " << metrics._98th_percentile_time.count() << '\n';
}
