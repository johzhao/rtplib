#include "md5.h"

#include <cstring>
#include <iomanip>
#include <sstream>

namespace utils {

static constexpr uint32_t S[64] = {
        7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 5,  9,  14, 20, 5,  9,
        14, 20, 5,  9,  14, 20, 5,  9,  14, 20, 4,  11, 16, 23, 4,  11, 16, 23, 4,  11, 16, 23,
        4,  11, 16, 23, 6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21,
};

static constexpr uint32_t K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

static uint32_t left_rotate(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
}

static void encode_le(uint8_t *output, uint32_t value) {
    output[0] = static_cast<uint8_t>(value);
    output[1] = static_cast<uint8_t>(value >> 8);
    output[2] = static_cast<uint8_t>(value >> 16);
    output[3] = static_cast<uint8_t>(value >> 24);
}

static void encode_le(uint8_t *output, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        output[i] = static_cast<uint8_t>(value >> (i * 8));
    }
}

static uint32_t decode_le(const uint8_t *input) {
    return static_cast<uint32_t>(input[0]) | (static_cast<uint32_t>(input[1]) << 8) |
           (static_cast<uint32_t>(input[2]) << 16) | (static_cast<uint32_t>(input[3]) << 24);
}

std::string MD5::Digest(const std::string &input) {
    MD5 md5;
    md5.Update(reinterpret_cast<const uint8_t *>(input.data()), input.size());
    return md5.Final();
}

std::string MD5::Digest(const void *data, size_t len) {
    MD5 md5;
    md5.Update(static_cast<const uint8_t *>(data), len);
    return md5.Final();
}

MD5::MD5() {
    Init();
}

void MD5::Update(const uint8_t *data, size_t len) {
    size_t index = bit_count_ / 8 % 64;

    bit_count_ += len * 8;

    size_t part_len = 64 - index;
    size_t i = 0;

    if (len >= part_len) {
        std::memcpy(buffer_ + index, data, part_len);
        Transform(buffer_);

        for (i = part_len; i + 63 < len; i += 64) {
            Transform(data + i);
        }

        index = 0;
    }

    if (i < len) {
        std::memcpy(buffer_ + index, data + i, len - i);
    }
}

std::string MD5::Final() {
    if (finalized_) {
        return result_;
    }

    uint64_t original_bit_count = bit_count_;

    uint8_t padding[64] = {};
    padding[0] = 0x80;

    size_t index = static_cast<size_t>((bit_count_ / 8) % 64);
    size_t pad_len = (index < 56) ? (56 - index) : (120 - index);

    Update(padding, pad_len);

    uint8_t length[8];
    encode_le(length, original_bit_count);

    Update(length, 8);

    uint8_t digest_bytes[16];

    encode_le(digest_bytes + 0, state_[0]);
    encode_le(digest_bytes + 4, state_[1]);
    encode_le(digest_bytes + 8, state_[2]);
    encode_le(digest_bytes + 12, state_[3]);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (uint8_t byte: digest_bytes) {
        oss << std::setw(2) << static_cast<unsigned int>(byte);
    }

    result_ = oss.str();
    finalized_ = true;

    return result_;
}

void MD5::Init() {
    state_[0] = 0x67452301;
    state_[1] = 0xefcdab89;
    state_[2] = 0x98badcfe;
    state_[3] = 0x10325476;

    bit_count_ = 0;
    bit_count_before_padding_ = 0;
    finalized_ = false;
    result_.clear();

    std::memset(buffer_, 0, sizeof(buffer_));
}

void MD5::Transform(const uint8_t block[64]) {
    uint32_t a = state_[0];
    uint32_t b = state_[1];
    uint32_t c = state_[2];
    uint32_t d = state_[3];

    uint32_t M[16];

    for (int i = 0; i < 16; ++i) {
        M[i] = decode_le(block + i * 4);
    }

    for (int i = 0; i < 64; ++i) {
        uint32_t F;
        int g;

        if (i < 16) {
            F = (b & c) | (~b & d);
            g = i;
        } else if (i < 32) {
            F = (d & b) | (~d & c);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            F = b ^ c ^ d;
            g = (3 * i + 5) % 16;
        } else {
            F = c ^ (b | ~d);
            g = (7 * i) % 16;
        }

        uint32_t temp = d;
        d = c;
        c = b;

        b = b + left_rotate(a + F + K[i] + M[g], S[i]);

        a = temp;
    }

    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
}

} // namespace utils
