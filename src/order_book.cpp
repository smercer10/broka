#include "order.hpp"
#include "trade.hpp"
#include <order_book.hpp>

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

auto OrderBook::levelsInfo() const -> OrderBookLevelsInfo
{
    LevelsInfo bidsInfo;
    LevelsInfo asksInfo;

    // Ensures no further reallocations.
    bidsInfo.reserve(m_bids.size());
    asksInfo.reserve(m_asks.size());

    auto createLevelInfo = [](Price price, const Orders& orders) {
        Quantity totalQuantity { 0 };

        for (const auto& order : orders) {
            totalQuantity += order->remainingQuantity();
        }

        return LevelInfo { price, totalQuantity };
    };

    for (const auto& [price, orders] : m_bids) {
        bidsInfo.emplace_back(createLevelInfo(price, orders));
    }

    for (const auto& [price, orders] : m_asks) {
        asksInfo.emplace_back(createLevelInfo(price, orders));
    }

    return OrderBookLevelsInfo { bidsInfo, asksInfo };
}

auto OrderBook::placeOrder(const OrderPtr& order) -> Trades
{
    Trades trades;
    auto shouldCancelAfter { false };

    if (m_orders.contains(order->id())) {
        return trades;
    }

    if (order->type() == OrderType::market && !convertMarketOrder(order)) {
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

auto OrderBook::updateOrder(const OrderUpdate& update) -> Trades
{
    if (!m_orders.contains(update.id())) {
        return {};
    }

    const auto& existingOrder { m_orders[update.id()].order };
    cancelOrder(update.id());
    return placeOrder(update.toOrder(existingOrder->side(), existingOrder->type()));
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

auto OrderBook::convertMarketOrder(const OrderPtr& order) -> bool
{
    if (order->side() == Side::buy && !m_asks.empty()) {
        const auto worstAskPrice { m_asks.rbegin()->first };
        order->toIoc(worstAskPrice);
        return true;
    }
    if (order->side() == Side::sell && !m_bids.empty()) {
        const auto worstBidPrice { m_bids.rbegin()->first };
        order->toIoc(worstBidPrice);
        return true;
    }
    return false;
}

auto OrderBook::matchOrders() -> Trades
{
    Trades trades;
    trades.reserve(std::min(m_bids.size(), m_asks.size())); // Ensures no further reallocations.

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
            auto& earliestBuyOrder { buyOrders.front() };
            auto& earliestSellOrder { sellOrders.front() };

            auto tradeQuantity { std::min(earliestBuyOrder->remainingQuantity(), earliestSellOrder->remainingQuantity()) };

            trades.emplace_back(
                tradeQuantity,
                TradeSideInfo { earliestBuyOrder->id(), bestBidPrice },
                TradeSideInfo { earliestSellOrder->id(), bestAskPrice });

            earliestBuyOrder->fill(tradeQuantity);
            earliestSellOrder->fill(tradeQuantity);

            if (earliestBuyOrder->isFilled()) {
                m_orders.erase(earliestBuyOrder->id());
                buyOrders.pop_front();
            }

            if (earliestSellOrder->isFilled()) {
                m_orders.erase(earliestSellOrder->id());
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
