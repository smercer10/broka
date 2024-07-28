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
    OrderBookLevelsInfo(LevelsInfo bidsInfo, LevelsInfo asksInfo)
        : m_bidsInfo { std::move(bidsInfo) }
        , m_asksInfo { std::move(asksInfo) }
    {
    }

    [[nodiscard]] auto bidsInfo() const -> const LevelsInfo& { return m_bidsInfo; }
    [[nodiscard]] auto asksInfo() const -> const LevelsInfo& { return m_asksInfo; }

private:
    LevelsInfo m_bidsInfo;
    LevelsInfo m_asksInfo;
};

class OrderBook {
public:
    auto cancelOrder(OrderId id) -> void;
    [[nodiscard]] auto placeOrder(OrderPtr order) -> Trades;

private:
    struct OrderEntry {
        OrderPtr order;
        Orders::iterator location;
    };

    std::map<Price, Orders, std::greater<>> m_bids;
    std::map<Price, Orders> m_asks;
    std::unordered_map<OrderId, OrderEntry> m_orders;

    [[nodiscard]] auto canMatchOrder(Side side, Price price) const -> bool;
    [[nodiscard]] auto matchOrders() -> Trades;

    auto cancelIocOrders(Orders& orders) -> void;
};
