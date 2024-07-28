#include "order.hpp"
#include "trade.hpp"
#include <order_book.hpp>

auto OrderBook::changeOrder(OrderChange change) -> Trades
{
    if (!m_orders.contains(change.id())) {
        return {};
    }

    const auto existingOrder { m_orders[change.id()].order };
    cancelOrder(change.id());
    return placeOrder(change.toOrder(existingOrder->side(), existingOrder->type()));
}

auto OrderBook::cancelOrder(OrderId id) -> void
{
    if (!m_orders.contains(id)) {
        return;
    }

    const auto& [order, it] { m_orders[id] };

    if (order->side() == Side::buy) {
        auto& buyOrders { m_bids[order->price()] };
        buyOrders.erase(it);
        if (buyOrders.empty()) {
            m_bids.erase(order->price());
        }
    } else {
        auto& sellOrders { m_asks[order->price()] };
        sellOrders.erase(it);
        if (sellOrders.empty()) {
            m_asks.erase(order->price());
        }
    }
}

auto OrderBook::placeOrder(OrderPtr order) -> Trades
{
    Trades trades;
    auto shouldCancelAfter { false };

    if (m_orders.contains(order->id())) {
        return trades;
    }

    if (order->type() == OrderType::ioc) {
        if (!canMatchOrder(order->side(), order->price())) {
            return trades;
        }
        shouldCancelAfter = true;
    }

    Orders::iterator it;

    if (order->side() == Side::buy) {
        auto& buyOrders { m_bids[order->price()] };
        buyOrders.emplace_back(order);
        it = --buyOrders.end();

    } else {
        auto& sellOrders { m_asks[order->price()] };
        sellOrders.emplace_back(order);
        it = --sellOrders.end();
    }

    m_orders.emplace(order->id(), OrderEntry { order, it });

    trades = matchOrders();

    if (shouldCancelAfter) {
        cancelOrder(order->id());
    }

    return trades;
}

auto OrderBook::canMatchOrder(Side side, Price price) const -> bool
{
    if (side == Side::buy) {
        return !m_asks.empty() && m_asks.begin()->first <= price;
    }
    if (side == Side::sell) {
        return !m_bids.empty() && m_bids.begin()->first >= price;
    }
    return false; // Should be unreachable.
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

    return trades;
}
