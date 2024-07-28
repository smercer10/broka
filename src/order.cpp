#include "order.hpp"
#include <cassert>

auto Order::fill(Quantity quantity) -> void
{
    assert(quantity <= m_remainingQuantity);
    m_remainingQuantity -= quantity;
}

auto Order::toIoc(Price price) -> void
{
    assert(m_type == OrderType::market);
    m_type = OrderType::ioc;
    m_price = price;
}
