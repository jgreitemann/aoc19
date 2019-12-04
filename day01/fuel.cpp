#include <range/v3/experimental/utility/generator.hpp>
#include <range/v3/numeric.hpp>
#include <range/v3/view/istream.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

#include <iostream>
#include <vector>

namespace coro_v3 = ranges::experimental;

int fuel(int mass) { return std::max(0, mass / 3 - 2); }

coro_v3::generator<int> rocket_fuel(int mass)
{
    while ((mass = fuel(mass))) co_yield mass;
}

int main()
{
    using namespace ranges;
    using namespace std::placeholders;
    auto sum = std::bind(accumulate, _1, 0);
    auto const in = istream_view<int>{std::cin} | to<std::vector<int>>();

    std::cout << "Part 1: " << sum(in | views::transform(fuel)) << '\n';

    std::cout << "Part 2: "
              << sum(in | views::transform(rocket_fuel) | views::join) << '\n';
}