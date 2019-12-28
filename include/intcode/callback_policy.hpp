#pragma once

namespace intcode::io_policy {

template <typename Int, typename WriteCallback, typename ReadCallback>
struct callback_policy
{
    callback_policy(WriteCallback && write_cb, ReadCallback && read_cb)
        : m_write_cb{std::move(write_cb)}, m_read_cb{std::move(read_cb)}
    {
    }
    void write(Int value) const { m_write_cb(value); }
    Int read() const { return m_read_cb(); }

private:
    WriteCallback m_write_cb;
    ReadCallback m_read_cb;
};

}  // namespace intcode::io_policy