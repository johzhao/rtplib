#ifndef RTPLIB_TCP_CLIENT_H
#define RTPLIB_TCP_CLIENT_H

#include <functional>
#include <memory>
#include <string>

#include "tcp_connection.h"

namespace network {

class TcpClient {
public:
    explicit TcpClient(std::shared_ptr<Reactor> reactor);

    ~TcpClient();

    using ConnectedCallback = std::function<void(TcpClient *client)>;

    using ReadCallback = std::function<void(TcpClient *client, const char *data, size_t length)>;

    using WriteCompleteCallback = std::function<void(TcpClient *client)>;

    using CloseCallback = std::function<void(TcpClient *client)>;

public:
    void SetConnectCallback(ConnectedCallback callback);

    void SetReadCallback(ReadCallback callback);

    void SetWriteCallback(WriteCompleteCallback callback);

    void SetCloseCallback(CloseCallback callback);

    int Initialize();

    int ConnectTo(const std::string &host, uint16_t port);

    void Send(const char *data, size_t length);

    void Shutdown();

private:
    int CreateChannel();

    int OnConnected();

    void OnConnected(Channel *channel);

    void OnRead(const char *buffer, size_t len);

    void OnWriteComplete();

    void OnClosed();

    void Release();

private:
    uint64_t id_ = 0;
    int client_fd_ = -1;
    std::shared_ptr<Reactor> reactor_;
    std::shared_ptr<Channel> channel_;
    std::shared_ptr<TcpConnection> connection_;
    std::string ip_;
    uint16_t port_ = 0;

    ConnectedCallback connected_callback_;
    ReadCallback read_callback_;
    WriteCompleteCallback write_callback_;
    CloseCallback close_callback_;
};

} // namespace network

#endif // RTPLIB_TCP_CLIENT_H
