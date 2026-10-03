#include "strings.h"

#include <algorithm>
#include <sstream>

namespace utils {

std::vector<std::string> Strings::Split(const std::string &data, const std::string &delimiter) {
    std::vector<std::string> result;

    std::size_t previous = 0;
    std::size_t current = data.find(delimiter);
    while (current != std::string::npos) {
        if (current > previous) {
            result.push_back(data.substr(previous, current - previous));
        }
        previous = current + delimiter.length();
        current = data.find(delimiter, previous);
    }

    if (previous != data.size()) {
        result.push_back(data.substr(previous));
    }

    return result;
}

std::string Strings::TrimSpace(const std::string &data) {
    const std::function keep_character_callback = [](char c) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            return false;
        }

        return true;
    };

    return Trim(data, keep_character_callback);
}

std::string Strings::Trim(const std::string &data, const std::function<bool(char)> &keep_character_callback) {
    if (data.empty()) {
        return data;
    }

    auto data_length = data.length();

    size_t start_position = 0;
    while (start_position < data_length) {
        const char character = data[start_position];
        if (keep_character_callback(character)) {
            break;
        }

        ++start_position;
    }

    auto end_position = static_cast<int>(data_length - 1);
    while (end_position >= 0) {
        const char character = data[end_position];
        if (keep_character_callback(character)) {
            break;
        }

        --end_position;
    }

    if (end_position <= static_cast<int>(start_position)) {
        return "";
    }

    return data.substr(start_position, end_position - start_position + 1);
}

std::string Strings::Hex2Bin(const std::string &str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i += 2) {
        const std::string byte = str.substr(i, 2);
        const char chr = static_cast<char>(strtol(byte.c_str(), nullptr, 16));
        result.push_back(chr);
    }
    return result;
}

std::string Strings::Bin2Hex(const std::string &data) {
    char temp[3] = {};
    std::stringstream builder;
    for (const unsigned char i: data) {
        sprintf(temp, "%02x", i);
        builder << temp;
    }
    return builder.str();
}

std::string Strings::ToLower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

bool Strings::StartsWith(const std::string &str, const std::string &prefix) {
    if (prefix.size() > str.size()) {
        return false;
    }

    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

bool Strings::EndsWith(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) {
        return false;
    }

    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string Strings::ReplaceAll(const std::string &data, const std::string &from, const std::string &to) {
    if (from.empty()) {
        return data;
    }

    std::string copied_data{data};
    size_t pos = 0;
    while ((pos = copied_data.find(from, pos)) != std::string::npos) {
        copied_data.replace(pos, from.length(), to);
        pos += to.length();
    }

    return copied_data;
}

} // namespace utils
