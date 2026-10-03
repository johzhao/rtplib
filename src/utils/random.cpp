#include "random.h"

#include <random>

namespace utils {

std::string Random::RandomNumber(int length) {
    static const std::vector<char> pool{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    return RandomString(pool, length);
}

std::string Random::RandomUpperHexNumber(int length) {
    static const std::vector<char> pool{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    return RandomString(pool, length);
}

std::string Random::RandomLowerHexNumber(int length) {
    static const std::vector<char> pool{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    return RandomString(pool, length);
}

std::string Random::RandomString(const std::vector<char> &pool, int length) {
    std::string result;
    result.reserve(length + 1);

    auto pool_size = pool.size();

    std::random_device seed;
    std::ranlux48 engine(seed());
    std::uniform_int_distribution<> distrib(0, static_cast<int>(pool_size));

    for (int i = 0; i < length; ++i) {
        const int random = distrib(engine);
        auto index = random % pool_size;
        result.push_back(pool[index]);
    }

    return result;
}

} // namespace utils
