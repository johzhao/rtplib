#ifndef RTPLIB_SEQUENCE_EXTENDER_H
#define RTPLIB_SEQUENCE_EXTENDER_H

#include <cstdint>

namespace rtplib {

class SequenceExtender {
public:
    SequenceExtender() = default;

    ~SequenceExtender() = default;

public:
    uint32_t Extend(uint16_t sequence);

    void Reset();

private:
    bool initialized_ = false;
    uint16_t last_sequence_ = 0;
    uint32_t cycles_ = 0;
    uint32_t max_extended_sequence_ = 0;
};

} // namespace rtplib

#endif // RTPLIB_SEQUENCE_EXTENDER_H
