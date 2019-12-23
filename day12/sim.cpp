#include <cmath>
#include <functional>
#include <iostream>
#include <string>

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
};

int main()
{
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
        | to<std::vector<moon_t>>();

    constexpr int N_steps = 1000;
    for (int step : views::ints(0, N_steps)) {
        if (step % (N_steps / 10) == 0) {
            std::cout << "\nAfter " << step << " steps:\n";
            for (auto const & m : moons) std::cout << m << '\n';
        }

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
    }

    std::cout << "\nPart 1: Total energy in the system: "
              << accumulate(moons, 0, std::plus<>{}, &moon_t::total_energy)
              << '\n';
}
