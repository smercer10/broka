#include "order.hpp"
#include <stdexcept>

void Order::fill(Quantity quantity)
{
    if (quantity > m_remainingQuantity) {
        throw std::runtime_error { "Cannot fill more than remaining order quantity." };
    }
    m_remainingQuantity -= quantity;
}
