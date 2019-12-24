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

using config_t = std::array<moon_t, 4>;

template <size_t... I>
bool config_cmp_impl(config_t const & lhs, config_t const & rhs,
                     std::index_sequence<I...>)
{
    return (... && (lhs[I] == rhs[I]));
}

bool operator==(config_t const & lhs, config_t const & rhs)
{
    return config_cmp_impl(lhs, rhs, std::make_index_sequence<4>{});
}

namespace std {
template <>
struct hash<moon_t>
{
    std::size_t operator()(moon_t const & m) const
    {
        std::hash<int> hasher{};
        std::size_t h = std::hash<int>{}(m.pos[0]);
        h = (h + (324723947 + hasher(m.pos[1]))) ^ 93485734985;
        h = (h + (324723947 + hasher(m.pos[2]))) ^ 93485734985;
        h = (h + (324723947 + hasher(m.vel[0]))) ^ 93485734985;
        h = (h + (324723947 + hasher(m.vel[1]))) ^ 93485734985;
        h = (h + (324723947 + hasher(m.vel[2]))) ^ 93485734985;
        return h;
    }
};
template <>
struct hash<config_t>
{
    std::size_t operator()(config_t const & c) const
    {
        std::hash<moon_t> hasher{};
        std::size_t h = 0;
        for (auto const & m : c)
            h = (h + (324723947 + hasher(m))) ^ 93485734985;
        return h;
    }
};
}  // namespace std

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

    if (moons.size() != 4)
        throw std::runtime_error("Input of four moons required!");
    config_t conf;
    copy(moons, conf.begin());

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

    constexpr int N_steps = 1000;
    for (int step : views::ints(0, N_steps)) {
        if (step % (N_steps / 10) == 0) {
            std::cout << "\nAfter " << step << " steps:\n";
            for (auto const & m : moons) std::cout << m << '\n';
        }
        evolve(moons);
    }

    std::cout << "\nPart 1: Total energy in the system: "
              << accumulate(moons, 0, std::plus<>{}, &moon_t::total_energy)
              << '\n';

    std::unordered_set<config_t> configs;
    for (unsigned long step : views::iota(0ul)) {
        if (step % 100000 == 0) std::cout << "step " << step << '\n';
        auto [it, not_recurrent] = configs.insert(conf);
        if (!not_recurrent) {
            std::cout << "Part 2: Found recurrent configuration after " << step
                      << " steps.\n";
            break;
        }
        evolve(conf);
    }
}
