#include <functional>
#include <thread>
#include <chrono>
#include <atomic>

class Timer {
public:
    Timer(int interval, std::function<void()> callback)
        : interval_(interval), callback_(callback), running_(false) {}

    void start() {
        running_ = true;
        thread_ = std::thread([this]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
                if (running_) {
                    callback_();
                }
            }
        });
    }

    void stop() {
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    ~Timer() {
        stop();
    }

private:
    int interval_;
    std::function<void()> callback_;
    std::atomic<bool> running_;
    std::thread thread_;
};