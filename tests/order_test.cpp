#include "order.hpp"
#include "gtest/gtest.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
TEST(OrderTest, fillOrder)
{
    Order order { 1, OrderType::gtc, Side::buy, 99, 200 };
    EXPECT_EQ(order.remainingQuantity(), 200);

    order.fill(50);
    EXPECT_EQ(order.remainingQuantity(), 150);
    EXPECT_FALSE(order.isFilled());

    order.fill(150);
    EXPECT_EQ(order.remainingQuantity(), 0);
    EXPECT_TRUE(order.isFilled());

    EXPECT_EQ(order.initialQuantity(), 200);
    EXPECT_DEATH(order.fill(1), ".*");
}

TEST(OrderTest, toIoc)
{
    Order order1 { 1, Side::sell, 150 };
    EXPECT_EQ(order1.type(), OrderType::market);
    EXPECT_EQ(order1.price(), Constants::invalidPrice);

    order1.toIoc(75);
    EXPECT_EQ(order1.type(), OrderType::ioc);
    EXPECT_EQ(order1.price(), 75);

    Order order2 { 2, OrderType::fok, Side::buy, 80, 100 };
    EXPECT_DEATH(order2.toIoc(50), ".*");
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
