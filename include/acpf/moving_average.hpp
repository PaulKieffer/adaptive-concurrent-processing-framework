#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace acpf {
/*
 * Maintains the moving average of a fixed-size window of samples.
 *
 * Samples are stored in a circular buffer. Once the window is full,
 * adding a new sample replaces the oldest sample.
 *
 * The moving average is updated in O(1) time using a running sum.
 *
 * If fewer samples than the configured window size have been added,
 * the average is calculated over the samples currently available.
 */
template <typename T> class MovingAverage {
  private:
    std::vector<T> samples_;
    std::size_t next_index_ = 0;
    std::size_t sample_count_ = 0;
    double prev_mavg_ = 0.0;
    T sum_{};

  public:
    /*
     * Creates a moving average with the specified window size.
     * Throws std::invalid_argument if window_size is zero.
     */
    explicit MovingAverage(std::size_t window_size) : samples_(window_size) {
        if (window_size == 0) {
            throw std::invalid_argument("MovingAverage requires a window size greater than zero");
        }
    }
    /*
     * Adds a sample to the moving average.
     * If the window is full, the oldest sample is replaced.
     */
    void add(T value) {
        prev_mavg_ = this->value();
        if (sample_count_ == samples_.size()) {
            sum_ -= samples_[next_index_];
        } else {
            ++sample_count_;
        }

        samples_[next_index_] = value;
        sum_ += value;

        next_index_ = (next_index_ + 1) % samples_.size();
    }
    /*
     * Returns the current moving average.
     * If no samples have been added, returns 0.0.
     */
    double value() const noexcept {
        if (sample_count_ == 0) {
            return 0.0;
        }

        return static_cast<double>(sum_) / sample_count_;
    }

    /*
     * Returns the difference between the current and the
     * last moving average.
     */
    double delta() const noexcept {
        return this->value() - prev_mavg_;
    }
};
} // namespace acpf