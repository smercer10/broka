#include <order_book.hpp>

auto OrderBook::canMatch(Side side, Price price) const -> bool
{
    switch (side) {
    case Side::buy:
        return !m_sellOrders.empty() && m_sellOrders.begin()->first <= price;
    case Side::sell:
        return !m_buyOrders.empty() && m_buyOrders.begin()->first >= price;
    }
}
