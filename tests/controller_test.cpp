#include <atomic>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include "acpf/task_queue.hpp"
#include "acpf/thread_pool.hpp"
#include "acpf/controller.hpp"


TEST(Controller, ShutsDownAllWorkers)
{   
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 4);
    acpf::Controller controller(queue, pool);
}
