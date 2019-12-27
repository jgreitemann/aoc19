#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <range/v3/view/chunk.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/getlines.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/tokenize.hpp>
#include <range/v3/view/transform.hpp>

int main()
{
    using namespace ranges;
    using term = std::pair<std::string_view, long>;

    std::map<std::string_view, long> balance;
    std::map<std::string_view, std::pair<long, std::vector<term>>> reactions;

    auto tokens =
        getlines(std::cin) | views::transform([](std::string const & line) {
            return line
                   | views::tokenize(std::regex{"([0-9]+) ([A-Z]+)"}, {1, 2})
                   | views::transform(&std::ssub_match::str)
                   | to<std::vector<std::string>>();
        })
        | to<std::vector<std::vector<std::string>>>();

    for (auto && line_tokens : tokens) {
        std::vector<term> terms;
        for (auto && two_tokens : line_tokens | views::chunk(2)) {
            auto it = begin(two_tokens);
            long factor = std::stoi(*(it++));
            auto [balance_it, b] = balance.emplace(*it, 0l);
            terms.push_back({balance_it->first, factor});
        }
        auto [product_sv, product_factor] = terms.back();
        terms.pop_back();
        reactions[product_sv] = {product_factor, std::move(terms)};
    }

    auto produce = [&](long fuel_amount) {
        long old_fuel = std::exchange(balance["FUEL"], -fuel_amount);
        bool reacted = true;
        while (reacted) {
            reacted = false;
            for (auto [desired_name, desired_amount] :
                 balance | views::filter([](auto const & b) {
                     return b.first != "ORE";
                 }) | views::filter([](auto const & b) {
                     return b.second < 0;
                 })) {
                auto const & reaction = reactions[desired_name];
                long times = (-desired_amount) / reaction.first
                             + ((-desired_amount) % reaction.first != 0);
                for (auto && [name, factor] : reaction.second)
                    balance[name] -= factor * times;
                balance[desired_name] += times * reaction.first;
                reacted = true;
                break;
            }
        }
        balance["FUEL"] += old_fuel + fuel_amount;
    };
    produce(1);

    long ore_per_fuel = -balance["ORE"];

    std::cout << "Part 1: " << ore_per_fuel
              << " ORE needed to produce 1 FUEL.\n";

    for (auto & [name, saldo] : balance) {
        saldo = 0;
    }
    balance["ORE"] = 1'000'000'000'000l;

    while (balance["ORE"] > ore_per_fuel)
        produce(balance["ORE"] / ore_per_fuel);

    long fuel_produced;
    long ore_left;
    while (balance["ORE"] >= 0) {
        fuel_produced = balance["FUEL"];
        ore_left = balance["ORE"];
        produce(1);
    }

    std::cout << "Part 2: Produced " << fuel_produced << " FUEL with "
              << ore_left << " ORE to spare.\n";
}