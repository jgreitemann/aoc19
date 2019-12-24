#pragma once

#include <range/v3/experimental/utility/generator.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/sliding.hpp>
#include <range/v3/view/transform.hpp>

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

inline constexpr auto to_pair = [](auto && rng) {
    auto it = rng.begin();
    using T = decltype(*it);
    return std::pair<T, T>{*it, *(++it)};
};
inline constexpr auto as_pairs =
    ranges::views::chunk(2) | ranges::views::transform(to_pair);
inline constexpr auto adjacent_pairs =
    ranges::views::sliding(2) | ranges::views::transform(to_pair);

template <typename Rng, typename T = std::reference_wrapper<
                            typename std::remove_reference_t<Rng>::value_type>>
auto pairs_gen(Rng && rng) -> ranges::experimental::generator<std::pair<T, T>>
{
    for (auto it1 = begin(rng); it1 != end(rng); ++it1)
        for (auto it2 = std::next(it1); it2 != end(rng); ++it2)
            co_yield std::pair<T, T>{*it1, *it2};
}

template <typename Rng, typename T = std::add_lvalue_reference_t<
                            typename std::remove_reference_t<Rng>::value_type>>
auto pairs(Rng && rng)
{
    using namespace ranges;
    return pairs_gen(std::forward<Rng>(rng))
           | views::transform([](auto && p) -> std::pair<T, T> { return {p}; });
}