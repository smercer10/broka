#pragma once
#include "common.hpp"
#include "order.hpp"
#include "trade.hpp"
#include <map>
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
    auto cancelOrder(OrderId id) -> void;
    [[nodiscard]] auto levelsInfo() const -> OrderBookLevelsInfo;
    [[nodiscard]] auto placeOrder(const OrderPtr& order) -> Trades;
    [[nodiscard]] auto size() const -> std::size_t { return m_orders.size(); }
    [[nodiscard]] auto updateOrder(const OrderUpdate& update) -> Trades;

private:
    struct OrderEntry {
        OrderPtr order;
        Orders::iterator location; // Points to the order in the corresponding m_bids or m_asks orders deque.
    };

    std::map<Price, Orders, std::greater<>> m_bids;
    std::map<Price, Orders> m_asks;
    std::unordered_map<OrderId, OrderEntry> m_orders;

    [[nodiscard]] auto canFullyFillOrder(Side side, Price price, Quantity quantity) const -> bool;
    [[nodiscard]] auto canPartiallyFillOrder(Side side, Price price) const -> bool;
    [[nodiscard]] auto convertMarketOrder(const OrderPtr& order) -> bool;
    [[nodiscard]] auto matchOrders() -> Trades;
};
