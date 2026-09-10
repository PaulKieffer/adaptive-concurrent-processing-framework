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

TEST(MovingAverage, CalculatesAverageForPartialWindow) {
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

TEST(MovingAverage, CalculatesDelta) {
    acpf::MovingAverage<int> ma(3);
    EXPECT_DOUBLE_EQ(ma.delta(), 0.0);
    ma.add(10);
    EXPECT_DOUBLE_EQ(ma.delta(), 10.0);
    ma.add(20);
    EXPECT_DOUBLE_EQ(ma.delta(), 5.0);
    ma.add(30);
    EXPECT_DOUBLE_EQ(ma.delta(), 5.0);
    ma.add(40);
    EXPECT_DOUBLE_EQ(ma.delta(), 10.0);
}

TEST(MovingAverage, SupportsSizeT) {
    acpf::MovingAverage<std::size_t> ma(3);
    EXPECT_DOUBLE_EQ(ma.value(), 0.0);
    ma.add(10);
    EXPECT_DOUBLE_EQ(ma.value(), 10.0);
    EXPECT_DOUBLE_EQ(ma.delta(), 10.0);
    ma.add(20);
    EXPECT_DOUBLE_EQ(ma.value(), 15.0);
    EXPECT_DOUBLE_EQ(ma.delta(), 5.0);
    ma.add(30);
    EXPECT_DOUBLE_EQ(ma.value(), 20.0);
    EXPECT_DOUBLE_EQ(ma.delta(), 5.0);
    ma.add(40);
    EXPECT_DOUBLE_EQ(ma.value(), 30.0);
    EXPECT_DOUBLE_EQ(ma.delta(), 10.0);
}

TEST(MovingAverage, SupportsDouble) {
    acpf::MovingAverage<double> ma(3);
    EXPECT_DOUBLE_EQ(ma.value(), 0.0);
    ma.add(10.0);
    EXPECT_DOUBLE_EQ(ma.value(), 10.0);
    EXPECT_DOUBLE_EQ(ma.delta(), 10.0);
    ma.add(20.0);
    EXPECT_DOUBLE_EQ(ma.value(), 15.0);
    EXPECT_DOUBLE_EQ(ma.delta(), 5.0);
    ma.add(30.0);
    EXPECT_DOUBLE_EQ(ma.value(), 20.0);
    EXPECT_DOUBLE_EQ(ma.delta(), 5.0);
    ma.add(40.0);
    EXPECT_DOUBLE_EQ(ma.value(), 30.0);
    EXPECT_DOUBLE_EQ(ma.delta(), 10.0);
}