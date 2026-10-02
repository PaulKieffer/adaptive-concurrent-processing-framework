#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "acpf/controller.hpp"
#include "acpf/task_queue.hpp"
#include "acpf/thread_pool.hpp"

TEST(Controller, ShutsDownAllWorkers) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 1, 1);
    acpf::Controller controller(pool, 10, 0, 0, 10);
}

TEST(Controller, ControllerReducesWorkerOnEmptyQueue) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 4, 4);
    acpf::Controller controller(pool, 10, 10, 20, 10);
    // nothing added to queue -> queue always empty
    EXPECT_EQ(pool.worker_count(), 4);
    struct timespec waiting_period = {0, 25000000}; // 25 ms
    // at least two update-cycles are needed, hence (wait > 20 ms)
    nanosleep(&waiting_period, NULL); // wait for 25 ms
    EXPECT_EQ(pool.worker_count(), 3);
}

TEST(Controller, MinOneWorkerAlive) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 1, 1);
    acpf::Controller controller(pool, 10, 10, 20, 10);
    struct timespec waiting_period = {0, 25000000}; // 25 ms
    // at least two update-cycles are needed, hence (wait > 20 ms)
    nanosleep(&waiting_period, NULL); // wait for 25 ms
    EXPECT_EQ(pool.worker_count(), 1);
}

TEST(Controller, ControllerIncreaseWorkersOnExceededThreshold) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 1, 4);
    acpf::Controller controller(pool, 2, 1, 1, 10);
    queue.push_back([] { sleep(2); });
    queue.push_back([] { sleep(1); });
    queue.push_back([] { sleep(1); });
    struct timespec waiting_period = {0, 25000000}; // 25 ms
    // at least two update-cycles are needed, hence (wait > 20 ms)
    nanosleep(&waiting_period, NULL); // wait for 25 ms
    EXPECT_EQ(pool.worker_count(), 2);
}

TEST(Controller, SchedulerInitiatesUpdates) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 2, 4);
    acpf::Controller controller(pool, 4, 1, 2, 10);
    struct timespec waiting_period = {0, 25000000}; // 25 ms
    // at least two update-cycles are needed, hence (wait > 20 ms)
    nanosleep(&waiting_period, NULL); // wait for 25 ms
    EXPECT_EQ(pool.worker_count(), 1);
    // t.join();
}