#ifndef RTPLIB_TIMER_MANAGER_H
#define RTPLIB_TIMER_MANAGER_H

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>

namespace timer {

class Timer;

class TimerManager {
public:
    TimerManager();

    ~TimerManager();

    using TimerId = uint64_t;

    using TimerCallback = std::function<void(uint64_t timer_id)>;

public:
    TimerId AddTimer(uint64_t milliseconds, TimerCallback callback, bool repeat = false);

    void CancelTimer(TimerId timer_id);

private:
    void Stop();

    TimerId GetNextTimerId();

    void RunLoop();

    class Timer {
    public:
        uint64_t id = 0;
        uint64_t interval = 0;
        std::chrono::steady_clock::time_point expire_time;
        TimerCallback callback = nullptr;
        bool repeat = false;
    };

    struct TimerCompare {
        bool operator()(const std::shared_ptr<Timer> &first, const std::shared_ptr<Timer> &second) const {
            return first->expire_time > second->expire_time;
        }
    };

private:
    std::thread work_thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> new_timer_added{false};
    std::mutex timer_mutex_;
    std::condition_variable timer_condition_variable;
    std::unordered_map<TimerId, std::shared_ptr<Timer>> timer_map_;
    std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, TimerCompare> timer_queue_;
    TimerId next_timer_id_ = 0;
};

} // namespace timer

#endif // RTPLIB_TIMER_MANAGER_H
