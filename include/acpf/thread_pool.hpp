#pragma once

#include <thread>
#include <vector>
#include <cstddef>
#include "acpf/task_queue.hpp"
#include <mutex>


namespace acpf {

    class ThreadPool {
    private:
        void worker_stopped(std::thread::id id);
        void reap_stopped_workers();
        TaskQueue& queue_;
        std::vector<std::thread> workers_;
        mutable std::mutex control_mutex_;
        std::size_t pending_reductions_ = 0;
        std::size_t stopped_threads_ = 0;
    public:
        /*
         * Creates a thread pool with the specified number of workers.
         * The worker threads consume and execute tasks from the 
         * provided TaskQueue.
         * Throws std::invalid_argument if worker_count is zero.
         */
        ThreadPool(TaskQueue& queue, std::size_t worker_count);
        /*
         * Creates a thread pool using the system's reported hardware
         * concurrency.
         * Throws std::invalid_argument if the system reports zero 
         * hardware threads.
         */
        explicit ThreadPool(TaskQueue& queue);
        /*
         * Destroys the thread pool.
         * The associated TaskQueue must be shut down and all stopped workers
         * must have been reaped before the ThreadPool is destroyed.
         */
        ~ThreadPool();
        /*
         * Returns the current number of worker threads.
         */
        std::size_t worker_count() const noexcept;
        /*
         * Requests the specified number of worker threads to stop.
         * Does not forcibly terminate running workers.
         */
        void reduce_workers(std::size_t count);
    };
}