#include <array>
#include <cmath>
#include <complex>
#include <functional>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <range/v3/action/sort.hpp>
#include <range/v3/action/stable_sort.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/cycle.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/remove.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/single.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <view/pair.hpp>
#include <view/transpose.hpp>

int main()
{
    using namespace ranges;
    using point_t = std::array<int, 2>;

    auto lines = getlines(std::cin) | to<std::vector<std::string>>();
    const int width = lines.front().size();
    const int height = lines.size();

    auto coords = views::zip(lines | views::join,
                             views::cartesian_product(views::ints(0, height),
                                                      views::ints(0, width))
                                 | views::transform([](auto && p) {
                                       auto [y, x] = p;
                                       return point_t{x, y};
                                   }))
                  | views::filter([](auto && p) { return p.first == '#'; })
                  | views::values | to<std::set<point_t>>();

    auto line_of_sight = [&coords](auto const & p) {
        auto const & [first, second] = p;
        point_t diff{second[0] - first[0], second[1] - first[1]};
        int gcd = std::gcd(diff[0], diff[1]);
        diff = {diff[0] / gcd, diff[1] / gcd};

        auto xstride = [](int a, int b, int s) -> any_view<int> {
            if (a == b) return views::single(a) | views::cycle;
            if (s < 0)
                return views::ints(b - s, a) | views::stride(-s)
                       | views::reverse;
            return views::ints(a + s, b) | views::stride(s);
        };

        return all_of(views::zip_with(
                          [](int x, int y) -> point_t {
                              return {x, y};
                          },
                          xstride(first[0], second[0], diff[0]),
                          xstride(first[1], second[1], diff[1])),
                      [&](point_t const & pt) {
                          return coords.find(pt) == coords.end();
                      });
    };

    auto coord_pairs = views::cartesian_product(coords, coords)
                       | views::filter([](auto const & p) {
                             auto const & [first, second] = p;
                             return first < second;
                         })
                       | views::filter(line_of_sight)
                       | views::transform([](auto && p) {
                             auto && [first, second] = p;
                             return std::array{first, second};
                         })
                       | to<std::vector<std::array<point_t, 2>>>();

    auto los_mset = coord_pairs | views::join | to<std::multiset<point_t>>();

    auto best_location = *max_element(
        los_mset, std::less<>{},
        [&los_mset](point_t const & p) { return los_mset.count(p); });

    std::cout << "Part 1:\n\tBest location: (" << best_location[0] << ", "
              << best_location[1] << ")\n\t# Asteroids with line of sight: "
              << los_mset.count(best_location) << '\n';

    auto distance = [best_location](point_t p) {
        p[0] -= best_location[0];
        p[1] -= best_location[1];
        return std::abs(p[0]) + std::abs(p[1]);
    };

    auto angle = [best_location](point_t p) {
        p[0] -= best_location[0];
        p[1] -= best_location[1];
        std::complex<double> c(p[1], -p[0] - 1e-9);
        return arg(c);
    };

    auto angle_approx_equiv = [angle](point_t lhs, point_t rhs) {
        return std::abs(angle(lhs) - angle(rhs)) < 1e-8;
    };
    auto approx_less = [](double a, double b) {
        return a < b && std::abs(a - b) > 1e-6;
    };

    auto coords_vec =
        coords | views::remove(best_location) | to<std::vector<point_t>>();

    coords_vec |= actions::sort(std::less<>{}, distance)
                  | actions::stable_sort(approx_less, angle);
    auto angle_groups = coords_vec | views::group_by(angle_approx_equiv)
                        | to<std::vector<std::vector<point_t>>>();

    std::cout << "Part 2:\n...\n";
    for (auto [i, p] : transpose(angle_groups) | views::join | views::enumerate
                           | views::drop(195) | views::take(5))
        std::cout << i + 1 << ": (" << p[0] << ", " << p[1] << ")\n";
}