#pragma once
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <range/v3/view/getlines.hpp>
#include <range/v3/view/transform.hpp>

namespace intcode {

namespace io_policy {

struct stream_policy
{
    stream_policy(std::istream & is = std::cin, std::ostream & os = std::cout);
    void write(int value) const;
    int read() const;

private:
    std::istream & m_is;
    std::ostream & m_os;
};

}  // namespace io_policy

struct istream_construct_t
{
};
inline constexpr istream_construct_t istream_construct;

template <typename IOPolicy = io_policy::stream_policy>
struct computer : private IOPolicy
{
    using memory_type = std::vector<int>;

    template <typename... Args>
    computer(memory_type && memory, Args &&... args)
        : IOPolicy(std::forward<Args>(args)...), m_memory(std::move(memory))
    {
    }

    template <typename... Args>
    computer(istream_construct_t, std::istream & mem_is, Args &&... args)
        : computer(ranges::getlines(mem_is, ',')
                       | ranges::views::transform(
                           [](auto const & str) { return std::stoi(str); })
                       | ranges::to<memory_type>(),
                   std::forward<Args>(args)...)
    {
    }

    void run()
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
                    access(0) = IOPolicy::read();
                    it += 1;
                    return true;
                case 4:
                    IOPolicy::write(access(0));
                    it += 1;
                    return true;
                case 5:
                    if (access(0))
                        it = m_memory.begin() + access(1);
                    else
                        it += 2;
                    return true;
                case 6:
                    if (!access(0))
                        it = m_memory.begin() + access(1);
                    else
                        it += 2;
                    return true;
                case 7:
                    access(2) = access(0) < access(1);
                    it += 3;
                    return true;
                case 8:
                    access(2) = access(0) == access(1);
                    it += 3;
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

    memory_type const & memory() const { return m_memory; }
    memory_type & memory() { return m_memory; }

private:
    memory_type m_memory;
};

}  // namespace intcode
