#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>

namespace acpf {

using Task = std::function<void()>;

class ThreadPool;

class TaskQueue {
  private:
    friend class ThreadPool;

    std::queue<Task> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool shutdown_ = false;
    std::size_t workers_to_stop_ = 0;

    /*
     * Requests one waiting worker to stop.
     * Wakes a waiting consumer.
     */
    void request_worker_stop();

  public:
    TaskQueue() = default;
    TaskQueue(const TaskQueue &) = delete;
    TaskQueue &operator=(const TaskQueue &) = delete;

    /*
     * Adds a task to the queue.
     * Returns false if the queue is shut down.
     */
    bool push(Task task);

    /*
     * Waits for and removes a task from the queue.
     * Returns false when the queue is shut down and empty.
     */
    bool wait_and_pop(Task &task);

    /*
     * Shuts down the queue and wakes waiting consumers.
     */
    void shutdown();

    /*
     * Returns the current number of tasks in the queue.
     */
    std::size_t size() const;
};
} // namespace acpf