#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main()
{
    using namespace ranges;

    auto memory = getlines(std::cin, ',') |
                  views::transform([](auto str) { return std::stoi(str); }) |
                  to<std::vector<int>>();

    auto run = [](int noun, int verb, std::vector<int> memory) {
        auto process = [&memory](auto & it) {
            int opcode = *(it++);
            switch (opcode) {
                case 1:
                    memory[it[2]] = memory[it[0]] + memory[it[1]];
                    it += 3;
                    return true;
                case 2:
                    memory[it[2]] = memory[it[0]] * memory[it[1]];
                    it += 3;
                    return true;
                case 99:
                    return false;
                default:
                    throw std::runtime_error("invalid opcode");
                    return false;
            }
        };

        memory[1] = noun;
        memory[2] = verb;
        for (auto ip = memory.begin(); process(ip);) {
        }
        return memory[0];
    };

    std::cout << "Part 1: " << run(12, 02, memory) << '\n';

    auto find_inputs = [&](int desired_result) {
        for (auto [noun, verb] : views::cartesian_product(views::iota(0, 100),
                                                          views::iota(0, 100)) |
                                     views::filter([&](auto pair) {
                                         auto [noun, verb] = pair;
                                         return run(noun, verb, memory) ==
                                                desired_result;
                                     }))
            return 100 * noun + verb;
        throw std::runtime_error(
            "could not find inputs matching desired result");
    };
    std::cout << "Part 2: " << find_inputs(19690720) << '\n';
}
