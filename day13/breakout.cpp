#include <chrono>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/view/join.hpp>
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

int main()
{
    using namespace ranges;
    using cv_policy = intcode::io_policy::condition_variable_policy<long>;
    using channel_t = cv_policy::channel_type;
    using arcade_t = intcode::computer<long, cv_policy>;

    channel_t in_channel, out_channel;
    arcade_t arcade(intcode::istream_construct, std::cin, in_channel,
                    out_channel);

    std::thread arcade_thread{[&] {
        arcade.run();

        // write termination code
        out_channel.write(-1);
    }};

    std::vector<std::vector<Tile>> screen;
    while (true) {
        long x = out_channel.read();
        if (x < 0) break;
        long y = out_channel.read();
        Tile t = static_cast<Tile>(out_channel.read());
        if (screen.size() <= static_cast<size_t>(y)) {
            screen.resize(y + 1);
        }
        if (screen[y].size() <= static_cast<size_t>(x)) {
            screen[y].resize(x + 1);
        }
        screen[y][x] = t;
    }

    if (arcade_thread.joinable()) arcade_thread.join();

    // draw the screen
    for (auto const & row : screen) {
        for (char c : row | views::transform(render)) {
            std::cout << c;
        }
        std::cout << '\n';
    }
    std::cout << '\n';

    std::cout << "Part 1: " << count_if(screen | views::join, [](Tile t) {
        return t == Tile::block;
    }) << " blocks are on screen\n";
}
