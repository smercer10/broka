#include "order.hpp"
#include <gtest/gtest.h>

TEST(OrderTest, doNothing)
{
    Order order {};
    order.doNothing();
    EXPECT_TRUE(true);
}
