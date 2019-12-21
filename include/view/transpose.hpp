#pragma once

#include <type_traits>
#include <vector>

#include <range/v3/experimental/utility/generator.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/view/indirect.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/ref.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

template <typename Rng>
auto transpose(Rng && rng)
    -> ranges::experimental::generator<ranges::any_view<std::remove_reference_t<
        typename std::remove_reference_t<Rng>::value_type::value_type>>>
{
    using namespace ranges;
    using It =
        typename std::remove_reference_t<Rng>::value_type::const_iterator;
    auto iters = rng | views::transform([](auto const & r) { return begin(r); })
                 | to<std::vector<It>>();

    auto advance = [&] {
        bool is_end = true;
        for (auto && [it, r] : views::zip(views::ref(iters), rng))
            if (it != end(r) && ++it != end(r)) is_end = false;
        return !is_end;
    };

    do {
        co_yield views::zip(iters, rng) | views::remove_if([](auto const & p) {
            auto const & [it, r] = p;
            return it == end(r);
        }) | views::keys
            | views::indirect;
    } while (advance());
}
