#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <utility>

#include <range/v3/experimental/utility/generator.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/indirect.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take_last.hpp>
#include <range/v3/view/take_while.hpp>
#include <range/v3/view/tokenize.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/zip.hpp>

#include <view/iterators.hpp>
#include <view/pair.hpp>

namespace coro_v3 = ranges::experimental;

struct object;
using graph_type = std::map<std::string, object>;
using graph_iterator = typename graph_type::iterator;

struct object
{
    graph_iterator orbiting;
    std::optional<int> depth = std::nullopt;
};

int get_depth(graph_iterator it)
{
    if (it->second.depth)
        return *it->second.depth;
    else
        return *(it->second.depth = get_depth(it->second.orbiting) + 1);
}

auto orbital_path(graph_iterator it) -> coro_v3::generator<graph_iterator>
{
    for (; it != graph_iterator{}; it = it->second.orbiting) co_yield it;
}

int main()
{
    using namespace ranges;

    // parse the individual relationships
    auto relations =
        getlines(std::cin) | views::transform([](std::string const & line) {
            return to_pair(line
                           | views::tokenize(
                               std::regex{"([A-Z0-9]+)\\)([A-Z0-9]+)"}, {1, 2})
                           | views::transform(&std::ssub_match::str));
        });

    // construct the graph
    graph_type graph;
    graph.insert(std::pair{"COM", object{{}, 0}});
    for (auto const & [orbitee, orbiter] : relations) {
        auto [it_orbitee, b] = graph.insert(std::pair{orbitee, object{}});
        graph[orbiter] = object{it_orbitee};
    }

    // calculate depths
    for (auto it : iterators(graph)) get_depth(it);
    std::cout << "Part 1: "
              << accumulate(graph | views::values
                                | views::transform([](auto const & obj) {
                                      return *obj.depth;
                                  }),
                            0)
              << '\n';

    // find minimum number of transfer orbits
    auto [me, my_depth] = graph["YOU"];
    auto [santa, santas_depth] = graph["SAN"];
    auto my_path = orbital_path(me) | to<std::vector<graph_iterator>>();
    auto santas_path = orbital_path(santa) | to<std::vector<graph_iterator>>();
    auto common_path =
        views::zip(my_path | views::reverse, santas_path | views::reverse)
        | views::take_while([](auto pair) { return pair.first == pair.second; })
        | views::transform([](auto pair) { return std::get<0>(pair); })
        | to<std::vector<graph_iterator>>();

    std::cout << "Part 2: "
              << *my_depth + *santas_depth
                     - 2 * (*common_path.back()->second.depth + 1)
              << '\n';
}