#include <intcode_computer.hpp>

#include <range/v3/view/getlines.hpp>
#include <range/v3/view/transform.hpp>

#include <stdexcept>
#include <string>
#include <utility>

using namespace intcode;
using namespace ranges;

computer::computer(computer::memory_type && memory, std::istream & is,
                   std::ostream & os)
    : m_memory(std::move(memory)), m_is(is), m_os(os)
{
}

computer::computer(istream_construct_t, std::istream & mem_is,
                   std::istream & is, std::ostream & os)
    : computer(ranges::getlines(mem_is, ',')
                   | ranges::views::transform(
                       [](auto const & str) { return std::stoi(str); })
                   | to<memory_type>(),
               is, os)
{
}

void computer::run()
{
    auto process = [&](auto & it) {
        int opcode = *(it++);
        char buf[7];
        sprintf(buf, "%.6i", opcode);
        auto access = [&](int x) -> int & {
            if (buf[3 - x] == '0')  // position mode
                return m_memory[it[x]];
            else  // immediate mode
                return it[x];
        };
        switch (opcode % 100) {
            case 1:
                access(2) = access(0) + access(1);
                it += 3;
                return true;
            case 2:
                access(2) = access(0) * access(1);
                it += 3;
                return true;
            case 3:
                std::cerr << "Input: ";
                m_is >> access(0);
                it += 1;
                return true;
            case 4:
                m_os << access(0) << '\n';
                it += 1;
                return true;
            case 99:
                return false;
            default:
                throw std::runtime_error("invalid opcode: "
                                         + std::to_string(opcode % 100));
                return false;
        }
    };
    for (auto ip = m_memory.begin(); process(ip);) {
    }
}
