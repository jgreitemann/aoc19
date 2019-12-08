#pragma once
#include <iostream>
#include <vector>

namespace intcode {

struct istream_construct_t
{
};
inline constexpr istream_construct_t istream_construct;

struct computer
{
    using memory_type = std::vector<int>;
    computer(memory_type && memory, std::istream & is = std::cin,
             std::ostream & os = std::cout);
    computer(istream_construct_t, std::istream & mem_is,
             std::istream & is = std::cin, std::ostream & os = std::cout);

    void run();
    memory_type const & memory() const { return m_memory; }
    memory_type & memory() { return m_memory; }

private:
    memory_type m_memory;
    std::istream & m_is;
    std::ostream & m_os;
};

}  // namespace intcode
