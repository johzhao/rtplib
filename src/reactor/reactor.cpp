#include "reactor.h"

#include <netinet/in.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ranges>

#include <spdlog/spdlog.h>

namespace rtp {

static constexpr int kMaxEpollEventCount = 64;
static constexpr uint64_t kMinToken = 1;
static constexpr uint64_t kMaxToken = UINT64_MAX - 1;

Reactor::Reactor()
    : events_(new epoll_event[kMaxEpollEventCount])
    , next_token_(kMinToken) {
}

Reactor::~Reactor() {
    Release();

    connections_.clear();
    delete[] events_;
}

int Reactor::Initialize() {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        SPDLOG_ERROR("create epoll failed with error {}, reason: '{}'", errno, strerror(errno)); // NOLINT(*-mt-unsafe)
        Release();

        return -1;
    }

    wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0) {
        SPDLOG_ERROR("create wakeup fd failed with error {}, reason {}", errno, strerror(errno)); // NOLINT(*-mt-unsafe)
        Release();

        return -1;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.u64 = 0;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wakeup_fd_, &event) < 0) {
        SPDLOG_ERROR("add wakeup fd to epoll failed with error {}, reason {}", errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)
        Release();

        return -1;
    }

    return 0;
}

int Reactor::RegisterForListen(int fd, const std::shared_ptr<IReactorHandler> &handler) {
    auto connection = std::make_shared<Connection>();
    connection->is_listen = true;
    connection->fd = fd;
    connection->token = GetToken();
    connection->handler = handler;

    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    event.data.fd = connection->fd;
    event.data.u64 = connection->token;

    auto rc = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, connection->fd, &event);
    if (rc < 0) {
        SPDLOG_ERROR("epoll_ctl add receive sock failed with error: {}, reason: '{}'", errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)

        return -1;
    }

    {
        const std::scoped_lock lock(mutex_);
        connections_[connection->token] = connection;
    }

    return 0;
}

int Reactor::RegisterForDataRead(int fd, const std::shared_ptr<IReactorHandler> &handler) {
    auto connection = std::make_shared<Connection>();
    connection->is_listen = false;
    connection->fd = fd;
    connection->token = GetToken();
    connection->handler = handler;

    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    event.data.fd = connection->fd;
    event.data.u64 = connection->token;

    auto rc = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, connection->fd, &event);
    if (rc < 0) {
        SPDLOG_ERROR("epoll_ctl add receive sock failed with error: {}, reason: '{}'", errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)

        return -1;
    }

    {
        const std::scoped_lock lock(mutex_);
        connections_[connection->token] = connection;
    }

    return 0;
}

int Reactor::UnRegister(int fd) {
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = fd;

    auto rc = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event);
    if (rc < 0) {
        SPDLOG_ERROR("epoll_ctl delete sock failed with error: {}, description: '{}'", errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)

        return -1;
    }

    {
        const std::scoped_lock lock(mutex_);

        for (auto &val: connections_ | std::views::values) {
            if (val->fd == fd) {
                connections_.erase(val->token);
                break;
            }
        }
    }

    return 0;
}

int Reactor::Start() {
    running_.store(true);
    work_thread_ = std::thread(&Reactor::RunLoop, this);

    return 0;
}

int Reactor::Stop() {
    running_.store(false);
    if (work_thread_.joinable()) {
        Wakeup();
        work_thread_.join();
    }

    return 0;
}

void Reactor::RunLoop() {
    constexpr int kMaxEvents = 128;

    epoll_event events[kMaxEvents];
    while (running_) {
        int ret = epoll_wait(epoll_fd_, events, kMaxEvents, -1);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            SPDLOG_ERROR("epoll wait failed with error {}, reason {}", errno, strerror(errno)); // NOLINT(*-mt-unsafe)

            break;
        }

        for (int i = 0; i < ret; ++i) {
            uint64_t token = events[i].data.u64;
            uint32_t event = events[i].events;
            if (token == 0) {
                HandleWakeup();

                continue;
            }

            if ((event & (EPOLLERR | EPOLLHUP)) != 0) {
                pending_close_.push_back(token);
                continue;
            }

            if ((event & EPOLLIN) != 0) {
                HandleRead(token);
            }

            if ((event & EPOLLOUT) != 0) {
                HandleWrite(token);
            }
        }

        ProcessPendingClose();
    }
}

