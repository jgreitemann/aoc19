#pragma once

#include <type_traits>
#include <utility>

#include <range/v3/view/transform.hpp>

inline constexpr auto graph = [](auto && f) {
    return ranges::views::transform(
        [f = std::forward<std::remove_reference_t<decltype(f)>>(f)](
            auto && val) {
            auto && res = f(val);
            return std::pair{
                std::forward<std::remove_reference_t<decltype(val)>>(val),
                std::forward<std::remove_reference_t<decltype(res)>>(res)};
        });
};
