#include "order.hpp"
#include "order_book.hpp"
#include "gtest/gtest.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
TEST(OrderBookTest, placeGtcOrder)
{
    OrderBook orderBook;
    OrderPtr order1 { std::make_shared<Order>(1, OrderType::gtc, Side::buy, 99, 150) };
    OrderPtr order2 { std::make_shared<Order>(2, OrderType::gtc, Side::sell, 101, 25) };
    OrderPtr order3 { std::make_shared<Order>(3, OrderType::gtc, Side::sell, 100, 50) };
    OrderPtr order4 { std::make_shared<Order>(4, OrderType::gtc, Side::buy, 100, 125) };
    OrderPtr order5 { std::make_shared<Order>(5, OrderType::gtc, Side::sell, 99, 100) };

    auto trades { orderBook.placeOrder(order1) };
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(orderBook.size(), 1);

    trades = orderBook.placeOrder(order2);
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(orderBook.size(), 2);

    trades = orderBook.placeOrder(order3);
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(orderBook.size(), 3);

    trades = orderBook.placeOrder(order4);
    EXPECT_EQ(trades[0].quantity(), 50);
    EXPECT_EQ(trades[0].buySideInfo().orderId, 4);
    EXPECT_EQ(trades[0].sellSideInfo().orderId, 3);
    EXPECT_EQ(orderBook.size(), 3);
    EXPECT_TRUE(order3->isFilled());
    EXPECT_EQ(order4->remainingQuantity(), 75);

    trades = orderBook.placeOrder(order5);
    EXPECT_EQ(trades[0].quantity(), 75);
    EXPECT_EQ(trades[0].buySideInfo().orderId, 4);
    EXPECT_EQ(trades[0].sellSideInfo().orderId, 5);
    EXPECT_EQ(trades[1].quantity(), 25);
    EXPECT_EQ(trades[1].buySideInfo().orderId, 1);
    EXPECT_EQ(trades[1].sellSideInfo().orderId, 5);
    EXPECT_EQ(orderBook.size(), 2);
    EXPECT_TRUE(order4->isFilled());
    EXPECT_TRUE(order5->isFilled());
    EXPECT_EQ(order1->remainingQuantity(), 125);
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
