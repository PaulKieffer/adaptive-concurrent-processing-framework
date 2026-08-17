#include <atomic>
#include <future>
#include <gtest/gtest.h>
#include "acpf/thread_pool.hpp"
#include "acpf/task_queue.hpp"


TEST(ThreadPool, ExecutesTask) {
    acpf::TaskQueue queue;
    std::atomic<int> executed_tasks = 0;

    // Synchronization between the worker thread and the test thread.
    std::promise<void> task_completed;
    auto task_finished = task_completed.get_future();

    ASSERT_TRUE(
        queue.push([&task_completed, &executed_tasks] {
            executed_tasks.fetch_add(1, std::memory_order_relaxed);
            // Signal that the task has been executed.
            task_completed.set_value();
        })
    );

    {
        acpf::ThreadPool pool(queue, 1);
        // Wait until the worker has actually executed the task.
        task_finished.wait();
        // Shutdown must happen before the ThreadPool is destroyed.
        queue.shutdown();
    }

    EXPECT_EQ(executed_tasks.load(), 1);
}