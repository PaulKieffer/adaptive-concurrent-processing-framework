#include <gtest/gtest.h>
#include <stdexcept>
#include "acpf/moving_average.hpp"


TEST(MovingAverage, ConstructsWithValidWindowSize) {
    acpf::MovingAverage<int> ma(1);
}

TEST(MovingAverage, RejectsWindowSizeZero) {
    EXPECT_THROW(
        acpf::MovingAverage<int> ma(0),
        std::invalid_argument
    );
}

TEST(MovingAverage, ReturnsZeroWhenEmpty) {
    acpf::MovingAverage<int> ma(2);
    EXPECT_DOUBLE_EQ(ma.value(), 0.0);
}

TEST(MovingAverage, CalculateseinAverageForPartialWindow) {
    acpf::MovingAverage<int> ma(4);
    ma.add(2);
    ma.add(3);
    EXPECT_DOUBLE_EQ(ma.value(), 2.5);
}

TEST(MovingAverage, ReplaceOldestSample) {
    acpf::MovingAverage<int> ma(2);
    ma.add(1);
    ma.add(2);
    EXPECT_DOUBLE_EQ(ma.value(), 1.5);
    ma.add(3);
    EXPECT_DOUBLE_EQ(ma.value(), 2.5);
}

