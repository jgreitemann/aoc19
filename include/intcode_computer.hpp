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
    computer(memory_type && memory);
    computer(istream_construct_t, std::istream & mem_is);

    void run();
    memory_type const & memory() const { return m_memory; }
    memory_type & memory() { return m_memory; }

private:
    memory_type m_memory;
};

}  // namespace intcode
