#pragma once

#include <range/v3/view/chunk.hpp>
#include <range/v3/view/sliding.hpp>
#include <range/v3/view/transform.hpp>

#include <utility>

inline constexpr auto to_pair = [](auto rng) {
    auto it = rng.begin();
    return std::pair{*it, *(++it)};
};
inline constexpr auto as_pairs =
    ranges::views::chunk(2) | ranges::views::transform(to_pair);
inline constexpr auto adjacent_pairs =
    ranges::views::sliding(2) | ranges::views::transform(to_pair);
