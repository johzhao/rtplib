#ifndef RTPLIB_BASE_64_H
#define RTPLIB_BASE_64_H

#include <string>

namespace utils {

class Base64 {
public:
    static std::string Encode(const std::string &input);

    static bool Decode(const std::string &input, std::string &output);
};

} // namespace utils

#endif // RTPLIB_BASE_64_H
