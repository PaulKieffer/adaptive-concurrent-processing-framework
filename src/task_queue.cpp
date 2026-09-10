#include <utility>

#include "acpf/task_queue.hpp"

namespace acpf {

bool TaskQueue::push(Task task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_)
            return false; // don't push new task on queue if shutdown activated
        queue_.push(std::move(task));
    }
    condition_.notify_one();
    return true;
}

bool TaskQueue::wait_and_pop(Task &task) {
    std::unique_lock lock(mutex_);

    // wake up on available task or queue shutting down
    // or stop-signal
    condition_.wait(lock, [this] { return !queue_.empty() || shutdown_ || workers_to_stop_ > 0; });

    // shutdown is complete once all pending tasks have been processed
    if (!queue_.empty()) {
        task = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    if (shutdown_ || workers_to_stop_ > 0) {
        if (workers_to_stop_ > 0) {
            --workers_to_stop_;
        }

        return false;
    }

    return false;
}

void TaskQueue::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
    }
    condition_.notify_all(); // wake waiting consumers, notify on shutdown-state
}

std::size_t TaskQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void TaskQueue::request_worker_stop() {
    {
        std::lock_guard lock(mutex_);
        ++workers_to_stop_;
    }
    condition_.notify_one();
}
} // namespace acpf