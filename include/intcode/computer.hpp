#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <range/v3/view/getlines.hpp>
#include <range/v3/view/transform.hpp>

#include <intcode/stream_policy.hpp>

namespace intcode {

struct istream_construct_t
{
};
inline constexpr istream_construct_t istream_construct;

template <typename Int = int, typename IOPolicy = io_policy::stream_policy<Int>>
struct computer : private IOPolicy
{
    using memory_type = std::vector<Int>;

    template <typename... Args>
    computer(memory_type && memory, Args &&... args)
        : IOPolicy(std::forward<Args>(args)...), m_memory(std::move(memory))
    {
    }

    template <typename... Args>
    computer(istream_construct_t, std::istream & mem_is, Args &&... args)
        : computer(ranges::getlines(mem_is, ',')
                       | ranges::views::transform([](auto const & str) {
                             std::stringstream ss{str};
                             Int res;
                             ss >> res;
                             return res;
                         })
                       | ranges::to<memory_type>(),
                   std::forward<Args>(args)...)
    {
    }

    void run()
    {
        std::size_t rb = 0;
        auto process = [&](auto & it) {
            Int opcode = *(it++);
            char buf[7];
            sprintf(buf, "%.6i", static_cast<int>(opcode));
            auto access = [&](std::ptrdiff_t x) -> Int & {
                switch (buf[3 - x]) {
                    case '0':  // position mode
                    {
                        std::size_t idx = static_cast<std::size_t>(it[x]);
                        if (idx >= m_memory.size()) m_memory.resize(idx + 1);
                        return m_memory[idx];
                    }
                    case '1':  // immediate mode
                        return it[x];
                    case '2':  // relative mode
                    {
                        std::size_t idx =
                            rb + static_cast<std::ptrdiff_t>(it[x]);
                        if (idx >= m_memory.size()) m_memory.resize(idx + 1);
                        return m_memory[idx];
                    }
                    default:
                        throw std::runtime_error("unknown access mode");
                }
            };
            switch (static_cast<int>(opcode % 100)) {
                case 1:
                    access(0);
                    access(1);
                    access(2);
                    access(2) = access(0) + access(1);
                    it += 3;
                    return true;
                case 2:
                    access(0);
                    access(1);
                    access(2);
                    access(2) = access(0) * access(1);
                    it += 3;
                    return true;
                case 3:
                    access(0);
                    access(0) = IOPolicy::read();
                    it += 1;
                    return true;
                case 4:
                    access(0);
                    IOPolicy::write(access(0));
                    it += 1;
                    return true;
                case 5:
                    access(0);
                    access(1);
                    if (access(0))
                        it = m_memory.begin()
                             + static_cast<std::ptrdiff_t>(access(1));
                    else
                        it += 2;
                    return true;
                case 6:
                    access(0);
                    access(1);
                    if (!access(0))
                        it = m_memory.begin()
                             + static_cast<std::ptrdiff_t>(access(1));
                    else
                        it += 2;
                    return true;
                case 7:
                    access(0);
                    access(1);
                    access(2);
                    access(2) = access(0) < access(1);
                    it += 3;
                    return true;
                case 8:
                    access(0);
                    access(1);
                    access(2);
                    access(2) = access(0) == access(1);
                    it += 3;
                    return true;
                case 9:
                    access(0);
                    rb += static_cast<std::ptrdiff_t>(access(0));
                    it += 1;
                    return true;
                case 99:
                    return false;
                default:
                    throw std::runtime_error(
                        "invalid opcode: "
                        + std::to_string(static_cast<int>(opcode % 100)));
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
