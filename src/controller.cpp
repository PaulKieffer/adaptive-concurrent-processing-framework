#include "acpf/controller.hpp"

namespace acpf {

    Controller::Controller(TaskQueue& queue, ThreadPool& pool)
        : queue_(queue), pool_(pool) {}

    Controller::~Controller() {
        queue_.shutdown();
        pool_.wait_for_workers_to_stop();
        pool_.reap_stopped_workers();
    }
}
