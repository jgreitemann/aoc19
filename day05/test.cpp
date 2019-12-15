#include <fstream>
#include <stdexcept>

#include <intcode/computer.hpp>

int main(int argc, const char * argv[])
{
    auto comp = [&] {
        if (argc != 2) throw std::runtime_error("invalid CL argument");
        std::ifstream is{argv[1]};
        return intcode::computer(intcode::istream_construct, is);
    }();

    comp.run();
}
