#pragma once

#include <condition_variable>
#include <mutex>

namespace intcode::io_policy {

template <typename Int>
struct condition_variable_policy
{
    struct channel_type
    {
        void write(Int val)
        {
            {
                std::unique_lock lock{mutex};
                cv.wait(lock, [this] { return !active; });
                value = val;
                active = true;
            }
            cv.notify_all();
        }
        Int read()
        {
            Int res = [&] {
                std::unique_lock lock{mutex};
                cv.wait(lock, [this] { return active; });
                active = false;
                return value;
            }();
            cv.notify_all();
            return res;
        }

    private:
        Int value;
        bool active = false;
        std::mutex mutex;
        std::condition_variable cv;
    };

    condition_variable_policy(channel_type & in_channel,
                              channel_type & out_channel)
        : m_in(in_channel), m_out(out_channel)
    {
    }
    void write(Int value) const { m_out.write(value); }
    Int read() const { return m_in.read(); }

private:
    channel_type & m_in;
    channel_type & m_out;
};

}  // namespace intcode::io_policy