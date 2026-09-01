# pragma once

#include <cstddef>
#include "acpf/thread_pool.hpp"
#include "acpf/task_queue.hpp"


namespace acpf {
    
    /*
     * Coordinates the lifecycle of a TaskQueue and its associated ThreadPool.
     *
     * The Controller holds references to both objects and does not take
     * ownership of them.
     *
     * During destruction, the Controller shuts down the TaskQueue and waits
     * for all worker threads to stop before reaping them.
     *
     * Running tasks are not forcibly interrupted. Destruction may therefore
     * block until all currently executing tasks have completed.
     */
    class Controller {
    private:
        TaskQueue& queue_;
        ThreadPool& pool_;
    public:

        /*
         * Creates a Controller for the specified TaskQueue and ThreadPool.
         * The ThreadPool must use the provided TaskQueue.
         */
        Controller(TaskQueue& queue, ThreadPool& pool);

        /*
         * Shuts down the TaskQueue and waits for all workers to stop.
         * Running tasks are not forcibly interrupted. Destruction may therefore
         * block until all currently executing tasks have completed.
         */
        ~Controller();
    };
}
