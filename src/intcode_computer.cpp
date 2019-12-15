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

condition_variable_policy::condition_variable_policy(channel_type & in_channel,
                                                     channel_type & out_channel)
    : m_in(in_channel), m_out(out_channel)
{
}

void condition_variable_policy::channel_type::write(int val)
{
    {
        std::unique_lock lock{mutex};
        cv.wait(lock, [this] { return !active; });
        value = val;
        active = true;
    }
    cv.notify_all();
}

int condition_variable_policy::channel_type::read()
{
    int res = [&] {
        std::unique_lock lock{mutex};
        cv.wait(lock, [this] { return active; });
        active = false;
        return value;
    }();
    cv.notify_all();
    return res;
}
