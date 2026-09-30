#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>

#include "acpf/moving_average.hpp"
#include "acpf/task_queue.hpp"
#include "acpf/thread_pool.hpp"

namespace acpf {
/*
 * Coordinates the lifecycle of a TaskQueue and its associated ThreadPool.
 *
 * The Controller holds non-owning references to the ThreadPool and its
 * associated TaskQueue. The TaskQueue reference is obtained from the
 * ThreadPool.
 *
 * The Controller defines the active lifetime of the associated processing
 * system. When the Controller is destroyed, the TaskQueue is shut down,
 * all workers are allowed to finish their current tasks and stop, and
 * stopped workers are reaped.
 *
 * Running tasks are not forcibly interrupted. Destruction may therefore
 * block until all currently executing tasks have completed.
 */
class Controller {
  private:
    TaskQueue &queue_;
    ThreadPool &pool_;
    MovingAverage<std::size_t> ma_queue_size_;
    double lower_threshold_;
    double upper_threshold_;
    std::size_t update_interval_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;

    // void update(); // TODO: make update private

  public:
    /*
     * Creates a Controller for the specified ThreadPool.
     * The Controller uses the TaskQueue associated with the ThreadPool.
     */
    Controller(ThreadPool &pool, std::size_t ma_window_size, double lower_threshold,
               double upper_threshold, std::size_t update_interval);
    /*
     * Ends the active lifetime of the associated processing system.
     *
     * The TaskQueue is shut down and all workers are allowed to finish before
     * they are reaped. After destruction, the associated ThreadPool has no
     * active workers and its TaskQueue no longer accepts new tasks.
     *
     * Running tasks are not forcibly interrupted. Destruction may therefore
     * block until all currently executing tasks have completed.
     */
    ~Controller();
    // TODO: once fully implemented form contract
    void update();
};
} // namespace acpf
