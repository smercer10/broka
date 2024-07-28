#pragma once
#include "common.hpp"
#include "order.hpp"

struct TradeSideInfo {
    OrderId orderId {};
    Price price {};
    Quantity quantity {};
};

class Trade {
public:
    Trade(const TradeSideInfo& buySideInfo, const TradeSideInfo& sellSideInfo)
        : m_buySideInfo { buySideInfo }
        , m_sellSideInfo { sellSideInfo }
    {
    }

    [[nodiscard]] auto buySideInfo() const -> const TradeSideInfo& { return m_buySideInfo; }
    [[nodiscard]] auto sellSideInfo() const -> const TradeSideInfo& { return m_sellSideInfo; }

private:
    TradeSideInfo m_buySideInfo;
    TradeSideInfo m_sellSideInfo;
};

using Trades = std::vector<Trade>;
