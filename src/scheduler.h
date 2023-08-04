#pragma once

#include <condition_variable>

namespace scheduler {

    namespace helper {
        template<typename Clock>
        void sleep_until(std::chrono::time_point<Clock> time, const std::stop_token& st) {
            std::condition_variable cv;
            std::mutex mutex;
            std::unique_lock<std::mutex> ul{ mutex };
            std::stop_callback stop_wait{ st, [&cv]() { cv.notify_one(); } };
            cv.wait_until(ul, time, [&st]() { return st.stop_requested(); });
        }
    }


    class SyncScheduler {
    public:
        SyncScheduler(std::stop_token st) : stop_token{ st } {}
        void run() {
            while (!stop_token.stop_requested()) {
                auto task_ptr = select_next_task();
                if (task_ptr == nullptr) {
                    return;
                }
                auto& task = *task_ptr;
                helper::sleep_until(task.get_next_execution(), stop_token);
                if (stop_token.stop_requested()) {
                    return;
                }
                task.run();
            }
        }

        void every(const std::chrono::milliseconds period, std::function<void(void)> func) {
            tasks.emplace_back(period, std::move(func));
        }

    private:
        struct Task {
            using time_point = std::chrono::time_point<std::chrono::steady_clock>;

            Task(const std::chrono::milliseconds period_, std::function<void()> func_) : period{ period_ }, func { std::move(func_) } {
                next_execution = std::chrono::steady_clock::now();
            }
            void run() {
                func();
                update_next_execution();
            }

            time_point get_next_execution() const noexcept {
                return next_execution;
            }

        private:
            void update_next_execution() noexcept {
                const auto now = std::chrono::steady_clock::now();
                next_execution = std::max(next_execution + period, now);
            }

            std::function<void(void)> func;
            std::chrono::milliseconds period;
            time_point next_execution;
        };

        Task* select_next_task() noexcept {
            auto minTask = std::min_element(tasks.begin()
                , tasks.end()
                , [](const Task& fst, const Task& snd) {
                    return fst.get_next_execution() < snd.get_next_execution();
                });
            return minTask != tasks.end() ? &*minTask : nullptr;
        }
        
        std::stop_token stop_token;
        std::vector<Task> tasks;
    };
}

