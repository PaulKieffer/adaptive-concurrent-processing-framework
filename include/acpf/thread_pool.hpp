#pragma once

#include <thread>
#include <vector>
#include <cstddef>
#include "acpf/task_queue.hpp"


namespace acpf {

    class ThreadPool {
    private:
        TaskQueue& queue_;
        std::vector<std::thread> workers_;
    public:
        /**
         * Creates a thread pool with the specified number of workers.
         *
         * The worker threads consume and execute tasks from the 
         * provided TaskQueue.
         *
         * Throws std::invalid_argument if worker_count is zero.
         */
        ThreadPool(TaskQueue& queue, std::size_t worker_count);
        /**
         * Creates a thread pool using the system's reported hardware
         * concurrency.
         *
         * Throws std::invalid_argument if the system reports zero 
         * hardware threads.
         */
        explicit ThreadPool(TaskQueue& queue);
        /**
         * Waits for all worker threads to finish.
         *
         * The associated TaskQueue must be shut down before the ThreadPool
         * is destroyed.
         */
        ~ThreadPool();
    };
}