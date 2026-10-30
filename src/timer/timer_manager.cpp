#include "timer_manager.h"

#include <spdlog/spdlog.h>

namespace timer {

TimerManager::TimerManager() {
    SPDLOG_INFO("start the timer manager");

    running_.store(true);

    work_thread_ = std::thread(&TimerManager::RunLoop, this);
}

TimerManager::~TimerManager() {
    Stop();
}

TimerManager::TimerId TimerManager::AddTimer(uint64_t milliseconds, TimerCallback callback, bool repeat) {
    auto timer = std::make_shared<Timer>();

    timer->id = GetNextTimerId();
    timer->interval = milliseconds;
    timer->expire_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds);
    timer->callback = std::move(callback);
    timer->repeat = repeat;

    {
        const std::scoped_lock scoped_lock(timer_mutex_);
        timer_map_[timer->id] = timer;
        timer_queue_.push(timer);
    }

    new_timer_added.store(true);
    timer_condition_variable.notify_one();

    return timer->id;
}

void TimerManager::CancelTimer(TimerId timer_id) {
    const std::scoped_lock scoped_lock(timer_mutex_);

    timer_map_.erase(timer_id);

    timer_condition_variable.notify_one();
}

void TimerManager::Stop() {
    running_.store(false);
    if (work_thread_.joinable()) {
        timer_condition_variable.notify_all();
        work_thread_.join();
    }
}

TimerManager::TimerId TimerManager::GetNextTimerId() {
    const std::scoped_lock scoped_lock(timer_mutex_);

    while (true) {
        ++next_timer_id_;

        if (next_timer_id_ == 0) {
            continue;
        }

        auto it = timer_map_.find(next_timer_id_);
        if (it != timer_map_.end()) {
            continue;
        }

        break;
    }

    return next_timer_id_;
}

void TimerManager::RunLoop() {
    while (running_.load()) {
        std::unique_lock unique_lock(timer_mutex_);

        if (timer_queue_.empty()) {
            timer_condition_variable.wait(unique_lock, [this] {
                return !running_ || !timer_queue_.empty();
            });

            continue;
        }

        auto timer = timer_queue_.top();
        if (timer->expire_time > std::chrono::steady_clock::now()) {
            timer_condition_variable.wait_until(unique_lock, timer->expire_time, [this] {
                return !running_ || new_timer_added.load();
            });

            new_timer_added.store(false);
            continue;
        }

        timer_queue_.pop();

        auto it = timer_map_.find(timer->id);
        if (it == timer_map_.end()) {
            continue;
        }

        if (timer->repeat) {
            timer->expire_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(timer->interval);
            timer_queue_.push(timer);
        } else {
            timer_map_.erase(timer->id);
        }

        unique_lock.unlock();

        try {
            timer->callback(timer->id);
        } catch (std::exception &ex) {
            SPDLOG_ERROR("invoke the callback of timer {} raised exception '{}'", timer->id, ex.what());
        }
    }
}

} // namespace timer
