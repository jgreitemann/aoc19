#pragma once

#include <iostream>

namespace intcode::io_policy {

template <typename Int>
struct stream_policy
{
    stream_policy(std::istream & is = std::cin, std::ostream & os = std::cout)
        : m_is(is), m_os(os)
    {
    }
    void write(Int value) const { m_os << value << '\n'; }
    Int read() const
    {
        if (&m_is == &std::cin) std::cerr << "Input: ";
        Int res;
        m_is >> res;
        return res;
    }

private:
    std::istream & m_is;
    std::ostream & m_os;
};

}  // namespace intcode::io_policy