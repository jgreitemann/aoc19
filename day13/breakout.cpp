#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/range/operations.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/remove.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/transform.hpp>

#include <intcode/computer.hpp>
#include <intcode/condition_variable_policy.hpp>

enum class Tile : long
{
    empty = 0,
    wall = 1,
    block = 2,
    paddle = 3,
    ball = 4
};

char render(Tile t)
{
    switch (t) {
        case Tile::empty:
            return ' ';
        case Tile::wall:
            return 'W';
        case Tile::block:
            return '#';
        case Tile::paddle:
            return '-';
        case Tile::ball:
            return 'o';
    }
}

using cv_policy = intcode::io_policy::condition_variable_policy<long>;
using channel_t = cv_policy::channel_type;
using screen_t = std::vector<std::vector<Tile>>;

struct arcade_policy
{
    arcade_policy(screen_t & screen, channel_t & out_channel)
        : m_screen{screen}, m_out{out_channel}
    {
    }

    long read() const
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        auto get_first_pos = [this](Tile t) {
            std::ptrdiff_t not_found =
                std::numeric_limits<std::ptrdiff_t>::max();
            using namespace ranges;
            for (std::ptrdiff_t x :
                 m_screen | views::transform([&](auto const & row) {
                     auto pos = find(row, t);
                     if (pos == end(row)) return not_found;
                     return pos - begin(row);
                 }) | views::remove(not_found))
                return x;
            return not_found;
        };
        auto ball_pos = get_first_pos(Tile::ball);
        auto paddle_pos = get_first_pos(Tile::paddle);
        if (ball_pos < paddle_pos) return -1;
        if (ball_pos > paddle_pos) return 1;
        return 0;
    }

    void write(long value) const { m_out.write(value); }

private:
    screen_t const & m_screen;
    channel_t & m_out;
};
using arcade_t = intcode::computer<long, arcade_policy>;

int main(int argc, const char * argv[])
{
    using namespace ranges;

    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <intcode-program>\n";
        return 1;
    }
    std::ifstream is{argv[1]};

    screen_t screen;

    channel_t out_channel;
    arcade_t arcade(intcode::istream_construct, is, screen, out_channel);
    auto memory_dump = arcade.memory();

    auto draw_screen = [&screen] {
        std::cout << "\033c";
        for (auto const & row : screen) {
            for (char c : row | views::transform(render)) {
                std::cout << c;
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    };

    auto control_arcade = [&] {
        std::thread arcade_thread{[&] {
            arcade.run();

            // write termination code
            out_channel.write(-2);
        }};

        long score = 0;
        while (true) {
            long x = out_channel.read();
            if (x == -2) break;
            long y = out_channel.read();

            if (x == -1 && y == 0) {
                // score update
                score = out_channel.read();
            } else {
                Tile t = static_cast<Tile>(out_channel.read());
                if (screen.size() <= static_cast<size_t>(y)) {
                    screen.resize(y + 1);
                }
                if (screen[y].size() <= static_cast<size_t>(x)) {
                    screen[y].resize(x + 1);
                }
                screen[y][x] = t;
            }
            if (score) {
                draw_screen();
                std::cout << "\nScore: " << score << '\n';
            }
        }
        if (arcade_thread.joinable()) arcade_thread.join();
    };

    control_arcade();
    draw_screen();

    std::cout << "Part 1: " << count_if(screen | views::join, [](Tile t) {
        return t == Tile::block;
    }) << " blocks are on screen\n";

    std::cout << "\nPress any button to start playing!";
    std::cin.get();

    // trick coin slot
    memory_dump[0] = 2;
    arcade.memory() = memory_dump;
    control_arcade();
}
