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

TEST(ThreadPool, ExecutesTasksConcurrently) {
    constexpr int worker_count = 4;
    constexpr int task_count = worker_count;

    acpf::TaskQueue queue;

    std::atomic<int> active_tasks = 0;

    /*
     * Signals when all tasks have entered their execution section.
     * The test waits for this signal to ensure that all workers are
     * executing a task concurrently before allowing the tasks to finish.
     */
    std::promise<void> tasks_started;
    auto all_tasks_started = tasks_started.get_future();

    /*
     * Shared release signal for all tasks.
     * Tasks remain active until the test explicitly releases them.
     * This prevents the first worker from finishing before the other
     * workers have had a chance to start their tasks.
     */
    std::promise<void> release_tasks;
    auto release = release_tasks.get_future().share();

    for (int i = 0; i < task_count; ++i) {
        ASSERT_TRUE(
            queue.push([&active_tasks, &tasks_started, release] {
                const int active = active_tasks.fetch_add(
                    1,
                    std::memory_order_relaxed
                ) + 1;

                /*
                 * Once all tasks are active, signal the test thread.
                 * Since task_count equals worker_count, reaching this
                 * point means that all workers are executing tasks at
                 * the same time.
                 */
                if (active == task_count) {
                    tasks_started.set_value();
                }

                /*
                 * Keep the task active until the test has verified
                 * that all workers reached this point.
                 */
                release.wait();

                active_tasks.fetch_sub(1, std::memory_order_relaxed);
            })
        );
    }

    {
        acpf::ThreadPool pool(queue, worker_count);

        /*
         * Wait until all workers are executing a task concurrently.
         * A timeout is used deliberately: if the ThreadPool does not
         * execute all tasks concurrently, the future would never become
         * ready and an unconditional wait could block the test forever.
         */
        const auto status =
            all_tasks_started.wait_for(std::chrono::seconds(5));

        /*
         * Release the tasks before shutting down the queue.
         * The tasks are currently blocked on release.wait(). They must
         * be allowed to finish before the ThreadPool can join its workers.
         */
        if (status == std::future_status::ready) {
            release_tasks.set_value();
        }

        /*
         * The ThreadPool does not shut down the TaskQueue itself.
         * Therefore the queue must be shut down explicitly before the
         * ThreadPool is destroyed. This allows workers that return to
         * wait_and_pop() to terminate cleanly.
         */
        queue.shutdown();

        /*
         * Only now do we evaluate the result.
         * This ordering is intentional: ASSERT_* may abort the current
         * test function. The queue has already been shut down above, so
         * the ThreadPool destructor cannot leave workers blocked forever.
         */
        ASSERT_EQ(status, std::future_status::ready);
    }

    EXPECT_EQ(
        active_tasks.load(std::memory_order_relaxed),
        0
    );
}

TEST(ThreadPool, ReportsWorkerCount) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 4);

    EXPECT_EQ(pool.worker_count(), 4);

    queue.shutdown();
}