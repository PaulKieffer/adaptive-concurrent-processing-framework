#include <atomic>
#include <thread>
#include <gtest/gtest.h>
#include "acpf/task_queue.hpp"


TEST(TaskQueue, CanStoreAndRetrieveTask) {
    acpf::TaskQueue queue;
    bool executed = false; 
    ASSERT_TRUE(queue.push([&executed] { executed = true; }));
    acpf::Task task; 
    ASSERT_TRUE(queue.wait_and_pop(task)); 
    task(); 
    EXPECT_TRUE(executed);
}

TEST(TaskQueue, ShutdownUnblocksWaitingConsumer) {
    acpf::TaskQueue queue;
    std::atomic<bool> result = true;
    std::thread worker([&queue, &result] {
        acpf::Task task;
        result = queue.wait_and_pop(task);
    });
    queue.shutdown();
    worker.join();
    EXPECT_FALSE(result);
}

TEST(TaskQueue, PendingTasksSurviveShutdown) {
    acpf::TaskQueue queue;
    int executed = 0;
    ASSERT_TRUE(queue.push([&executed] { ++executed; }));
    ASSERT_TRUE(queue.push([&executed] { ++executed; }));
    acpf::Task task;
    queue.shutdown();
    ASSERT_TRUE(queue.wait_and_pop(task));
    task();
    ASSERT_TRUE(queue.wait_and_pop(task));
    task();
    EXPECT_FALSE(queue.wait_and_pop(task));
    EXPECT_EQ(executed, 2);
}

TEST(TaskQueue, RejectPushAfterShutdown) {
    acpf::TaskQueue queue;
    bool executed = false;
    acpf::Task task;
    queue.shutdown();
    EXPECT_FALSE(queue.push([&executed] { executed = true; }));
    EXPECT_FALSE(queue.wait_and_pop(task));
    EXPECT_FALSE(executed);
}