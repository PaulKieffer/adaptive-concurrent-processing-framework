#include <algorithm>
#include <cstddef>
#include <mutex>
#include <stdexcept>
#include <thread>

#include "acpf/task_queue.hpp"
#include "acpf/thread_pool.hpp"

namespace acpf {

ThreadPool::ThreadPool(TaskQueue &queue, std::size_t worker_count) : queue_(queue) {
    if (worker_count == 0) {
        throw std::invalid_argument("ThreadPool requires at least one worker");
    }
    // worker loop
    for (std::size_t i = 0; i < worker_count; ++i) {
        workers_.emplace_back([this] {
            Task task;

            while (queue_.wait_and_pop(task)) {
                task();
            }
            // worker has stopped
            worker_stopped(std::this_thread::get_id());
        });
    }
}

ThreadPool::ThreadPool(TaskQueue &queue) : ThreadPool(queue, std::thread::hardware_concurrency()) {}

ThreadPool::~ThreadPool() = default;

std::size_t ThreadPool::worker_count() const noexcept {
    std::lock_guard lock(control_mutex_);
    return workers_.size();
}

void ThreadPool::reduce_workers(std::size_t count) {
    std::lock_guard lock(control_mutex_);

    // calculate allowed amount of workers to reduce (min 1 active worker)
    const std::size_t effective_count = workers_.size() - pending_reductions_;
    if (effective_count <= 1 || count == 0) {
        return;
    }
    const std::size_t reducible = effective_count - 1;
    const std::size_t requested = std::min(count, reducible);
    pending_reductions_ += requested;

    // request workers to stop
    for (std::size_t i = 0; i < requested; ++i) {
        queue_.request_worker_stop();
    }
}

void ThreadPool::reap_stopped_workers() {
    std::vector<std::thread> stopped;
    {
        std::lock_guard lock(control_mutex_);
        // stopped workers, added to stopped removed from workers_
        while (stopped_threads_ > 0) {
            stopped.push_back(std::move(workers_.back()));
            workers_.pop_back();
            --stopped_threads_;
        }
    }
    // join workers in stopped
    for (auto &worker : stopped) {
        worker.join();
    }
}

void ThreadPool::worker_stopped(std::thread::id id) {
    std::lock_guard lock(control_mutex_);
    // move stopped thread to end of array
    for (std::size_t i = 0; i < workers_.size(); ++i) {
        if (workers_[i].get_id() == id) {
            std::swap(workers_[i], workers_.back());
            ++stopped_threads_;
            if (pending_reductions_ > 0) {
                --pending_reductions_;
            }
            break;
        }
    }
    control_condition_.notify_one();
}

void ThreadPool::wait_for_workers_to_stop() {
    std::unique_lock lock(control_mutex_);

    control_condition_.wait(lock, [this] { return stopped_threads_ == workers_.size(); });
}

} // namespace acpf