#include "tcp_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <spdlog/spdlog.h>

namespace network {

TcpClient::TcpClient(std::shared_ptr<Reactor> reactor)
    : reactor_(std::move(reactor)) {
}

TcpClient::~TcpClient() {
    Release();
}

void TcpClient::SetConnectCallback(ConnectedCallback callback) {
    connected_callback_ = std::move(callback);
}

void TcpClient::SetReadCallback(ReadCallback callback) {
    read_callback_ = std::move(callback);
}

void TcpClient::SetWriteCallback(WriteCompleteCallback callback) {
    write_callback_ = std::move(callback);
}

void TcpClient::SetCloseCallback(CloseCallback callback) {
    close_callback_ = std::move(callback);
}

int TcpClient::Initialize() {
    client_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (client_fd_ == -1) {
        SPDLOG_ERROR("create listen socket failed");
        Release();

        return -1;
    }

    int opt = 1;
    if (setsockopt(client_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        SPDLOG_ERROR("set socket reuse address failed");
        Release();

        return -1;
    }

    return 0;
}

int TcpClient::ConnectTo(const std::string &host, uint16_t port) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    inet_pton(AF_INET, host.c_str(), &address.sin_addr);
    address.sin_port = htons(port);

    ip_ = host;
    port_ = port;

    auto ret = CreateChannel();
    if (ret != 0) {
        SPDLOG_ERROR("create channel failed {}", ret);

        return ret;
    }

    ret = connect(client_fd_, reinterpret_cast<sockaddr *>(&address), sizeof(address));
    if (ret == 0) {
        // 连接立即成功
        SPDLOG_INFO("socket was connected");

        ret = OnConnected();
        if (ret != 0) {
            return ret;
        }
    } else if (errno == EINPROGRESS) {
        // 连接尚未完成
        SPDLOG_DEBUG("connection was in progress");
    } else {
        // 连接立即失败
        SPDLOG_ERROR("connect failed {}, {}", ret, strerror(errno));
        return ret;
    }

    return 0;
}

void TcpClient::Send(const char *data, size_t length) {
    if (connection_ != nullptr) {
        connection_->Send(data, length);
    }
}

void TcpClient::Shutdown() {
    if (connection_ != nullptr) {
        connection_->Shutdown();
    }
}

int TcpClient::CreateChannel() {
    channel_ = std::make_shared<Channel>(client_fd_);

    channel_->SetWriteCallback([this](Channel *chan) {
        OnConnected(chan);
    });

    auto ret = reactor_->Add(channel_, EPOLLOUT);
    if (ret != 0) {
        SPDLOG_ERROR("reactor add channel failed {}", ret);

        return ret;
    }

    return 0;
}

int TcpClient::OnConnected() {
    SPDLOG_DEBUG("tcp client connected");

    connection_ = std::make_shared<TcpConnection>(reactor_);
    auto ret = connection_->Initialize(channel_, ip_, port_);
    if (ret != 0) {
        SPDLOG_ERROR("connection initialize failed {}", ret);

        return ret;
    }

    connection_->SetReadCallback([this](TcpConnection *connection, const char *data, size_t length) {
        OnRead(data, length);
    });

    connection_->SetWriteCallback([this](TcpConnection *connection) {
        OnWriteComplete();
    });

    connection_->SetCloseCallback([this](TcpConnection *connection) {
        OnClosed();
    });

    if (connected_callback_ != nullptr) {
        connected_callback_(this);
    }

    return 0;
}

void TcpClient::OnConnected(Channel *channel) {
    auto ret = OnConnected();
    if (ret != 0) {
        // TODO: notify by callback
    }
}

void TcpClient::OnRead(const char *buffer, size_t len) {
    if (read_callback_ != nullptr) {
        read_callback_(this, buffer, len);
    }
}

void TcpClient::OnWriteComplete() {
    if (write_callback_ != nullptr) {
        write_callback_(this);
    }
}

void TcpClient::OnClosed() {
    if (close_callback_ != nullptr) {
        close_callback_(this);
    }
}

void TcpClient::Release() {
}

} // namespace network
