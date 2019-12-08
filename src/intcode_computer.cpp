#include <intcode_computer.hpp>

#include <range/v3/view/getlines.hpp>
#include <range/v3/view/transform.hpp>

#include <stdexcept>
#include <string>
#include <utility>

using namespace intcode;
using namespace ranges;

computer::computer(computer::memory_type && memory)
    : m_memory(std::move(memory))
{
}

computer::computer(istream_construct_t, std::istream & mem_is)
    : computer(ranges::getlines(mem_is, ',')
               | ranges::views::transform(
                   [](auto const & str) { return std::stoi(str); })
               | to<memory_type>())
{
}

void computer::run()
{
    auto process = [&](auto & it) {
        int opcode = *(it++);
        switch (opcode) {
            case 1:
                m_memory[it[2]] = m_memory[it[0]] + m_memory[it[1]];
                it += 3;
                return true;
            case 2:
                m_memory[it[2]] = m_memory[it[0]] * m_memory[it[1]];
                it += 3;
                return true;
            case 99:
                return false;
            default:
                throw std::runtime_error("invalid opcode");
                return false;
        }
    };
    for (auto ip = m_memory.begin(); process(ip);) {
    }
}
