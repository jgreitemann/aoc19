#include <chrono>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <thread>
#include <tuple>
#include <utility>

#include <range/v3/action/shuffle.hpp>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/minmax.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/transform.hpp>

#include <intcode/callback_policy.hpp>
#include <intcode/computer.hpp>

#include <lee.hpp>

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

    auto print_map = [&](auto path = std::deque<pos_t>{}) {
        std::cout << "--------------------------------\n";
        auto [min_x, max_x] = minmax(
            map | views::keys | views::transform([](pos_t p) { return p[0]; }));
        auto [min_y, max_y] = minmax(
            map | views::keys | views::transform([](pos_t p) { return p[1]; }));
        for (int y : views::iota(min_y, max_y + 1)) {
            for (int x : views::iota(min_x, max_x + 1)) {
                if (pos_t{x, y} == pos_t{0, 0}) {
                    std::cout << 'S';
                } else if (droid_pos == pos_t{x, y}) {
                    std::cout << 'X';
                } else if (find(path, pos_t{x, y}) != end(path)) {
                    std::cout << '*';
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

    auto droid_init_mem = droid.memory();
    droid.run([&terminate] { return !terminate; });

    pos_t oxygen_pos = droid_pos;

    std::cout << "Oxygen System found at " << oxygen_pos[0] << ", "
              << oxygen_pos[1] << '\n';

    // find the shortest path
    while (true) {
        // shortest path given current information
        auto path = shortest_path({0, 0}, oxygen_pos, [&map](pos_t p) {
            return map[p] != Tile::Wall;
        });

        // check the path for unknown walls
        terminate = false;
        droid_pos = {0, 0};
        auto path_it = path.begin();
        auto check_droid = [&] {
            auto write_cb = [&](long status) {
                Tile status_tile = static_cast<Tile>(status + 1);
                if (status_tile == Tile::Wall) {
                    std::cout
                        << "Hit a previously unknown wall at: " << (*path_it)[0]
                        << ", " << (*path_it)[1] << '\n';
                    map[*path_it] = Tile::Wall;
                } else {
                    droid_pos = *path_it;
                }
                terminate = (status_tile != Tile::Empty);
            };
            auto read_cb = [&]() -> long {
                auto next = *(++path_it);
                for (auto && [dir, p, t] : surroundings(droid_pos))
                    if (p == next) return dir;
                return 0;
            };
            using droid_t = intcode::computer<
                long, intcode::io_policy::callback_policy<
                          long, decltype(write_cb), decltype(read_cb)>>;
            return droid_t(typename droid_t::memory_type{droid_init_mem},
                           std::move(write_cb), std::move(read_cb));
        }();
        check_droid.run([&] { return !terminate; });

        if (droid_pos == oxygen_pos) {
            print_map(path);
            std::cout << "Part 1: Shortest path requires " << path.size() - 1
                      << " movement commands.\n";
            break;
        }
    }
}
