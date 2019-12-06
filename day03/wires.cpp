#include <range/v3/action/sort.hpp>
#include <range/v3/experimental/utility/generator.hpp>
#include <range/v3/numeric.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/istream.hpp>
#include <range/v3/view/set_algorithm.hpp>
#include <range/v3/view/tokenize.hpp>
#include <range/v3/view/transform.hpp>

#include <cmath>
#include <functional>
#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace coro_v3 = ranges::experimental;

inline constexpr auto to_pair = [](auto rng) {
    auto it = rng.begin();
    return std::pair{*it, *(++it)};
};
inline constexpr auto as_pairs =
    ranges::views::chunk(2) | ranges::views::transform(to_pair);
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

using point_t = std::pair<int, int>;
inline constexpr point_t origin{0, 0};

template <typename PairRng>
coro_v3::generator<point_t> wire_points(PairRng const & rng)
{
    point_t pos = origin;
    for (auto [dir, dist] : rng) {
        switch (dir) {
            case 'U':
                while (dist-- > 0) {
                    pos.second++;
                    co_yield pos;
                }
                break;
            case 'D':
                while (dist-- > 0) {
                    pos.second--;
                    co_yield pos;
                }
                break;
            case 'R':
                while (dist-- > 0) {
                    pos.first++;
                    co_yield pos;
                }
                break;
            case 'L':
                while (dist-- > 0) {
                    pos.first--;
                    co_yield pos;
                }
                break;
            default:
                break;
        }
    }
}

int main()
{
    using namespace ranges;

    auto wires = to_pair(
        getlines(std::cin) | views::transform([](std::string const & line) {
            return wire_points(
                       line
                       | views::tokenize(std::regex{"([UDLR])([0-9]+)"}, {1, 2})
                       | as_pairs | views::transform([](auto const & pair) {
                             auto const & [sm_char, sm_int] = pair;
                             return std::pair{*sm_char.first,
                                              std::stoi(sm_int.str())};
                         }))
                   | to<std::set<point_t>>();
        }));

    auto manhattan = [](auto point) {
        auto [x, y] = point;
        return std::abs(x) + std::abs(y);
    };

    using crossing_pair_t = std::pair<std::pair<int, int>, int>;
    auto crossings = views::set_intersection(wires.first, wires.second)
                     | graph(manhattan) | to<std::vector<crossing_pair_t>>()
                     | actions::sort(std::less<>{}, &crossing_pair_t::second);

    for (auto [point, dist] : crossings) {
        auto [x, y] = point;
        std::cout << x << ", " << y << ": " << dist << '\n';
    }
}