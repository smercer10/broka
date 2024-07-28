#pragma once
#include <bits/chrono.h>

using Price = int;
using Quantity = int;

namespace Constants {
inline constexpr Price invalidPrice { -1 };
inline constexpr auto marketCloseHour { std::chrono::hours { 16 } };
} // namespace Constants
