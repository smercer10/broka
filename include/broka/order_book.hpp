#pragma once
#include "common.hpp"
#include "order.hpp"
#include "trade.hpp"
#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

struct LevelInfo {
    Price price {};
    Quantity quantity {};
};

using LevelsInfo = std::vector<LevelInfo>;

class OrderBookLevelsInfo {
public:
    OrderBookLevelsInfo(LevelsInfo bidLevelsInfo, LevelsInfo askLevelsInfo)
        : m_bidLevelsInfo { std::move(bidLevelsInfo) }
        , m_askLevelsInfo { std::move(askLevelsInfo) }
    {
    }

    [[nodiscard]] auto bidLevelsInfo() const -> const LevelsInfo& { return m_bidLevelsInfo; }
    [[nodiscard]] auto askLevelsInfo() const -> const LevelsInfo& { return m_askLevelsInfo; }

private:
    LevelsInfo m_bidLevelsInfo;
    LevelsInfo m_askLevelsInfo;
};

class OrderBook {
public:
    OrderBook();
    ~OrderBook();

    // Prevent copying and moving to avoid concurrency complications.
    OrderBook(const OrderBook&) = delete;
    auto operator=(const OrderBook&) -> OrderBook& = delete;
    OrderBook(OrderBook&&) = delete;
    auto operator=(OrderBook&&) -> OrderBook& = delete;

    auto cancelOrder(OrderId id) -> void;
    [[nodiscard]] auto levelsInfo() const -> OrderBookLevelsInfo;
    [[nodiscard]] auto placeOrder(const OrderPtr& order) -> Trades;
    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto updateOrder(const OrderUpdate& update) -> Trades;

private:
    struct OrderEntry {
        OrderPtr order;
        Orders::iterator location; // Points to the order in the m_bids or m_asks orders deque.
    };

    std::map<Price, Orders, std::greater<>> m_bids;
    std::map<Price, Orders> m_asks;
    std::unordered_map<OrderId, OrderEntry> m_orders;

    mutable std::mutex m_mutex;
    std::condition_variable m_shutdownCond;
    std::atomic<bool> m_shutdown;
    std::thread m_temporalThread; // Runs in the background to check for expired day orders.

    auto cancelExpiredDayOrders() -> void;
    auto cancelOrders(const OrderIds& ids) -> void;

    // Should only be called when holding the lock.
    auto cancelOrderNoLock(OrderId id) -> void;
    [[nodiscard]] auto canFullyFillOrderNoLock(Side side, Price price, Quantity quantity) const -> bool;
    [[nodiscard]] auto canPartiallyFillOrderNoLock(Side side, Price price) const -> bool;
    [[nodiscard]] auto convertMarketOrderNoLock(const OrderPtr& order) -> bool;
    [[nodiscard]] auto levelsInfoNoLock() const -> OrderBookLevelsInfo;
    [[nodiscard]] auto matchOrdersNoLock() -> Trades;
    [[nodiscard]] auto placeOrderNoLock(const OrderPtr& order) -> Trades;
};
