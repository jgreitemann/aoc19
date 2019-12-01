#include <range/v3/numeric.hpp>
#include <range/v3/view/istream.hpp>
#include <range/v3/view/transform.hpp>

#include <iostream>

int main()
{
    using namespace ranges;
    auto in = istream_view<int>{std::cin};
    std::cout << accumulate(in | views::transform(
                                     [](int mass) { return mass / 3 - 2; }),
                            0)
              << '\n';
}