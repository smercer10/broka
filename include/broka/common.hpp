#pragma once
#include <chrono>

using Price = unsigned int;
using Quantity = unsigned int;

namespace Constants {
inline constexpr Price invalidPrice { 0 };
inline constexpr auto marketCloseHour { std::chrono::hours { 16 } };
} // namespace Constants
