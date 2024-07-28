#pragma once
#include "common.hpp"
#include "order.hpp"
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
private:
    struct OrderEntry {
        OrderPtr order;
        Orders::iterator location;
    };

    std::map<Price, Orders, std::greater<>> m_buyOrders;
    std::map<Price, Orders> m_sellOrders;
    std::unordered_map<OrderId, OrderEntry> m_orders;
};
