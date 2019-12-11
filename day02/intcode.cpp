#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <intcode_computer.hpp>

int main()
{
    using namespace ranges;

    intcode::computer comp(intcode::istream_construct, std::cin);

    auto eval = [](int noun, int verb, intcode::computer<> comp) {
        comp.memory()[1] = noun;
        comp.memory()[2] = verb;
        comp.run();
        return comp.memory()[0];
    };

    std::cout << "Part 1: " << eval(12, 02, comp) << '\n';

    auto find_inputs = [&](int desired_result) {
        for (auto [noun, verb] :
             views::cartesian_product(views::iota(0, 100), views::iota(0, 100))
                 | views::filter([&](auto pair) {
                       auto [noun, verb] = pair;
                       return eval(noun, verb, comp) == desired_result;
                   }))
            return 100 * noun + verb;
        throw std::runtime_error(
            "could not find inputs matching desired result");
    };
    std::cout << "Part 2: " << find_inputs(19690720) << '\n';
}
