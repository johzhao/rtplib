#ifndef RANDOM_H
#define RANDOM_H

#include <string>
#include <vector>

namespace utils {

class Random {
public:
    static std::string RandomNumber(int length);

    static std::string RandomUpperHexNumber(int length);

    static std::string RandomLowerHexNumber(int length);

private:
    static std::string RandomString(const std::vector<char> &pool, int length);
};

} // namespace utils

#endif // RANDOM_H
