#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/sliding.hpp>
#include <range/v3/view/transform.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

inline constexpr auto to_pair = [](auto rng) {
    auto it = rng.begin();
    return std::pair{*it, *(++it)};
};
inline constexpr auto adjacent_pairs =
    ranges::views::sliding(2) | ranges::views::transform(to_pair);

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
              return any_of(str | adjacent_pairs,
                            pair_compare(std::equal_to<>{}));
          });

    std::cout << "Part 1: " << count_all(codes) << '\n';
}