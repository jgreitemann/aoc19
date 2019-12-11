#include <intcode_computer.hpp>

#include <utility>

using namespace intcode::io_policy;

stream_policy::stream_policy(std::istream & is, std::ostream & os)
    : m_is(is), m_os(os)
{
}

void stream_policy::write(int value) const { m_os << value << '\n'; }

int stream_policy::read() const
{
    if (&m_is == &std::cin) std::cerr << "Input: ";
    int res;
    m_is >> res;
    return res;
}
