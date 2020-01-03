#pragma once
#include <array>
#include <deque>
#include <map>
#include <tuple>
#include <utility>

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/view/cache1.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/transform.hpp>

template <size_t I, typename Point>
Point get_neighbor(Point p, std::tuple_element_t<I, Point> d)
{
    std::get<I>(p) += d;
    return p;
}

template <typename Point, std::size_t... I>
auto neighbors_impl(Point const & p, std::index_sequence<I...>)
    -> std::array<Point, sizeof...(I)>
{
    return {get_neighbor<I / 2>(p, I % 2 ? +1 : -1)...};
}

template <typename Point>
auto neighbors(Point const & p)
{
    using Indices = std::make_index_sequence<2 * std::tuple_size_v<Point>>;
    return neighbors_impl(p, Indices{});
}

template <typename Point, typename Predicate,
          typename Map = std::map<Point, int>>
std::deque<Point> shortest_path(Point start, Point target, Predicate && pred)
{
    using namespace ranges;

    Map distances;
    distances[start] = 1;

    /* wave expansion */ {
        Map last_wave, this_wave;
        last_wave[start] = 1;

        enum class WaveStatus
        {
            Rolling,
            Ebbed,
            TargetReached
        };

        WaveStatus wave_status = WaveStatus::Rolling;
        for (int n_wave = 2; wave_status == WaveStatus::Rolling; ++n_wave) {
            wave_status = WaveStatus::Ebbed;
            for (Point n : last_wave | views::keys
                               | views::transform(
                                   [](Point const & p) { return neighbors(p); })
                               | views::cache1 | views::join) {
                if (!pred(n)) continue;
                auto [it, inserted] = distances.emplace(n, n_wave);
                if (inserted) {
                    this_wave.emplace(n, n_wave);
                    if (n == target) {
                        wave_status = WaveStatus::TargetReached;
                        break;
                    } else {
                        wave_status = WaveStatus::Rolling;
                    }
                }
            }
            last_wave = std::move(this_wave);
            this_wave.clear();
        }
        if (wave_status == WaveStatus::Ebbed) return {};
    }

    /* backtracking */
    std::deque<Point> path{target};
    for (Point current_point = path.front(); path.front() != start;
         path.push_front(current_point)) {
        auto && neighborhood = neighbors(current_point);
        current_point = *find_if(neighborhood,
                                 [&distances, d = distances[current_point] - 1](
                                     Point p) { return distances[p] == d; });
    }
    return path;
}