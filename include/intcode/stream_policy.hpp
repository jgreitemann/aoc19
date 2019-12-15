#pragma once

#include <iostream>

namespace intcode::io_policy {

struct stream_policy
{
    stream_policy(std::istream & is = std::cin, std::ostream & os = std::cout)
        : m_is(is), m_os(os)
    {
    }
    void write(int value) const { m_os << value << '\n'; }
    int read() const
    {
        if (&m_is == &std::cin) std::cerr << "Input: ";
        int res;
        m_is >> res;
        return res;
    }

private:
    std::istream & m_is;
    std::ostream & m_os;
};

}  // namespace intcode::io_policy