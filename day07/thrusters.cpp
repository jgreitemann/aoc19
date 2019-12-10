#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/zip.hpp>

#include <intcode_computer.hpp>
#include <view/combinatorics.hpp>
#include <view/graph.hpp>
#include <view/pair.hpp>
#include <view/ref.hpp>

int main(int argc, const char * argv[])
{
    using namespace ranges;

    auto controller_factory = [&](std::istream & is, std::ostream & os) {
        if (argc != 2) throw std::runtime_error("invalid CL argument");
        std::ifstream ifs{argv[1]};
        return intcode::computer(intcode::istream_construct, ifs, is, os);
    };

    constexpr int N_amp = 5;
    using result_pair_t = std::pair<std::vector<int>, int>;
    auto res = permutations(views::iota(0, N_amp)) | graph([&](auto perm) {
                   std::array<std::stringstream, N_amp + 1> sss;
                   for (auto && [ss, phase_set] : views::zip(sss, perm))
                       ss << phase_set << ' ';
                   sss.front() << 0;
                   for (auto && [is, os] : sss | lvalue_ref | adjacent_pairs)
                       controller_factory(is, os).run();
                   int thruster_output;
                   sss.back() >> thruster_output;
                   return thruster_output;
               })
               | to<std::vector<result_pair_t>>();

    std::cout << "Part 1: "
              << max_element(res, std::less<>{}, &result_pair_t::second)->second
              << '\n';
}
