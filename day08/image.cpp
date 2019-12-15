#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <range/v3/algorithm/count.hpp>
#include <range/v3/algorithm/min_element.hpp>
#include <range/v3/range/operations.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/drop_while.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>

int main()
{
    using namespace ranges;

    constexpr std::size_t width = 25;
    constexpr std::size_t height = 6;

    auto number_string =
        (getlines(std::cin) | to<std::vector<std::string>>()).front();
    auto layers = number_string | views::chunk(width * height);

    auto char_count = [](char c) {
        return [c](auto && rng) { return count(rng, c); };
    };

    auto && target_layer = *min_element(layers, std::less<>{}, char_count('0'));
    std::cout << "Part 1: "
              << count(target_layer, '1') * count(target_layer, '2') << '\n';

    auto render = views::transform([](char c) {
        switch (c) {
            case '0':
                return ' ';
            case '1':
                return '#';
            default:
                return 'o';
        }
    });

    std::cout << "Part 2: \n\n";
    for (auto && row : views::ints(0ul, width * height) | views::chunk(width)) {
        for (size_t offset : row) {
            auto layer_values = number_string | views::drop(offset)
                                | views::stride(width * height);
            auto top = layer_values
                       | views::drop_while([](char c) { return c == '2'; })
                       | views::take(1);
            if (empty(top))
                std::cout << 'o';
            else
                std::cout << *(top | render).begin();
        }
        std::cout << '\n';
    }
}