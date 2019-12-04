#include <range/v3/view/getlines.hpp>
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
    memory[1] = 12;
    memory[2] = 02;

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

    for (auto ip = memory.begin(); process(ip);) {
    }

    std::cout << memory[0] << '\n';
}