#include <functional>
#include <iostream>
#include <map>
#include <thread>
#include <utility>

#include <intcode/computer.hpp>
#include <intcode/condition_variable_policy.hpp>

int main()
{
    using namespace ranges;
    using cv_policy = intcode::io_policy::condition_variable_policy<int>;
    using channel_t = cv_policy::channel_type;
    using robot_t = intcode::computer<int, cv_policy>;
    using pos_t = std::pair<int, int>;

    struct panel_t
    {
        int count_painted = 0;
        int color = 0;
    };

    std::map<pos_t, panel_t> hull;
    pos_t pos = {0, 0};
    int orientation = 0;

    channel_t in_channel, out_channel;
    robot_t robot(intcode::istream_construct, std::cin, in_channel,
                  out_channel);

    std::thread robo_thread{[&] {
        robot.run();

        // satisfy final write request
        in_channel.read();

        // write termination code
        out_channel.write(42);
    }};

    while (true) {
        panel_t & current_panel = hull[pos];
        in_channel.write(current_panel.color);

        int paint_cmd = out_channel.read();
        if (paint_cmd == 42) {
            break;
        } else {
            current_panel.color = paint_cmd;
            current_panel.count_painted++;
        }

        int turn_cmd = out_channel.read();
        if (turn_cmd) {
            orientation = (orientation + 1) % 4;
        } else {
            orientation = (orientation + 3) % 4;
        }

        switch (orientation) {
            case 0:
                pos.second++;
                break;
            case 1:
                pos.first++;
                break;
            case 2:
                pos.second--;
                break;
            case 3:
                pos.first--;
                break;
            default:
                throw std::logic_error("invalid orientation");
        }
    }

    if (robo_thread.joinable()) robo_thread.join();

    std::cout << "Part 1: " << hull.size() << " panels painted\n";
}
