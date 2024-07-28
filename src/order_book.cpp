#include "trade.hpp"
#include <order_book.hpp>

auto OrderBook::canMatchOrder(Side side, Price price) const -> bool
{
    switch (side) {
    case Side::buy:
        return !m_asks.empty() && m_asks.begin()->first <= price;
    case Side::sell:
        return !m_bids.empty() && m_bids.begin()->first >= price;
    default:
        return false; // Should be unreachable.
    }
}

auto OrderBook::matchOrders() -> Trades
{
    Trades trades;
    trades.reserve(std::min(m_bids.size(), m_asks.size())); // Prevents unnecessary reallocations.

    while (true) {
        if (m_bids.empty() || m_asks.empty()) {
            break;
        }

        auto& [bestBidPrice, buyOrders] { *m_bids.begin() };
        auto& [bestAskPrice, sellOrders] { *m_asks.begin() };

        if (bestBidPrice < bestAskPrice) {
            break;
        }

        while (!buyOrders.empty() && !sellOrders.empty()) {
            auto earliestBuyOrder { buyOrders.front() };
            auto earliestSellOrder { sellOrders.front() };

            auto tradeQuantity { std::min(earliestBuyOrder->remainingQuantity(), earliestSellOrder->remainingQuantity()) };

            auto earliestBuyOrderId { earliestBuyOrder->id() };
            auto earliestSellOrderId { earliestSellOrder->id() };

            trades.emplace_back(
                tradeQuantity,
                TradeSideInfo { earliestBuyOrderId, bestBidPrice },
                TradeSideInfo { earliestSellOrderId, bestAskPrice });

            earliestBuyOrder->fill(tradeQuantity);
            earliestSellOrder->fill(tradeQuantity);

            if (earliestBuyOrder->isFilled()) {
                m_orders.erase(earliestBuyOrderId);
                buyOrders.pop_front();
            }

            if (earliestSellOrder->isFilled()) {
                m_orders.erase(earliestSellOrderId);
                sellOrders.pop_front();
            }

            if (buyOrders.empty()) {
                m_bids.erase(bestBidPrice);
            }

            if (sellOrders.empty()) {
                m_asks.erase(bestAskPrice);
            }
        }
    }

    for (auto& bid : m_bids) {
        auto& remainingBuyOrders = bid.second;
        cancelIocOrders(remainingBuyOrders);
    }

    for (auto& ask : m_asks) {
        auto& remainingSellOrders = ask.second;
        cancelIocOrders(remainingSellOrders);
    }

    return trades;
}

auto OrderBook::cancelIocOrders(Orders& orders) -> void
{
    for (auto it { orders.begin() }; it != orders.end();) {
        auto& order { *it };

        if (order->type() == OrderType::ioc) {
            m_orders.erase(order->id());
            it = orders.erase(it);
            continue;
        }

        ++it;
    }
}
