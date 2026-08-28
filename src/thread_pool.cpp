#include <cstddef>
#include <thread>
#include <stdexcept>
#include "acpf/thread_pool.hpp"
#include "acpf/task_queue.hpp"
#include <mutex>
#include <algorithm>


namespace acpf {

    ThreadPool::ThreadPool(TaskQueue& queue, std::size_t worker_count)
        : queue_(queue) {
        if (worker_count == 0) {
            throw std::invalid_argument(
                "ThreadPool requires at least one worker"
            );
        }
        for (std::size_t i = 0; i < worker_count; ++i) {
            workers_.emplace_back([this] {
                Task task;

                while (queue_.wait_and_pop(task)) {
                    task();
                }

                // Worker has actually stopped.
                worker_stopped(std::this_thread::get_id());
            });
        }
    }

    ThreadPool::ThreadPool(TaskQueue& queue) 
        : ThreadPool(queue, std::thread::hardware_concurrency()) {}

    ThreadPool::~ThreadPool() = default;


    std::size_t ThreadPool::worker_count() const noexcept {
        std::lock_guard lock(control_mutex_);
        return workers_.size();
    }

    void ThreadPool::reduce_workers(std::size_t count) {
        std::lock_guard lock(control_mutex_);

        const std::size_t effective_count =
            workers_.size() - pending_reductions_;

        if (effective_count <= 1 || count == 0) {
            return;
        }

        const std::size_t reducible =
            effective_count - 1;

        const std::size_t requested =
            std::min(count, reducible);

        pending_reductions_ += requested;

        for (std::size_t i = 0; i < requested; ++i) {
            queue_.request_worker_stop();
        }
    }


    void ThreadPool::reap_stopped_workers() {
        std::vector<std::thread> stopped;

        {
            std::lock_guard lock(control_mutex_);

            while (stopped_threads_ > 0) {
                stopped.push_back(std::move(workers_.back()));
                workers_.pop_back();
                --stopped_threads_;
            }
        }

        for (auto& worker : stopped) {
            worker.join();
        }
    }


    void ThreadPool::worker_stopped(std::thread::id id) {
        std::lock_guard lock(control_mutex_);
        
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
    }

}