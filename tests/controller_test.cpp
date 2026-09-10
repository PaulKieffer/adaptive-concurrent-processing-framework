#include <gtest/gtest.h>

#include "acpf/controller.hpp"
#include "acpf/task_queue.hpp"
#include "acpf/thread_pool.hpp"

TEST(Controller, ShutsDownAllWorkers) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 4);
    acpf::Controller controller(pool);
}
