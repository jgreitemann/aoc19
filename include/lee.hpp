#pragma once
#include <array>
#include <deque>
#include <map>
#include <set>
#include <tuple>
#include <utility>

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/experimental/utility/generator.hpp>
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

template <typename Point, typename Predicate>
auto waves(Point center, Predicate && pred)
    -> ranges::experimental::generator<std::set<Point>>
{
    using namespace ranges;
    using Set = std::set<Point>;

    enum class WaveStatus
    {
        Rolling,
        Ebbed
    };

    Set last_wave;
    Set this_wave{center};
    Set next_wave;
    co_yield this_wave;

    WaveStatus wave_status = WaveStatus::Rolling;
    while (wave_status == WaveStatus::Rolling) {
        wave_status = WaveStatus::Ebbed;
        for (Point n : this_wave | views::transform([](Point const & p) {
                           return neighbors(p);
                       }) | views::cache1
                           | views::join) {
            if (!pred(n)) continue;
            if (find(last_wave, n) == end(last_wave)) {
                next_wave.insert(n);
                wave_status = WaveStatus::Rolling;
            }
        }
        last_wave = std::move(this_wave);
        this_wave = std::move(next_wave);
        next_wave.clear();
        co_yield this_wave;
    }
}

template <typename Point, typename Predicate>
std::deque<Point> shortest_path(Point start, Point target, Predicate && pred)
{
    using namespace ranges;
    using Map = std::map<Point, int>;

    Map distances;

    auto ebbed = [&] {
        for (auto && [n_wave, wave] : waves(start, pred) | views::enumerate) {
            for (auto p : wave) {
                distances.emplace(p, n_wave + 1);
                if (p == target) return false;
            }
        }
        return true;
    }();

    if (ebbed) return {};

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