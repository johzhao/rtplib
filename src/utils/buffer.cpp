#include "buffer.h"

namespace utils {
Buffer::Buffer(int initialCapacity)
    : buffer_(initialCapacity), reader_offset_(0), writer_offset_(0) {
}

Buffer::~Buffer() = default;

size_t Buffer::ReadableBytes() const {
    return writer_offset_ - reader_offset_;
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - writer_offset_;
}

void Buffer::Append(const char *data, size_t len) {
    const std::lock_guard<std::mutex> lock(buffer_mutex_);

    EnsureWritableBytes(len);
    std::copy(data, data + len, WritableAddress());
    writer_offset_ += len;
}

const char *Buffer::Peek() const {
    return buffer_.data() + reader_offset_;
}

void Buffer::Consume(size_t len) {
    const std::lock_guard<std::mutex> lock(buffer_mutex_);

    if (len < ReadableBytes()) {
        reader_offset_ += len;
    } else {
        ConsumeAll();
    }
}

char *Buffer::Begin() {
    return buffer_.data();
}

char *Buffer::WritableAddress() {
    return Begin() + writer_offset_;
}

void Buffer::EnsureWritableBytes(size_t len) {
    if (WritableBytes() < len) {
        MakeSpace(len);
    }
}

void Buffer::ConsumeAll() {
    reader_offset_ = 0;
    writer_offset_ = 0;
}

void Buffer::MakeSpace(size_t len) {
    if (WritableBytes() + reader_offset_ < len) {
        buffer_.resize(writer_offset_ + len);
    } else {
        // move readable data to the front, make space inside buffer
        auto readable = ReadableBytes();
        std::copy(Begin() + reader_offset_, Begin() + writer_offset_, Begin());
        reader_offset_ = 0;
        writer_offset_ = reader_offset_ + readable;
    }
}
} // namespace utils
