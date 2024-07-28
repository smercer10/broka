#pragma once
#include "common.hpp"
#include "order.hpp"
#include <vector>

struct TradeSideInfo {
    OrderId orderId {};
    Price price {};
};

class Trade {
public:
    Trade(Quantity quantity, const TradeSideInfo& buySideInfo, const TradeSideInfo& sellSideInfo)
        : m_quantity { quantity }
        , m_buySideInfo { buySideInfo }
        , m_sellSideInfo { sellSideInfo }

    {
    }

    [[nodiscard]] auto quantity() const -> Quantity { return m_quantity; }
    [[nodiscard]] auto buySideInfo() const -> const TradeSideInfo& { return m_buySideInfo; }
    [[nodiscard]] auto sellSideInfo() const -> const TradeSideInfo& { return m_sellSideInfo; }

private:
    Quantity m_quantity;
    TradeSideInfo m_buySideInfo;
    TradeSideInfo m_sellSideInfo;
};

using Trades = std::vector<Trade>;
