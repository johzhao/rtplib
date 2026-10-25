#ifndef RTPLIB_TCP_SERVER_H
#define RTPLIB_TCP_SERVER_H

#include <netinet/in.h>

#include <functional>
#include <memory>
#include <string>

#include "reactor.h"
#include "tcp_connection.h"

namespace network {

class TcpServer {
public:
    explicit TcpServer(std::shared_ptr<Reactor> reactor);

    ~TcpServer();

    using ConnectionCallback = std::function<void(const std::shared_ptr<TcpConnection> &connection)>;

public:
    void SetConnectionCallback(ConnectionCallback callback);

    int Start(uint16_t port, const std::string &host = "0.0.0.0");

private:
    void OnAccept();

    void HandleNewConnection(const sockaddr_in &address, int client_fd);

    void Release();

private:
    int listen_fd_ = -1;
    std::shared_ptr<Reactor> reactor_;
    ConnectionCallback connection_callback_;
};

} // namespace network

#endif // RTPLIB_TCP_SERVER_H
