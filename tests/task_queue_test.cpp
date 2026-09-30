#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "acpf/task_queue.hpp"

TEST(TaskQueue, CanStoreAndRetrieveTask) {
    acpf::TaskQueue queue;
    bool executed = false;
    ASSERT_TRUE(queue.push_back([&executed] { executed = true; }));
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
    ASSERT_TRUE(queue.push_back([&executed] { ++executed; }));
    ASSERT_TRUE(queue.push_back([&executed] { ++executed; }));
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
    EXPECT_FALSE(queue.push_back([&executed] { executed = true; }));
    EXPECT_FALSE(queue.wait_and_pop(task));
    EXPECT_FALSE(executed);
}

TEST(TaskQueue, MultipleProducers) {
    acpf::TaskQueue queue;

    constexpr int producer_count = 4;
    constexpr int tasks_per_producer = 100;
    constexpr int expected_tasks = producer_count * tasks_per_producer;

    std::atomic<int> rejected_tasks = 0;
    std::atomic<int> executed_tasks = 0;

    std::vector<std::thread> producers;
    producers.reserve(producer_count);

    for (int i = 0; i < producer_count; ++i) {
        producers.emplace_back([&queue, &rejected_tasks, &executed_tasks, &tasks_per_producer] {
            for (int j = 0; j < tasks_per_producer; ++j) {
                const bool accepted = queue.push_back(
                    [&executed_tasks] { executed_tasks.fetch_add(1, std::memory_order_relaxed); });

                if (!accepted) {
                    rejected_tasks.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto &producer : producers) {
        producer.join();
    }

    queue.shutdown();

    acpf::Task task;
    int retrieved_tasks = 0;

    while (queue.wait_and_pop(task)) {
        task();
        ++retrieved_tasks;
    }

    EXPECT_EQ(rejected_tasks.load(), 0);
    EXPECT_EQ(retrieved_tasks, expected_tasks);
    EXPECT_EQ(executed_tasks.load(), expected_tasks);
}

TEST(TaskQueue, MultipleConsumer) {
    acpf::TaskQueue queue;

    constexpr int consumer_count = 4;
    constexpr int tasks = 400;
    constexpr int expected_tasks = tasks;

    std::atomic<int> executed_tasks = 0;

    for (int i = 0; i < tasks; ++i) {
        ASSERT_TRUE(queue.push_back([&executed_tasks] { ++executed_tasks; }));
    }

    queue.shutdown();

    std::vector<std::thread> consumers;
    consumers.reserve(consumer_count);

    for (int i = 0; i < consumer_count; ++i) {
        consumers.emplace_back([&queue] {
            acpf::Task task;

            while (queue.wait_and_pop(task)) {
                task();
            }
        });
    }

    for (auto &consumer : consumers) {
        consumer.join();
    }

    EXPECT_EQ(executed_tasks, expected_tasks);
}

TEST(TaskQueue, MultiProducerMultiConsumer) {
    acpf::TaskQueue queue;

    constexpr int producer_count = 2;
    constexpr int consumer_count = 2;
    constexpr int tasks_per_producer = 100;
    constexpr int expected_tasks = producer_count * tasks_per_producer;

    std::atomic<int> rejected_tasks = 0;
    std::atomic<int> executed_tasks = 0;

    std::vector<std::thread> producers;
    producers.reserve(producer_count);
    std::vector<std::thread> consumers;
    consumers.reserve(consumer_count);

    for (int i = 0; i < consumer_count; ++i) {
        consumers.emplace_back([&queue] {
            acpf::Task task;

            while (queue.wait_and_pop(task)) {
                task();
            }
        });
    }

    for (int i = 0; i < producer_count; ++i) {
        producers.emplace_back([&queue, &rejected_tasks, &executed_tasks, &tasks_per_producer] {
            for (int j = 0; j < tasks_per_producer; ++j) {
                const bool accepted = queue.push_back(
                    [&executed_tasks] { executed_tasks.fetch_add(1, std::memory_order_relaxed); });

                if (!accepted) {
                    rejected_tasks.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto &producer : producers) {
        producer.join();
    }

    queue.shutdown();

    for (auto &consumer : consumers) {
        consumer.join();
    }

    EXPECT_EQ(rejected_tasks.load(), 0);
    EXPECT_EQ(executed_tasks.load(), expected_tasks);
}

TEST(TaskQueue, ReportsSize) {
    acpf::TaskQueue queue;

    EXPECT_EQ(queue.size(), 0);

    ASSERT_TRUE(queue.push_back([] {}));
    EXPECT_EQ(queue.size(), 1);

    acpf::Task task;
    ASSERT_TRUE(queue.wait_and_pop(task));
    EXPECT_EQ(queue.size(), 0);

    queue.shutdown();
}