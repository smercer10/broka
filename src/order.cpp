#include "order.hpp"
#include <stdexcept>

auto Order::fill(Quantity quantity) -> void
{
    if (quantity > m_remainingQuantity) {
        throw std::runtime_error { "Cannot fill more than remaining order quantity." };
    }
    m_remainingQuantity -= quantity;
}

auto Order::toIoc(Price price) -> void
{
    m_type = OrderType::ioc;
    m_price = price;
}
