#include "acpf/controller.hpp"

namespace acpf {

    Controller::Controller(ThreadPool& pool)
        : queue_(pool.queue_), pool_(pool) {}

    Controller::~Controller() {
        queue_.shutdown();
        pool_.wait_for_workers_to_stop();
        pool_.reap_stopped_workers();
    }

    void Controller::update() {
        pool_.reap_stopped_workers();
        // request queue size and recalculate ma's
        // if workload and utilization is at optimum do nothing
        // else either reduce or increase workers
    }
}
