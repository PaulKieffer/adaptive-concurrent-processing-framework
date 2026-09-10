#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <vector>

#include "acpf/task_queue.hpp"

namespace acpf {

class Controller;

class ThreadPool {
  private:
    friend class Controller;
    /*
     * Records that a worker has stopped and marks its thread for reaping.
     * Called by a worker when its worker loop terminates.
     */
    void worker_stopped(std::thread::id id);
    /*
     * Joins all workers that have already stopped and removes them
     * from the worker list.
     * This function does not wait for active workers to stop.
     */
    void reap_stopped_workers();

    TaskQueue &queue_;
    std::vector<std::thread> workers_;
    mutable std::mutex control_mutex_;
    std::condition_variable control_condition_;
    std::size_t pending_reductions_ = 0;
    std::size_t stopped_threads_ = 0;

  public:
    /*
     * Creates a thread pool with the specified number of workers.
     * The worker threads consume and execute tasks from the
     * provided TaskQueue.
     * Throws std::invalid_argument if worker_count is zero.
     */
    ThreadPool(TaskQueue &queue, std::size_t worker_count);
    /*
     * Creates a thread pool using the system's reported hardware
     * concurrency.
     * Throws std::invalid_argument if the system reports zero
     * hardware threads.
     */
    explicit ThreadPool(TaskQueue &queue);
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
    /*
     * Waits until all currently active worker threads have stopped.
     *
     * This function does not request workers to stop and does not join or
     * remove stopped workers. Worker termination must be triggered separately,
     * for example by shutting down the associated TaskQueue or requesting
     * worker reductions.
     *
     * This function is intended for lifecycle coordination by the Controller.
     */
    void wait_for_workers_to_stop();
};
} // namespace acpf