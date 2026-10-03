#ifndef RTPLIB_MD_5_H
#define RTPLIB_MD_5_H

#include <cstdint>
#include <string>

namespace utils {

class MD5 {
public:
    static std::string Digest(const std::string &input);

    static std::string Digest(const void* data, size_t len);

public:
    MD5();

    ~MD5() = default;

public:
    void Update(const uint8_t* data, size_t len);

    std::string Final();

private:
    void Init();

    void Transform(const uint8_t block[64]);

private:
    uint32_t state_[4]{};
    uint64_t bit_count_ = 0;

    uint64_t bit_count_before_padding_ = 0;

    uint8_t buffer_[64] = {};

    bool finalized_ = false;
    std::string result_;
};

} // namespace utils


#endif // RTPLIB_MD_5_H