uint64_t Reactor::GetToken() {
    auto result = next_token_;

    ++next_token_;
    if (next_token_ < kMinToken || next_token_ > kMaxToken) {
        next_token_ = kMinToken;
    }

    return result;
}

void Reactor::Wakeup() const {
    uint64_t value = 1;
    ssize_t bytes = write(wakeup_fd_, &value, sizeof(value));
    if (bytes < 0 && errno != EAGAIN) {
        SPDLOG_ERROR("write wakeup fd failed with error {}, reason {}", errno, strerror(errno)); // NOLINT(*-mt-unsafe)
    }
}

void Reactor::HandleWakeup() const {
    uint64_t value = 0;
    while (true) {
        ssize_t count = read(wakeup_fd_, &value, sizeof(value));
        if (count == sizeof(value)) {
            continue;
        }

        if (count < 0 && errno == EAGAIN) {
            break;
        }

        if (count < 0 && errno == EINTR) {
            continue;
        }

        break;
    }
}

void Reactor::HandleRead(const uint64_t &token) {
    std::shared_ptr<Connection> connection = nullptr;
    {
        const std::scoped_lock lock(mutex_);
        connection = FindConnection(token);
    }

    if (connection == nullptr) {
        return;
    }

    if (connection->is_listen) {
        connection->handler->HandleIncomingConnection(connection->fd);

        return;
    }

    constexpr int kReceiveBufferSize = 8192;
    char buffer[kReceiveBufferSize];

    while (true) {
        sockaddr_in address{};
        socklen_t address_len = sizeof(address);

        auto received = recvfrom(connection->fd, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr *>(&address),
                                 &address_len);
        if (received > 0) {
            connection->handler->HandleReceivedData(connection->fd, buffer, received,
                                                    reinterpret_cast<sockaddr *>(&address), address_len);

            continue;
        }

        if (received == 0) {
            pending_close_.push_back(token);

            return;
        }

        if (errno == EINTR) {
            continue;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }

        pending_close_.push_back(token);

        return;
    }
}

void Reactor::HandleWrite(const uint64_t &token) {
    std::shared_ptr<Connection> connection = nullptr;
    {
        const std::scoped_lock lock(mutex_);
        connection = FindConnection(token);
    }

    if (connection == nullptr) {
        return;
    }

    connection->handler->HandleWritable(connection->fd);
}

void Reactor::HandleClose(const uint64_t &token) {
    std::shared_ptr<Connection> connection = nullptr;
    {
        const std::scoped_lock lock(mutex_);
        connection = FindConnection(token);
    }

    if (connection == nullptr) {
        return;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = connection->fd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, connection->fd, &event);

    {
        const std::scoped_lock lock(mutex_);
        connections_.erase(token);
    }

    connection->handler->HandleDisconnected(connection->fd);
}

void Reactor::ProcessPendingClose() {
    if (pending_close_.empty()) {
        return;
    }

    std::vector<uint64_t> closing;
    closing.swap(pending_close_);

    for (unsigned long &i: closing) {
        HandleClose(i);
    }
}

std::shared_ptr<Reactor::Connection> Reactor::FindConnection(const uint64_t &token) {
    auto it = connections_.find(token);
    if (it != connections_.end()) {
        return it->second;
    }

    return nullptr;
}

void Reactor::Release() {
    running_.store(false);
    if (work_thread_.joinable()) {
        Wakeup();
        work_thread_.join();
    }

    if (wakeup_fd_ >= 0) {
        close(wakeup_fd_);
        wakeup_fd_ = -1;
    }

    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

} // namespace rtp
