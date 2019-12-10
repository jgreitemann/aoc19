#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/experimental/utility/generator.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/transform.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <view/pair.hpp>

namespace coro_v3 = ranges::experimental;

template <typename Compare>
auto pair_compare(Compare && compare)
{
    return [cmp = std::forward<Compare>(compare)](auto const & pair) -> bool {
        return cmp(pair.first, pair.second);
    };
}

template <typename Rng>
auto count_all(Rng && rng)
{
    return ranges::count_if(std::forward<Rng>(rng), [](auto) { return true; });
}

template <typename Rng>
auto multiplicities(Rng && rng) -> coro_v3::generator<std::size_t>
{
    using T = typename std::remove_reference_t<Rng>::value_type;
    std::size_t streak = 0;
    for (auto const & [a, b] : rng | adjacent_pairs) {
        ++streak;
        if (a != b) {
            co_yield streak;
            streak = 0;
        }
    }
    co_yield ++streak;
}

int main(int argc, const char * argv[])
{
    using namespace ranges;
    if (argc != 3)
        throw std::invalid_argument("invalid number of CL arguments");
    int min = std::atoi(argv[1]);
    int max = std::atoi(argv[2]) + 1;

    auto codes =
        views::iota(min, max)
        | views::transform(static_cast<std::string (&)(int)>(std::to_string))
        | views::filter([](std::string const & str) {
              return all_of(str | adjacent_pairs,
                            pair_compare(std::less_equal<>{}));
          })
        | views::filter([](std::string const & str) {
              return any_of(multiplicities(str),
                            [](size_t m) { return m >= 2; });
          });
    std::cout << "Part 1: " << count_all(codes) << '\n';

    auto excl_codes =
        codes | views::filter([](std::string const & str) {
            return any_of(multiplicities(str), [](size_t m) { return m == 2; });
        });
    std::cout << "Part 2: " << count_all(excl_codes) << '\n';
}