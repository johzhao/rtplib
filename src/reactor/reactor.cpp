#include "reactor.h"

#include <netinet/in.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <unistd.h>

#include <spdlog/spdlog.h>

namespace network {

static constexpr int kMaxEpollEventCount = 64;
static constexpr uint64_t kMinToken = 1;
static constexpr uint64_t kMaxToken = UINT64_MAX - 1;

Reactor::Reactor(int id)
    : id_(id)
    , events_(new epoll_event[kMaxEpollEventCount])
    , next_token_(kMinToken) {
}

Reactor::~Reactor() {
    Release();

    delete[] events_;
}

int Reactor::Initialize() {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        SPDLOG_ERROR("reactor {} create epoll failed with error {}, reason: '{}'", id_, errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)
        Release();

        return -1;
    }

    wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0) {
        SPDLOG_ERROR("reactor {} create wakeup fd failed with error {}, reason {}", id_, errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)
        Release();

        return -1;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.u64 = 0;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wakeup_fd_, &event) < 0) {
        SPDLOG_ERROR("reactor {} add wakeup fd to epoll failed with error {}, reason {}", id_, errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)
        Release();

        return -1;
    }

    return 0;
}

int Reactor::Add(const std::shared_ptr<Channel> &channel, uint32_t events) {
    auto token = GetToken();

    epoll_event event{};
    event.events = events;
    event.data.u64 = token;

    auto rc = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, channel->Fd(), &event);
    if (rc < 0) {
        SPDLOG_ERROR("reactor {} epoll_ctl add receive sock failed with error: {}, reason: '{}'", id_, errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)

        return -1;
    }

    {
        const std::scoped_lock lock(mutex_);
        channel_map_[token] = channel;
    }

    channel->SetToken(token);

    return 0;
}

int Reactor::Modify(const std::shared_ptr<Channel> &channel, uint32_t events) {
    if (channel->Token() == 0) {
        return Add(channel, events);
    }

    epoll_event event{};
    event.events = events;
    event.data.u64 = channel->Token();

    auto rc = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, channel->Fd(), &event);
    if (rc < 0) {
        SPDLOG_ERROR("reactor {} epoll_ctl modify receive sock failed with error: {}, reason: '{}'", id_, errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)

        return -1;
    }

    {
        const std::scoped_lock lock(mutex_);
        channel_map_[channel->Token()] = channel;
    }

    return 0;
}

int Reactor::Remove(const std::shared_ptr<Channel> &channel) {
    if (channel->Token() == 0) {
        SPDLOG_ERROR("reactor {} remove channel which token was invalid", id_);

        return -1;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = channel->Fd();

    auto rc = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->Fd(), &event);
    if (rc < 0) {
        SPDLOG_ERROR("reactor {} epoll_ctl delete sock failed with error: {}, description: '{}'", id_, errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)

        return -1;
    }

    {
        const std::scoped_lock lock(mutex_);

        channel_map_.erase(channel->Token());
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

            SPDLOG_ERROR("reactor {} epoll wait failed with error {}, reason {}", id_, errno,
                         strerror(errno)); // NOLINT(*-mt-unsafe)

            break;
        }

        for (int i = 0; i < ret; ++i) {
            uint64_t token = events[i].data.u64;
            if (token == 0) {
                HandleWakeup();

                continue;
            }

            DoDispatch(events[i].events, token);
        }
    }
}

void Reactor::DoDispatch(const uint32_t &events, const uint64_t &token) {
    std::shared_ptr<Channel> channel = nullptr;
    {
        const std::scoped_lock lock(mutex_);
        channel = FindChannel(token);
    }

    if (channel == nullptr) {
        return;
    }

    if ((events & EPOLLIN) != 0) {
        channel->HandleReadEvent();
    }

    if ((events & EPOLLOUT) != 0) {
        channel->HandleWriteEvent();
    }

    if ((events & (EPOLLERR | EPOLLHUP)) != 0) {
        channel->HandleErrorEvent();
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
        SPDLOG_ERROR("reactor {} write wakeup fd failed with error {}, reason {}", id_, errno,
                     strerror(errno)); // NOLINT(*-mt-unsafe)
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

std::shared_ptr<Channel> Reactor::FindChannel(const uint64_t &token) {
    auto it = channel_map_.find(token);
    if (it != channel_map_.end()) {
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

    channel_map_.clear();
}

} // namespace network
