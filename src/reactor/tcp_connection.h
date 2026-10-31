#ifndef RTPLIB_TCP_CONNECTION_H
#define RTPLIB_TCP_CONNECTION_H

#include <memory>

#include "channel.h"
#include "reactor.h"

namespace network {

class TcpConnection {
public:
    explicit TcpConnection(std::shared_ptr<Reactor> reactor);

    ~TcpConnection();

    using ReadCallback = std::function<void(TcpConnection *connection, const char *data, size_t length)>;

    using WriteCompleteCallback = std::function<void(TcpConnection *connection)>;

    using CloseCallback = std::function<void(TcpConnection *connection)>;

public:
    int Initialize(std::shared_ptr<Channel> channel, const std::string &ip, uint16_t port);

    uint64_t Id() const;

    void SetReadCallback(ReadCallback callback);

    void SetWriteCallback(WriteCompleteCallback callback);

    void SetCloseCallback(CloseCallback callback);

    bool IsConnected() const;

    void Send(const char *data, size_t length);

    void Shutdown();

private:
    void OnReceived(Channel *channel);

    void HandleReceivedData(const char *buffer, size_t len);

    void HandleDisconnected();

    void OnWritable(Channel *channel);

    void SendImpl(Channel *channel);

    void Release();

private:
    uint64_t id_ = 0;
    std::shared_ptr<Reactor> reactor_;
    std::shared_ptr<Channel> channel_;
    std::string ip_;
    uint16_t port_ = 0;

    std::vector<char> receive_buffer_;
    ReadCallback read_callback_;

    size_t send_buffer_offset_ = 0;
    std::vector<char> send_buffer_;
    WriteCompleteCallback write_callback_;

    bool closed_ = false;
    CloseCallback close_callback_;
};

} // namespace network

#endif // RTPLIB_TCP_CONNECTION_H
