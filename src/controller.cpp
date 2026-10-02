#include <chrono>
#include <iostream>

#include "acpf/controller.hpp"

namespace acpf {

Controller::Controller(ThreadPool &pool, std::size_t ma_window_size, double lower_threshold,
                       double upper_threshold, std::size_t update_interval)
    : queue_(pool.queue_), pool_(pool), ma_queue_size_(ma_window_size),
      lower_threshold_(lower_threshold), upper_threshold_(upper_threshold),
      update_interval_(update_interval), scheduler_thread_{&Controller::scheduler, this} {
    std::cout << "controller contructor\n";
}

Controller::~Controller() {
    queue_.shutdown();
    pool_.wait_for_workers_to_stop();
    pool_.reap_stopped_workers();
    Controller::stop();
    scheduler_thread_.join();
}

void Controller::update() {
    pool_.reap_stopped_workers();
    std::cout << "queue size: " << queue_.size() << "\n";
    std::cout << "worker count: " << pool_.worker_count() << "\n";
    std::cout << "ma: " << ma_queue_size_.value() << "\n";
    std::cout << "delta: " << ma_queue_size_.delta() << "\n";
    ma_queue_size_.add(queue_.size());
    // eval policy - reduce/increase worker count
    if (ma_queue_size_.value() > 0) {
        if (ma_queue_size_.delta() < lower_threshold_) {
            pool_.reduce_workers(1);
        } else if (ma_queue_size_.delta() > upper_threshold_) {
            pool_.increase_workers(1);
        } // if delta elem of [lower_thshold_, upper_threshold], do nothing
    } else {
        if (ma_queue_size_.delta() == 0) {
            std::cout << "reduce workers\n";
            pool_.reduce_workers(1);
        }
    }
}

void Controller::scheduler() {
    std::unique_lock<std::mutex> lock(mutex_);

    while (!stop_) {
        const bool should_stop = cv_.wait_for(lock, std::chrono::milliseconds(update_interval_),
                                              [this] { return stop_; });

        if (should_stop) {
            break;
        }

        lock.unlock();
        update();
        lock.lock();
    }
}

void Controller::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }

    cv_.notify_one();
}
} // namespace acpf
