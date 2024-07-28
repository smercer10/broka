#pragma once
#include "common.hpp"
#include <memory>
#include <vector>

using OrderId = int;

enum class OrderType {
    fillOrKill,
    goodTillCancel,
};

enum class Side {
    buy,
    sell,
};

class Order {
public:
    Order(OrderId id, OrderType type, Side side, Price price, Quantity quantity)
        : m_id { id }
        , m_type { type }
        , m_side { side }
        , m_price { price }
        , m_initialQuantity { quantity }
        , m_remainingQuantity { quantity }
    {
    }

    [[nodiscard]] auto id() const -> OrderId { return m_id; }
    [[nodiscard]] auto type() const -> OrderType { return m_type; }
    [[nodiscard]] auto side() const -> Side { return m_side; }
    [[nodiscard]] auto price() const -> Price { return m_price; }
    [[nodiscard]] auto initialQuantity() const -> Quantity { return m_initialQuantity; }
    [[nodiscard]] auto remainingQuantity() const -> Quantity { return m_remainingQuantity; }
    [[nodiscard]] auto filledQuantity() const -> Quantity { return m_initialQuantity - m_remainingQuantity; }

    auto fill(Quantity quantity) -> void;

private:
    OrderId m_id;
    OrderType m_type;
    Side m_side;
    Price m_price;
    Quantity m_initialQuantity;
    Quantity m_remainingQuantity;
};

using OrderPtr = std::shared_ptr<Order>;
using Orders = std::vector<OrderPtr>;

class AdjustableOrder {
public:
    AdjustableOrder(OrderId id, Price price, Quantity quantity)
        : m_id { id }
        , m_price { price }
        , m_quantity { quantity }
    {
    }

    [[nodiscard]] auto id() const -> OrderId { return m_id; }
    [[nodiscard]] auto price() const -> Price { return m_price; }
    [[nodiscard]] auto quantity() const -> Quantity { return m_quantity; }

    [[nodiscard]] auto toOrder(Side side, OrderType type) const -> OrderPtr
    {
        return std::make_shared<Order>(m_id, type, side, m_price, m_quantity);
    }

private:
    OrderId m_id;
    Price m_price;
    Quantity m_quantity;
};
