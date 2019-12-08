#pragma once
#include <range/v3/experimental/utility/generator.hpp>
#include <type_traits>

template <typename Rng>
auto iterators(Rng && rng) -> ranges::experimental::generator<
    std::remove_reference_t<decltype(rng.begin())>>
{
    for (auto it = rng.begin(); it != rng.end(); ++it) co_yield it;
}
