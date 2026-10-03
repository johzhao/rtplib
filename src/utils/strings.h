#ifndef STRINGS_H
#define STRINGS_H

#include <functional>
#include <string>
#include <vector>

namespace utils {

class Strings {
public:
    static std::vector<std::string> Split(const std::string &data, const std::string &delimiter);

    static std::string TrimSpace(const std::string &data);

    static std::string Trim(const std::string &data, const std::function<bool(char)> &keep_character_callback);

    static std::string Hex2Bin(const std::string &str);

    static std::string Bin2Hex(const std::string &data);

    static std::string ToLower(const std::string &str);

    static bool StartsWith(const std::string &str, const std::string &prefix);

    static bool EndsWith(const std::string &str, const std::string &suffix);

    static std::string ReplaceAll(const std::string &data, const std::string &from, const std::string &to);
};

} // namespace utils

#endif // STRINGS_H
