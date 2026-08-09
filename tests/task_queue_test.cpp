#include <atomic>
#include <thread>
#include <gtest/gtest.h>
#include "acpf/task_queue.hpp"


TEST(TaskQueue, CanStoreAndRetrieveTask) {
    acpf::TaskQueue queue;
    bool executed = false; 
    queue.push([&executed] { executed = true; });
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