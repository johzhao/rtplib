#include "base64.h"

#include <cstdint>

namespace utils {

static const char BASE64_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "0123456789+/";

static int base64Value(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }

    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }

    if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }

    if (c == '+') {
        return 62;
    }

    if (c == '/') {
        return 63;
    }

    return -1;
}

std::string Base64::Encode(const std::string &input) {
    std::string output;

    int val = 0;
    int bits = -6;

    for (unsigned char c: input) {
        val = (val << 8) + c;
        bits += 8;

        while (bits >= 0) {
            output.push_back(BASE64_TABLE[(val >> bits) & 0x3F]);

            bits -= 6;
        }
    }

    if (bits > -6) {
        output.push_back(BASE64_TABLE[((val << 8) >> (bits + 8)) & 0x3F]);
    }

    while ((output.size() % 4) != 0) {
        output.push_back('=');
    }

    return output;
}

bool Base64::Decode(const std::string &input, std::string &output) {
    output.clear();

    int value = 0;
    int bits = -8;

    for (char c : input) {
        // 忽略 Base64 中允许的空白
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            continue;
        }

        if (c == '=') {
            break;
        }

        int v = base64Value(c);

        if (v < 0) {
            return false;
        }

        value = (value << 6) | v;
        bits += 6;

        if (bits >= 0) {
            output.push_back(static_cast<uint8_t>((value >> bits) & 0xFF));

            bits -= 8;
        }
    }

    return true;
}

} // namespace utils