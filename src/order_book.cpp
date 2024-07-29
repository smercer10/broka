#include "order.hpp"
#include "trade.hpp"
#include <chrono>
#include <mutex>
#include <order_book.hpp>

OrderBook::OrderBook()
    : m_shutdown { false }
    , m_temporalThread { &OrderBook::cancelExpiredDayOrders, this }
{
}

OrderBook::~OrderBook()
{
    m_shutdown.store(true, std::memory_order_release);
    m_shutdownCond.notify_one();
    if (m_temporalThread.joinable()) {
        m_temporalThread.join();
    }
}

auto OrderBook::cancelOrder(OrderId id) -> void
{
    std::lock_guard lock { m_mutex };
    cancelOrderNoLock(id);
}

auto OrderBook::levelsInfo() const -> OrderBookLevelsInfo
{
    std::lock_guard lock { m_mutex };

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
    std::lock_guard lock { m_mutex };
    return placeOrderNoLock(order);
}

auto OrderBook::size() const -> std::size_t
{
    std::lock_guard lock { m_mutex };
    return m_orders.size();
}

auto OrderBook::updateOrder(const OrderUpdate& update) -> Trades
{
    std::lock_guard lock { m_mutex };

    if (!m_orders.contains(update.id())) {
        return {};
    }

    const auto& existingOrder { m_orders[update.id()].order };
    cancelOrderNoLock(update.id());
    return placeOrderNoLock(update.toOrder(existingOrder->side(), existingOrder->type()));
}

auto OrderBook::cancelExpiredDayOrders() -> void
{
    using namespace std::chrono; // NOLINT(google-build-using-namespace)

    while (true) {
        const auto now { system_clock::now() };
        const auto today { floor<days>(now) };
        const auto marketClose { today + Constants::marketCloseHour };

        // If the market is already closed, wait until market close tomorrow.
        auto timeUntilClose = marketClose - now;
        if (timeUntilClose <= 0s) {
            timeUntilClose += 24h;
        }
        {
            std::unique_lock lock { m_mutex };
            if (m_shutdownCond.wait_for(lock, timeUntilClose, [this] { return m_shutdown.load(std::memory_order_acquire); })) {
                return;
            }
        }

        std::vector<OrderId> expiredOrders;
        {
            std::lock_guard lock { m_mutex };
            for (const auto& [id, entry] : m_orders) {
                if (entry.order->type() == OrderType::day) {
                    expiredOrders.emplace_back(id);
                }
            }
        }
        cancelOrders(expiredOrders);
    }
}

auto OrderBook::cancelOrders(const OrderIds& ids) -> void
{
    std::lock_guard lock { m_mutex };
    for (const auto id : ids) {
        cancelOrderNoLock(id);
    }
}

auto OrderBook::cancelOrderNoLock(OrderId id) -> void
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

auto OrderBook::canFullyFillOrderNoLock(Side side, Price price, Quantity quantity) const -> bool
{
    auto orderBookLevelsInfo { levelsInfo() };
    const auto& levelsInfo { side == Side::buy ? orderBookLevelsInfo.askLevelsInfo() : orderBookLevelsInfo.bidLevelsInfo() };

    Quantity totalQuantity { 0 };

    for (const auto& levelInfo : levelsInfo) {
        if (side == Side::buy && levelInfo.price > price) {
            break;
        }
        if (side == Side::sell && levelInfo.price < price) {
            break;
        }

        totalQuantity += levelInfo.quantity;
        if (totalQuantity >= quantity) {
            return true;
        }
    }
    return false;
}

auto OrderBook::canPartiallyFillOrderNoLock(Side side, Price price) const -> bool
{
    if (side == Side::buy) {
        return !m_asks.empty() && m_asks.begin()->first <= price;
    }
    if (side == Side::sell) {
        return !m_bids.empty() && m_bids.begin()->first >= price;
    }
    return false; // Should be unreachable.
}

auto OrderBook::convertMarketOrderNoLock(const OrderPtr& order) -> bool
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

auto OrderBook::matchOrdersNoLock() -> Trades
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

auto OrderBook::placeOrderNoLock(const OrderPtr& order) -> Trades
{
    Trades trades;

    if (m_orders.contains(order->id())) {
        return trades;
    }
    if (order->type() == OrderType::market && !convertMarketOrderNoLock(order)) {
        return trades;
    }
    if (order->type() == OrderType::fok && !canFullyFillOrderNoLock(order->side(), order->price(), order->initialQuantity())) {
        return trades;
    }
    if (order->type() == OrderType::ioc && !canPartiallyFillOrderNoLock(order->side(), order->price())) {
        return trades;
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

    trades = matchOrdersNoLock();

    if (order->type() == OrderType::ioc && !order->isFilled()) {
        cancelOrderNoLock(order->id());
    }

    return trades;
}
