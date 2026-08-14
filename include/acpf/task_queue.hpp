#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

namespace acpf {

    using Task = std::function<void()>;

    class TaskQueue {
    private:
        std::queue<Task> queue_;
        std::mutex mutex_;
        std::condition_variable condition_;
        bool shutdown_ = false;
    public:
        TaskQueue() = default;
        TaskQueue(const TaskQueue&) = delete;
        TaskQueue& operator=(const TaskQueue&) = delete;
        
        /**
         * Adds a task to the queue.
         *
         * Parameter:
         *   task - Task passed by value.
         *
         * Return:
         *   true  - Task was accepted.
         *   false - Task was rejected because the queue is shut down.
         *
         * Blocking:
         *   Does not block waiting for a consumer or task execution.
         *
         * Thread-safety:
         *   Can be called concurrently from multiple threads.
         *
         * Synchronization:
         *   A successfully inserted task signals a waiting consumer.
         */
        bool push(Task task);

        /**
         * Waits for and removes a task from the queue.
         *
         * Parameter:
         *   task - Output reference receiving the removed task.
         *
         * Return:
         *   true  - A task was removed and assigned to task.
         *   false - The queue is shut down and empty.
         *
         * Blocking:
         *   Blocks while the queue is empty and not shut down.
         *
         * Thread-safety:
         *   Can be called concurrently from multiple threads.
         */
        bool wait_and_pop(Task& task);

        /**
         * Shuts down the queue.
         *
         * Thread-safety:
         *   Can be called concurrently from multiple threads.
         *
         * Synchronization:
         *   Wakes all threads currently waiting in wait_and_pop().
         */
        void shutdown();
    };
}