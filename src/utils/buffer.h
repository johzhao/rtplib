#ifndef UTILS_BUFFER_H
#define UTILS_BUFFER_H

#include <mutex>
#include <vector>

namespace utils {
class Buffer {
public:
    explicit Buffer(int initialCapacity);

    ~Buffer();

public:
    size_t ReadableBytes() const;

    size_t WritableBytes() const;

    void Append(const char *data, size_t len);

    const char* Peek() const;

    void Consume(size_t len);

private:
    char *Begin();

    char *WritableAddress();

    void EnsureWritableBytes(size_t len);

    void ConsumeAll();

    void MakeSpace(size_t len);

private:
    std::mutex buffer_mutex_;
    std::vector<char> buffer_;
    size_t reader_offset_;
    size_t writer_offset_;
};
} // namespace utils

#endif // UTILS_BUFFER_H
