# pragma once

#include <cstddef>
#include "acpf/thread_pool.hpp"
#include "acpf/task_queue.hpp"


namespace acpf {

    class Controller {
    private:
        TaskQueue& queue_;
        ThreadPool& pool_;
    public:
        Controller(TaskQueue& queue, ThreadPool& pool);

        ~Controller();
    };
}
