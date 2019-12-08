#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <utility>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/indirect.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/tokenize.hpp>

#include <view/iterators.hpp>
#include <view/pair.hpp>

struct object;
using graph_type = std::map<std::string, object>;
using graph_iterator = typename graph_type::iterator;

struct object
{
    graph_iterator orbiting;
    std::optional<int> depth;
};

int get_depth(graph_iterator it)
{
    if (it->second.depth)
        return *it->second.depth;
    else
        return *(it->second.depth = get_depth(it->second.orbiting) + 1);
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
}