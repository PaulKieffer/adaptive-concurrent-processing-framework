#include <atomic>
#include <future>
#include <gtest/gtest.h>
#include "acpf/thread_pool.hpp"
#include "acpf/task_queue.hpp"


TEST(ThreadPool, ExecutesTask) {
    acpf::TaskQueue queue;
    std::atomic<int> executed_tasks = 0;

    std::promise<void> task_completed;
    auto task_finished = task_completed.get_future();

    ASSERT_TRUE(
        queue.push([&task_completed, &executed_tasks] {
            executed_tasks.fetch_add(1, std::memory_order_relaxed);
            task_completed.set_value();
        })
    );

    {
        acpf::ThreadPool pool(queue, 1);

        /*
         * A timeout prevents the test from blocking indefinitely if the
         * worker fails to execute the task.
         *
         * One second provides sufficient tolerance for a slow or heavily
         * loaded system. The timeout is nevertheless a test assumption,
         * so an exceptionally slow system could cause a false failure.
         */
        const auto status =
            task_finished.wait_for(std::chrono::seconds(1));

        queue.shutdown();

        ASSERT_EQ(status, std::future_status::ready);
    }

    EXPECT_EQ(executed_tasks.load(), 1);
}

TEST(ThreadPool, ExecutesMultipleTasks) {
    constexpr int task_count = 100;

    acpf::TaskQueue queue;
    std::atomic<int> executed_task_count = 0;
    
    std::promise<void> tasks_completed;
    auto tasks_finished = tasks_completed.get_future();

    for (int i = 0; i < task_count; ++i) {
        ASSERT_TRUE(
            queue.push([&executed_task_count, &tasks_completed] {
                const int count = executed_task_count.fetch_add(
                    1,
                    std::memory_order_relaxed
                ) + 1;

                if (count == task_count) {
                    tasks_completed.set_value();
                }
            })
        );
    }

    {
        acpf::ThreadPool pool(queue, 1);

        /*
         * Wait until all submitted tasks have been executed.
         *
         * A timeout is used deliberately: if a task is lost or a worker
         * fails to execute a task, the future would never become ready and
         * an unconditional wait would block the test indefinitely.
         *
         * Five seconds should provide sufficient tolerance for a slow
         * test environment while still ensuring that a faulty test
         * terminates instead of hanging indefinitely.
         *
         * The timeout itself is therefore a test assumption: on an
         * exceptionally slow or heavily loaded system, the test could
         * fail even though the ThreadPool is functionally correct.
         */
        const auto status =
            tasks_finished.wait_for(std::chrono::seconds(5));

        queue.shutdown();

        ASSERT_EQ(status, std::future_status::ready);
    }

    EXPECT_EQ(executed_task_count.load(), task_count);
}