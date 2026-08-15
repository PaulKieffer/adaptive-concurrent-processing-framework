#include "acpf/task_queue.hpp"

namespace acpf {
    
    bool TaskQueue::push(Task task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (shutdown_) return false; // don't push new task on queue if shutdown activated
            queue_.push(std::move(task));
        }
        condition_.notify_one(); 
        return true;
    }

    bool TaskQueue::wait_and_pop(Task& task) {
        std::unique_lock<std::mutex> lock(mutex_);
        // wake up on available task or queue shutting down
        condition_.wait(lock, [this] { return !queue_.empty() || shutdown_; });
        // shutdown is complete once all pending tasks have been processed
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
        condition_.notify_all(); // wake waiting consumers, notify on shutdown-state
    }
}