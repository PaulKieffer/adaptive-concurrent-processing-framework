#include <cstddef>
#include <thread>
#include <stdexcept>
#include "acpf/thread_pool.hpp"
#include "acpf/task_queue.hpp"

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
            });
        }
    }

    ThreadPool::ThreadPool(TaskQueue& queue) 
        : ThreadPool(queue, std::thread::hardware_concurrency()) {}

    ThreadPool::~ThreadPool() {
        for (auto& worker : workers_) {
            worker.join();
        }
    }
}