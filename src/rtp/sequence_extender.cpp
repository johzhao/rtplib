#include "sequence_extender.h"

namespace rtplib {

uint32_t SequenceExtender::Extend(uint16_t sequence) {
    if (!initialized_) {
        initialized_ = true;
        last_sequence_ = sequence;
        cycles_ = 0;
        max_extended_sequence_ = sequence;

        return sequence;
    }

    uint16_t last = last_sequence_;

    /*
     * RTP sequence number arithmetic is modulo 65536.
     *
     * Forward distance:
     *
     *   seq - last
     *
     * interpreted as int16_t.
     *
     * Example:
     *
     *   last = 65535
     *   seq  = 0
     *
     *   0 - 65535 = 1 modulo 65536
     *
     * so this is forward.
     */

    auto delta = static_cast<int16_t>(static_cast<uint16_t>(sequence - last));
    if (delta > 0) {
        /*
         * Detect wrap:
         *
         * 65535 -> 0
         */
        if (sequence < last) {
            ++cycles_;
        }

        uint32_t extended = (cycles_ << 16) | sequence;
        if (extended > max_extended_sequence_) {
            max_extended_sequence_ = extended;
            last_sequence_ = sequence;
        }

        return extended;
    }

    if (delta == 0) {
        return (cycles_ << 16) | sequence;
    }

    // ------------------------------------------------------------
    // Backward packet.
    //
    // Could simply be reordering:
    //
    //   100
    //   102
    //   101
    //
    // But could also be previous-cycle packet:
    //
    //   0
    //   65535
    // ------------------------------------------------------------

    uint32_t currentCycle = cycles_;
    uint32_t currentExt = (currentCycle << 16) | sequence;

    /*
     * If sequence is near the beginning of the range while
     * current packet is near the end, this packet probably
     * belongs to the previous cycle.
     */
    if (sequence > 0xC000 && last < 0x4000 && currentCycle > 0) {
        currentExt = ((currentCycle - 1) << 16) | sequence;
    }

    /*
     * If sequence is near the end and current packet is near
     * the beginning, it might belong to the next cycle.
     *
     * We DO NOT advance cycles here because this packet is
     * out-of-order and should not move the stream head.
     */
    else if (sequence < 0x4000 && last > 0xC000) {
        currentExt = ((currentCycle + 1) << 16) | sequence;
    }

    return currentExt;
}

void SequenceExtender::Reset() {
    initialized_ = false;
    last_sequence_ = 0;
    cycles_ = 0;
    max_extended_sequence_ = 0;
}

} // namespace rtplib
