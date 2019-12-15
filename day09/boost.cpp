#include <fstream>
#include <stdexcept>

#include <boost/multiprecision/cpp_int.hpp>

#include <intcode/computer.hpp>

int main(int argc, const char * argv[])
{
    using boost::multiprecision::cpp_int;
    auto comp = [&] {
        if (argc != 2) throw std::runtime_error("invalid CL argument");
        std::ifstream is{argv[1]};
        return intcode::computer<cpp_int,
                                 intcode::io_policy::stream_policy<cpp_int>>(
            intcode::istream_construct, is);
    }();

    comp.run();
}
