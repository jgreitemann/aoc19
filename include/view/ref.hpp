#pragma once
#include <range/v3/view/transform.hpp>

#include <type_traits>

inline constexpr auto lvalue_ref = ranges::views::transform(
    [](auto && x) -> std::add_lvalue_reference_t<decltype(x)> { return x; });
