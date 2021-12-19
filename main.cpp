#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <bitset>
#include <memory_resource>

#include "input_gen.h"

// Based on https://codereview.stackexchange.com/q/272128
// from https://codereview.stackexchange.com/users/58360/coderodde
bool is_permutation_palindrome_original(const std::string& text)
{
    const std::size_t buffer_size = 8 * 1024;
    std::array<std::byte, buffer_size> scratch;
    std::pmr::monotonic_buffer_resource resource(scratch.data(), buffer_size);
    std::pmr::unordered_map<char, size_t> counter_map;
//    counter_map.reserve(256);

    for (const auto ch : text)
        ++counter_map[ch];

    size_t number_of_odd_chars = 0;

    for (const auto &pair: counter_map)
        if (counter_map[pair.first] % 2 == 1)
        {
            ++number_of_odd_chars;

            if (number_of_odd_chars > 1)
                return false;
        }

    return true;
}

// based on https://codereview.stackexchange.com/a/272130
// from https://codereview.stackexchange.com/users/42409/deduplicator
bool is_permutation_palindrome_array(std::string_view s) noexcept {
    unsigned char counts[1u + (unsigned char)-1] {};
    for (unsigned char c : s)
        ++counts[c];
    return std::count_if(std::begin(counts), std::end(counts), [](auto a){ return a % 2; }) < 2;
}

// https://codereview.stackexchange.com/a/272129
// from https://codereview.stackexchange.com/users/129343/g-sliepen
bool is_permutation_palindrome_bitset(const std::string& text)
{
    std::bitset<256> odd_characters;

    for (const auto ch : text)
        odd_characters.flip(static_cast<std::uint8_t>(ch));

    return odd_characters.count() <= 1;
}

#include <chrono>
namespace chrono = std::chrono;

struct sample_t {
    chrono::nanoseconds original_time;
    chrono::nanoseconds array_time;
    chrono::nanoseconds bitset_time;
};

sample_t measure(const std::string& input) {
    sample_t sample{};
    {
        auto start_time = chrono::steady_clock::now();
        volatile bool is_pal = is_permutation_palindrome_original(input);
        auto end_time = chrono::steady_clock::now();
        sample.original_time = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time);
    }


    {
        auto start_time = chrono::steady_clock::now();
        volatile bool is_pal = is_permutation_palindrome_array(input);
        auto end_time = chrono::steady_clock::now();
        sample.array_time = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time);
    }

    {
        auto start_time = chrono::steady_clock::now();
        volatile bool is_pal = is_permutation_palindrome_bitset(input);
        auto end_time = chrono::steady_clock::now();
        sample.bitset_time = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time);
    }

    return sample;
}

struct metric_t {
    chrono::nanoseconds min = chrono::nanoseconds(std::numeric_limits<std::int64_t>::max());
    chrono::nanoseconds max = chrono::nanoseconds(0);
    chrono::nanoseconds sum = chrono::nanoseconds(0);

    void update(chrono::nanoseconds new_sample) {
        if (min > new_sample) {
            min = new_sample;
        }

        if (max < new_sample) {
            max = new_sample;
        }

        sum += new_sample;
    }
};


#include <sstream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " <input-size> <run-count> <output-file>\n";
        return EXIT_FAILURE;
    }

    const std::size_t size = std::stoull(argv[1]);
    const std::size_t target_amount_of_runs = std::stoull(argv[2]);

    std::vector<sample_t> samples;
    samples.reserve(target_amount_of_runs);
    for (std::size_t i = 0; i < target_amount_of_runs; ++i) {
        auto input = shino::generate_random_input(size);
        auto sample = measure(input);
        samples.push_back(sample);
    }

    metric_t metrics_for_og;
    metric_t metrics_for_array;
    metric_t metrics_for_bitset;
    for (const auto& sample: samples) {
        metrics_for_og.update(sample.original_time);
        metrics_for_array.update(sample.array_time);
        metrics_for_bitset.update(sample.bitset_time);
    }

    chrono::nanoseconds avg_time_og = metrics_for_og.sum / target_amount_of_runs;
    chrono::nanoseconds avg_time_array = metrics_for_array.sum / target_amount_of_runs;
    chrono::nanoseconds avg_time_bitset = metrics_for_bitset.sum / target_amount_of_runs;

    std::vector<chrono::nanoseconds> og_samples;
    og_samples.reserve(target_amount_of_runs);
    std::vector<chrono::nanoseconds> array_samples;
    array_samples.reserve(target_amount_of_runs);
    std::vector<chrono::nanoseconds> bitset_samples;
    bitset_samples.reserve(target_amount_of_runs);
    for (const auto& sample: samples) {
        og_samples.push_back(sample.original_time);
        array_samples.push_back(sample.array_time);
        bitset_samples.push_back(sample.bitset_time);
    }

    std::sort(og_samples.begin(), og_samples.end());
    std::sort(array_samples.begin(), array_samples.end());
    std::sort(bitset_samples.begin(), bitset_samples.end());

    std::ostringstream oss;
    oss << "original function metrics (ns):\n"
        << "\tmin: " << metrics_for_og.min.count() << '\n'
        << "\tmax: " << metrics_for_og.max.count() << '\n'
        << "\tavg: " << avg_time_og.count() << '\n'
        << "\t98th percentile: " << og_samples[target_amount_of_runs * 0.98].count()
              << "\n\n";

    oss << "array based function metrics (ns):\n"
        << "\tmin: " << metrics_for_array.min.count() << '\n'
        << "\tmax: " << metrics_for_array.max.count() << '\n'
        << "\tavg: " << avg_time_array.count() << '\n'
        << "\t98th percentile: " << array_samples[target_amount_of_runs * 0.98].count()
              << "\n\n";

    oss << "bitset based function metrics (ns):\n"
        << "\tmin: " << metrics_for_bitset.min.count() << '\n'
        << "\tmax: " << metrics_for_bitset.max.count() << '\n'
        << "\tavg: " << avg_time_bitset.count() << '\n'
        << "\t98th percentile: " << bitset_samples[target_amount_of_runs * 0.98].count()
              << "\n\n";

    std::ofstream output_file(argv[3]);
    if (!output_file) {
        std::cerr << "opening output file failed, printing results to stderr:\n"
                  << oss.str();
        return EXIT_FAILURE;
    }

    output_file << oss.str();
}
