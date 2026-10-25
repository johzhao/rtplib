#ifndef RTPLIB_CHANNEL_H
#define RTPLIB_CHANNEL_H

#include <cstdint>
#include <functional>
#include <memory>

namespace network {

class Channel : public std::enable_shared_from_this<Channel> {
public:
    using Callback = std::function<void(Channel *channel)>;

    explicit Channel(int fd);

    ~Channel();

public:
    void SetReadCallback(Callback callback);

    void SetWriteCallback(Callback callback);

    void SetErrorCallback(Callback callback);

    int Fd() const;

    void HandleReadEvent();

    void HandleWriteEvent();

    void HandleErrorEvent();

    uint64_t Token() const;

    void SetToken(const uint64_t &token);

private:
    int fd_;
    uint64_t token_ = 0;
    Callback read_callback_ = nullptr;
    Callback write_callback_ = nullptr;
    Callback error_callback_ = nullptr;
};

} // namespace network

#endif // RTPLIB_CHANNEL_H
