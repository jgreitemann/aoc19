#pragma once
#include <range/v3/experimental/utility/generator.hpp>

#include <algorithm>
#include <type_traits>
#include <vector>

template <typename Rng>
auto permutations(Rng && rng) -> ranges::experimental::generator<
    std::vector<std::remove_reference_t<decltype(*rng.begin())>> const &>
{
    auto v =
        rng
        | ranges::to<
            std::vector<std::remove_reference_t<decltype(*rng.begin())>>>();
    do {
        co_yield v;
    } while (std::next_permutation(v.begin(), v.end()));
}
