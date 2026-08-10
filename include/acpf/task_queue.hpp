#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

namespace acpf {

    using Task = std::function<void()>;

    class TaskQueue {
    private:
        std::queue<Task> queue_;
        std::mutex mutex_;
        std::condition_variable condition_;
        bool shutdown_ = false;
    public:
        TaskQueue() = default;
        TaskQueue(const TaskQueue&) = delete;
        TaskQueue& operator=(const TaskQueue&) = delete;
        bool push(Task task);
        bool wait_and_pop(Task& task);
        void shutdown();
    };
}