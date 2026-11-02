#ifndef RTPLIB_UDP_CONNECTION_H
#define RTPLIB_UDP_CONNECTION_H

#include <netinet/in.h>

#include "reactor.h"

namespace network {

class UdpConnection {
public:
    explicit UdpConnection(std::shared_ptr<Reactor> reactor);

    ~UdpConnection();

    using ReadCallback = std::function<void(UdpConnection *connection, const sockaddr_in &address, const char *data, size_t length)>;

    using WriteCompleteCallback = std::function<void(UdpConnection *connection)>;

public:
    void SetReadCallback(ReadCallback callback);

    void SetWriteCompleteCallback(WriteCompleteCallback callback);

    int Initialize();

    int BindLocal(uint16_t port = 0, const std::string &host = "0.0.0.0");

    int SendTo(const sockaddr_in &address, const char *data, size_t length);

private:
    void OnDataReceived(Channel *channel);

    void HandleReceivedData(const sockaddr_in &address, const char *data, size_t length);

private:
    std::shared_ptr<Reactor> reactor_;
    std::shared_ptr<Channel> channel_;
    ReadCallback read_callback_;
    WriteCompleteCallback write_complete_callback_;

    std::vector<char> receive_buffer_;
    std::vector<char> send_buffer_;
};

} // namespace network

#endif // RTPLIB_UDP_CONNECTION_H
