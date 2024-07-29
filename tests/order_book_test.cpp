#include "order.hpp"
#include "order_book.hpp"
#include "gtest/gtest.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
TEST(OrderBookTest, placeFokOrder)
{
    OrderBook orderBook;
    OrderPtr order1 { std::make_shared<Order>(1, OrderType::fok, Side::buy, 99, 150) };
    OrderPtr order2 { std::make_shared<Order>(2, OrderType::gtc, Side::buy, 99, 50) };
    OrderPtr order3 { std::make_shared<Order>(3, OrderType::fok, Side::sell, 99, 51) };
    OrderPtr order4 { std::make_shared<Order>(4, OrderType::fok, Side::sell, 99, 30) };
    OrderPtr order5 { std::make_shared<Order>(5, OrderType::fok, Side::sell, 99, 20) };

    auto trades { orderBook.placeOrder(order1) };
    EXPECT_EQ(orderBook.size(), 0);
    EXPECT_EQ(order1->remainingQuantity(), 150);

    trades = orderBook.placeOrder(order2);
    EXPECT_EQ(orderBook.size(), 1);

    trades = orderBook.placeOrder(order3);
    EXPECT_EQ(orderBook.size(), 1);
    EXPECT_EQ(order3->remainingQuantity(), 51);

    trades = orderBook.placeOrder(order4);
    EXPECT_EQ(orderBook.size(), 1);
    EXPECT_EQ(trades[0].quantity(), 30);
    EXPECT_EQ(trades[0].buySideInfo().orderId, 2);
    EXPECT_EQ(trades[0].sellSideInfo().orderId, 4);
    EXPECT_TRUE(order4->isFilled());
    EXPECT_EQ(order2->remainingQuantity(), 20);

    trades = orderBook.placeOrder(order5);
    EXPECT_EQ(orderBook.size(), 0);
    EXPECT_EQ(trades[0].quantity(), 20);
    EXPECT_EQ(trades[0].buySideInfo().orderId, 2);
    EXPECT_EQ(trades[0].sellSideInfo().orderId, 5);
    EXPECT_TRUE(order2->isFilled());
    EXPECT_TRUE(order5->isFilled());
}

TEST(OrderBookTest, placeGtcOrder)
{
    OrderBook orderBook;
    OrderPtr order1 { std::make_shared<Order>(1, OrderType::gtc, Side::buy, 99, 150) };
    OrderPtr order2 { std::make_shared<Order>(2, OrderType::gtc, Side::sell, 101, 25) };
    OrderPtr order3 { std::make_shared<Order>(3, OrderType::gtc, Side::sell, 100, 50) };
    OrderPtr order4 { std::make_shared<Order>(4, OrderType::gtc, Side::buy, 100, 125) };
    OrderPtr order5 { std::make_shared<Order>(5, OrderType::gtc, Side::sell, 99, 100) };

    auto trades { orderBook.placeOrder(order1) };
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

TEST(OrderBookTest, placeIocOrder)
{
    OrderBook orderBook;
    OrderPtr order1 { std::make_shared<Order>(2, OrderType::ioc, Side::buy, 98, 150) };
    OrderPtr order2 { std::make_shared<Order>(2, OrderType::gtc, Side::buy, 99, 50) };
    OrderPtr order3 { std::make_shared<Order>(3, OrderType::ioc, Side::sell, 101, 25) };
    OrderPtr order4 { std::make_shared<Order>(4, OrderType::ioc, Side::sell, 99, 100) };
    OrderPtr order5 { std::make_shared<Order>(5, OrderType::ioc, Side::sell, 99, 15) };

    auto trades { orderBook.placeOrder(order1) };
    EXPECT_EQ(orderBook.size(), 0);
    EXPECT_EQ(order1->remainingQuantity(), 150);

    trades = orderBook.placeOrder(order2);
    EXPECT_EQ(orderBook.size(), 1);

    trades = orderBook.placeOrder(order3);
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(orderBook.size(), 1);
    EXPECT_EQ(order3->remainingQuantity(), 25);

    trades = orderBook.placeOrder(order4);
    EXPECT_EQ(trades[0].quantity(), 50);
    EXPECT_EQ(trades[0].buySideInfo().orderId, 2);
    EXPECT_EQ(trades[0].sellSideInfo().orderId, 4);
    EXPECT_EQ(orderBook.size(), 0);
    EXPECT_TRUE(order2->isFilled());
    EXPECT_EQ(order4->remainingQuantity(), 50);

    trades = orderBook.placeOrder(order5);
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(orderBook.size(), 0);
    EXPECT_EQ(order5->remainingQuantity(), 15);
}

TEST(OrderBookTest, placeMarketOrder)
{
    OrderBook orderBook;
    OrderPtr order1 { std::make_shared<Order>(0, Side::buy, 150) };
    OrderPtr order2 { std::make_shared<Order>(2, OrderType::gtc, Side::buy, 10, 20) };
    OrderPtr order3 { std::make_shared<Order>(3, OrderType::gtc, Side::sell, 500, 50) };
    OrderPtr order4 { std::make_shared<Order>(4, OrderType::gtc, Side::sell, 400, 25) };
    OrderPtr order5 { std::make_shared<Order>(5, Side::sell, 30) };
    OrderPtr order6 { std::make_shared<Order>(6, Side::buy, 100) };

    auto trades { orderBook.placeOrder(order1) };
    EXPECT_EQ(order1->type(), OrderType::market);
    EXPECT_EQ(orderBook.size(), 0);
    EXPECT_EQ(order1->remainingQuantity(), 150);

    trades = orderBook.placeOrder(order2);
    EXPECT_EQ(orderBook.size(), 1);

    trades = orderBook.placeOrder(order3);
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(orderBook.size(), 2);

    trades = orderBook.placeOrder(order4);
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(orderBook.size(), 3);

    trades = orderBook.placeOrder(order5);
    EXPECT_EQ(order5->type(), OrderType::ioc);
    EXPECT_EQ(trades[0].quantity(), 20);
    EXPECT_EQ(trades[0].buySideInfo().orderId, 2);
    EXPECT_EQ(trades[0].buySideInfo().price, 10);
    EXPECT_EQ(trades[0].sellSideInfo().orderId, 5);
    EXPECT_EQ(trades[0].sellSideInfo().price, 10);
    EXPECT_EQ(orderBook.size(), 2);
    EXPECT_TRUE(order2->isFilled());
    EXPECT_EQ(order5->remainingQuantity(), 10);

    trades = orderBook.placeOrder(order6);
    EXPECT_EQ(order5->type(), OrderType::ioc);
    EXPECT_EQ(trades[0].quantity(), 25);
    EXPECT_EQ(trades[0].buySideInfo().orderId, 6);
    EXPECT_EQ(trades[0].buySideInfo().price, 500);
    EXPECT_EQ(trades[0].sellSideInfo().orderId, 4);
    EXPECT_EQ(trades[0].sellSideInfo().price, 400);
    EXPECT_EQ(trades[1].quantity(), 50);
    EXPECT_EQ(trades[1].buySideInfo().orderId, 6);
    EXPECT_EQ(trades[1].buySideInfo().price, 500);
    EXPECT_EQ(trades[1].sellSideInfo().orderId, 3);
    EXPECT_EQ(trades[1].sellSideInfo().price, 500);
    EXPECT_EQ(orderBook.size(), 0);
    EXPECT_TRUE(order3->isFilled());
    EXPECT_TRUE(order4->isFilled());
    EXPECT_EQ(order6->remainingQuantity(), 25);
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
