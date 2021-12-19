#ifndef IS_PAL_INPUT_GEN_H
#define IS_PAL_INPUT_GEN_H

#include <string>
#include <random>
#include <algorithm>

namespace shino {

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

}

#endif //IS_PAL_INPUT_GEN_H
