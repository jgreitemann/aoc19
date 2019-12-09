#include <range/v3/action/sort.hpp>
#include <range/v3/action/stable_sort.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/experimental/utility/generator.hpp>
#include <range/v3/numeric.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/istream.hpp>
#include <range/v3/view/set_algorithm.hpp>
#include <range/v3/view/tokenize.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <cmath>
#include <functional>
#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <view/graph.hpp>
#include <view/pair.hpp>

namespace coro_v3 = ranges::experimental;

using point_t = std::pair<int, int>;
inline constexpr point_t origin{0, 0};

template <typename PairRng>
coro_v3::generator<point_t> wire_points(PairRng const & rng)
{
    point_t pos = origin;
    co_yield pos;
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
    using enum_point_t = std::pair<std::ptrdiff_t, point_t>;

    auto wires = to_pair(
        getlines(std::cin) | views::transform([](std::string const & line) {
            auto wp =
                wire_points(
                    line
                    | views::tokenize(std::regex{"([UDLR])([0-9]+)"}, {1, 2})
                    | as_pairs | views::transform([](auto const & pair) {
                          auto const & [sm_char, sm_int] = pair;
                          return std::pair{*sm_char.first,
                                           std::stoi(sm_int.str())};
                      }))
                | views::enumerate | to<std::vector<enum_point_t>>();
            wp |= actions::stable_sort(std::less<>{}, &enum_point_t::second);
            wp |= actions::unique(std::equal_to<>{}, &enum_point_t::second);
            return wp;
        }));

    auto crossings =
        views::set_intersection(wires.first, wires.second, {},
                                &enum_point_t::second, &enum_point_t::second)
        | to<std::vector<enum_point_t>>();

    // Part 1
    auto manhattan = [](auto point) {
        auto [x, y] = point;
        return std::abs(x) + std::abs(y);
    };
    using graph_pair_t = std::pair<point_t, int>;
    auto manhattan_graph =
        crossings | views::transform(&enum_point_t::second) | graph(manhattan)
        | to<std::vector<graph_pair_t>>()
        | actions::sort(std::less<>{}, &graph_pair_t::second);

    for (auto [point, dist] : manhattan_graph) {
        auto [x, y] = point;
        std::cout << x << ", " << y << ": " << dist << '\n';
    }

    std::cout << "------------------\n";

    // Part 2
    auto steps2 =
        views::set_intersection(wires.second, wires.first, {},
                                &enum_point_t::second, &enum_point_t::second)
        | views::transform(&enum_point_t::first) | to<std::vector<int>>();

    auto combined = views::zip(crossings, steps2)
                    | views::transform([](auto pair) {
                          auto [ep, step2] = pair;
                          auto [step1, point] = ep;
                          return std::pair{point, step1 + step2};
                      })
                    | to<std::vector<graph_pair_t>>()
                    | actions::sort(std::less<>{}, &graph_pair_t::second);
    for (auto [point, step] : combined) {
        auto [x, y] = point;
        std::cout << x << ", " << y << ": " << step << '\n';
    }
}