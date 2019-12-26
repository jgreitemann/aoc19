#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>
#include <utility>

#include <range/v3/algorithm/copy.hpp>
#include <range/v3/algorithm/equal.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/tokenize.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <view/pair.hpp>

struct moon_t
{
    std::array<int, 3> pos;
    std::array<int, 3> vel;

    int total_energy() const
    {
        using namespace ranges;
        constexpr int (*iabs)(int) = &std::abs;
        return accumulate(pos, 0, std::plus<>{}, iabs)
               * accumulate(vel, 0, std::plus<>{}, iabs);
    }

    friend std::ostream & operator<<(std::ostream & os, moon_t const & m)
    {
        os << "pos=<x=" << m.pos[0] << ", y=" << m.pos[1] << ", z=" << m.pos[2]
           << ">, vel=<x=" << m.vel[0] << ", y=" << m.vel[1]
           << ", z=" << m.vel[2] << ">";
        return os;
    }

    friend bool operator==(moon_t const & lhs, moon_t const & rhs)
    {
        using namespace ranges;
        return equal(lhs.pos, rhs.pos) && equal(lhs.vel, rhs.vel);
    }
};

using config_t = std::vector<moon_t>;

int main(int argc, const char * argv[])
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <N_steps>\n";
        return 1;
    }
    std::size_t N_steps = std::atol(argv[1]);

    using namespace ranges;
    auto moons =
        getlines(std::cin) | views::transform([](std::string const & line) {
            return line
                   | views::tokenize(
                       std::regex{"<x=(-?[0-9]+), y=(-?[0-9]+), z=(-?[0-9]+)>"},
                       {1, 2, 3})
                   | views::transform(
                       [](auto const & sm) { return std::stoi(sm.str()); })
                   | to<std::vector<int>>();
        })
        | views::transform([](auto && vec) {
              return moon_t{{vec[0], vec[1], vec[2]}, {}};
          })
        | to<config_t>();

    auto evolve = [](auto & moons) {
        // apply gravity
        auto unit = [](int i) { return i ? i / std::abs(i) : 0; };
        for (auto && [a, b] : pairs(moons)) {
            for (auto && [pa, va, pb, vb] :
                 views::zip(a.pos, a.vel, b.pos, b.vel)) {
                int delta_v = unit(pb - pa);
                va += delta_v;
                vb -= delta_v;
            }
        }

        // update positions
        for (auto & a : moons)
            for (auto && [p, v] : views::zip(a.pos, a.vel)) p += v;
    };

    for (size_t step : views::ints(1ul, N_steps + 1)) {
        evolve(moons);
        if (step % (N_steps / 10) == 0) {
            std::cout << "\nAfter " << step << " steps:\n";
            for (auto const & m : moons) std::cout << m << '\n';
        }
    }

    std::cout << "\nPart 1: Total energy in the system: "
              << accumulate(moons, 0, std::plus<>{}, &moon_t::total_energy)
              << '\n';

    config_t target_config = moons;

    for (size_t step : views::iota(N_steps + 1)) {
        if (step % 1000000000 == 0) std::cout << step << " steps...\n";
        evolve(moons);
        if (moons == target_config) {
            std::cout << "\n Part 2: Target configuration recurred at step "
                      << step << ", i.e. after a period of " << step - N_steps
                      << " steps.\n";
            break;
        }
    }
}
