#include <gtest/gtest.h>
#include <unistd.h>

#include "acpf/controller.hpp"
#include "acpf/task_queue.hpp"
#include "acpf/thread_pool.hpp"

TEST(Controller, ShutsDownAllWorkers) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 1, 1);
    acpf::Controller controller(pool, 10, 0, 0, 10);
}

TEST(Controller, UpdateReducesWorkerOnNoChangeInQueue) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 2, 2);
    acpf::Controller controller(pool, 10, 10, 20, 10);
    // nothing added to queue -> queue always empty
    controller.update();
    sleep(0.01); // simulates waiting period between updates
    EXPECT_EQ(pool.worker_count(), 2);
    controller.update();
    sleep(0.01);
    EXPECT_EQ(pool.worker_count(), 1);
}

TEST(Controller, MinOneWorkerAlive) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 1, 1);
    acpf::Controller controller(pool, 10, 10, 20, 10);
    controller.update();
    sleep(0.01); // simulates waiting period between updates
    EXPECT_EQ(pool.worker_count(), 1);
}

TEST(Controller, UpdateIncreaseWorkersOnFullQueue) {
    acpf::TaskQueue queue;
    acpf::ThreadPool pool(queue, 1, 2);
    acpf::Controller controller(pool, 1, 1, 1, 10);
    queue.push_back([] { sleep(2); });
    queue.push_back([] { sleep(2); });
    sleep(0.01);
    controller.update();
    queue.push_back([] { sleep(1); });
    queue.push_back([] { sleep(1); });
    queue.push_back([] { sleep(1); });
    sleep(0.01);
    controller.update();
    queue.push_back([] { sleep(1); });
    queue.push_back([] { sleep(1); });
    queue.push_back([] { sleep(1); });
    sleep(0.01);
    controller.update();
    EXPECT_EQ(pool.worker_count(), 2);
}