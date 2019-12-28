#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <thread>
#include <tuple>
#include <utility>

#include <range/v3/action/shuffle.hpp>
#include <range/v3/algorithm/minmax.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

#include <intcode/callback_policy.hpp>
#include <intcode/computer.hpp>

enum class Tile : long
{
    Unknown = 0,
    Wall = 1,
    Empty = 2,
    OxygenSystem = 3
};

int main()
{
    using namespace ranges;
    using pos_t = std::array<int, 2>;

    pos_t droid_pos = {0, 0};
    pos_t prospective_pos;
    std::map<pos_t, Tile> map = {{droid_pos, Tile::Empty}};
    auto surroundings =
        [&map](pos_t loc) -> std::array<std::tuple<int, pos_t, Tile>, 4> {
        auto lookup = [&map](int i, pos_t n) -> std::tuple<int, pos_t, Tile> {
            return {i, n, map[n]};
        };
        return {
            lookup(1, {loc[0], loc[1] - 1}), lookup(2, {loc[0], loc[1] + 1}),
            lookup(3, {loc[0] - 1, loc[1]}), lookup(4, {loc[0] + 1, loc[1]})};
    };

    auto print_map = [&] {
        std::cout << "--------------------------------\n";
        auto [min_x, max_x] = minmax(
            map | views::keys | views::transform([](pos_t p) { return p[0]; }));
        auto [min_y, max_y] = minmax(
            map | views::keys | views::transform([](pos_t p) { return p[1]; }));
        for (int y : views::iota(min_y, max_y + 1)) {
            for (int x : views::iota(min_x, max_x + 1)) {
                if (droid_pos == pos_t{x, y}) {
                    std::cout << 'X';
                } else {
                    switch (map[{x, y}]) {
                        case Tile::Unknown:
                            std::cout << ' ';
                            break;
                        case Tile::Empty:
                            std::cout << '.';
                            break;
                        case Tile::Wall:
                            std::cout << '#';
                            break;
                        case Tile::OxygenSystem:
                            std::cout << 'O';
                            break;
                    }
                }
            }
            std::cout << '\n';
        }
    };

    bool terminate = false;
    std::mt19937 urng{42};

    auto droid = [&] {
        auto write_cb = [&](long status) {
            Tile status_tile{status + 1};
            map[prospective_pos] = status_tile;
            switch (status_tile) {
                case Tile::OxygenSystem:
                    terminate = true;
                    std::cout << "Oxygen system found!\n";
                    print_map();
                    [[fallthrough]];
                case Tile::Empty:
                    droid_pos = prospective_pos;
                    break;
                default:
                    break;
            }
        };
        auto read_cb = [&]() -> long {
            auto surr = surroundings(droid_pos) | actions::shuffle(urng);
            for (auto const & [i, neighbor_pos, neighbor_status] : surr)
                if (neighbor_status == Tile::Unknown) {
                    prospective_pos = neighbor_pos;
                    return i;
                }
            for (auto const & [i, neighbor_pos, neighbor_status] : surr)
                if (neighbor_status == Tile::Empty) {
                    prospective_pos = neighbor_pos;
                    return i;
                }
            return 0;
        };
        using droid_t = intcode::computer<
            long, intcode::io_policy::callback_policy<long, decltype(write_cb),
                                                      decltype(read_cb)>>;
        return droid_t(intcode::istream_construct, std::cin,
                       std::move(write_cb), std::move(read_cb));
    }();

    droid.run([&terminate] { return !terminate; });

    std::cout << "Oxygen System found at " << droid_pos[0] << ", "
              << droid_pos[1] << '\n';
}
