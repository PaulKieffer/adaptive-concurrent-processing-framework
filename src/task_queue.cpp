#include "acpf/task_queue.hpp"

namespace acpf {
    void TaskQueue::push(Task task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (shutdown_) return; // don't push new task on queue if shutdown activated
            queue_.push(std::move(task));
        }
        condition_.notify_one(); // notify a waiting thread on new task
    }

    bool TaskQueue::wait_and_pop(Task& task) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty() || shutdown_; });
        if (queue_.empty() && shutdown_) { return false; }
        task = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void TaskQueue::shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        condition_.notify_all(); // notify all waiting threads on new condition
    }
}