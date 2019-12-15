#include <array>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <range/v3/algorithm/max_element.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/zip.hpp>

#include <intcode/computer.hpp>
#include <intcode/condition_variable_policy.hpp>
#include <view/combinatorics.hpp>
#include <view/graph.hpp>
#include <view/pair.hpp>
#include <view/ref.hpp>

template <class T>
struct type_identity
{
    using type = T;
};

int main(int argc, const char * argv[])
{
    using namespace ranges;

    auto controller_factory = [&](auto policy_type, auto &&... args) {
        if (argc != 2) throw std::runtime_error("invalid CL argument");
        std::ifstream ifs{argv[1]};
        return intcode::computer<typename decltype(policy_type)::type>(
            intcode::istream_construct, ifs,
            std::forward<decltype(args)>(args)...);
    };

    constexpr int N_amp = 5;
    using result_pair_t = std::pair<std::vector<int>, int>;
    auto res = permutations(views::iota(0, N_amp)) | graph([&](auto perm) {
                   type_identity<intcode::io_policy::stream_policy> pol{};
                   std::array<std::stringstream, N_amp + 1> sss;
                   for (auto && [ss, phase_set] : views::zip(sss, perm))
                       ss << phase_set << ' ';
                   sss.front() << 0;
                   for (auto && [is, os] : sss | lvalue_ref | adjacent_pairs)
                       controller_factory(pol, is, os).run();
                   int thruster_output;
                   sss.back() >> thruster_output;
                   return thruster_output;
               })
               | to<std::vector<result_pair_t>>();

    std::cout << "Part 1: "
              << max_element(res, std::less<>{}, &result_pair_t::second)->second
              << '\n';

    res =
        permutations(views::iota(N_amp, 2 * N_amp)) | graph([&](auto perm) {
            using cv_policy = intcode::io_policy::condition_variable_policy;
            type_identity<cv_policy> pol{};
            using channel_t = cv_policy::channel_type;
            std::array<channel_t, N_amp + 1> chs{};
            for (auto && [ch, phase_set] : views::zip(chs, perm))
                ch.write(phase_set);

            auto futures =
                chs | lvalue_ref | adjacent_pairs
                | views::transform([&](auto && ch_pair) -> std::future<void> {
                      return std::async(
                          std::launch::async,
                          [&](channel_t & in_ch, channel_t & out_ch) {
                              try {
                                  controller_factory(pol, in_ch, out_ch).run();
                              } catch (std::exception & e) {
                                  std::cerr << "Computer crashed with error: "
                                            << e.what() << '\n';
                                  std::terminate();
                              }
                          },
                          std::ref(ch_pair.first), std::ref(ch_pair.second));
                  })
                | to<std::vector<std::future<void>>>();

            std::atomic<bool> alive = true;
            std::future<int> feedback_future =
                std::async(std::launch::async, [&] {
                    int last_output = 0;
                    while (true) {
                        std::future<int> f =
                            std::async(std::launch::async,
                                       [&] { return chs.back().read(); });
                        bool ready = false;
                        while (alive) {
                            if (auto status =
                                    f.wait_for(std::chrono::milliseconds(1));
                                status == std::future_status::ready) {
                                ready = true;
                                break;
                            }
                        }
                        if (ready) {
                            last_output = f.get();
                            chs.front().write(last_output);
                        } else {
                            // write anything to unblock the read
                            chs.back().write(0);
                            return last_output;
                        }
                    }
                });

            chs.front().write(0);

            for (auto & f : futures) f.wait();
            alive = false;
            return feedback_future.get();
        })
        | to<std::vector<result_pair_t>>();

    std::cout << "Part 2: "
              << max_element(res, std::less<>{}, &result_pair_t::second)->second
              << '\n';
}
