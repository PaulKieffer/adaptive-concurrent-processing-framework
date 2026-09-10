#include "acpf/controller.hpp"

namespace acpf {

Controller::Controller(ThreadPool &pool) : queue_(pool.queue_), pool_(pool) {}

Controller::~Controller() {
    queue_.shutdown();
    pool_.wait_for_workers_to_stop();
    pool_.reap_stopped_workers();
}

void Controller::update() {
    pool_.reap_stopped_workers();
    // TODO: ma_queue_size_.add(queue_.size());
    // TODO: if workload and utilization is at optimum do nothing
    // TODO: else either reduce or increase workers
}
} // namespace acpf
