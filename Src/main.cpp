#include "Tests/Tests.hpp"
#include "Tests/PerformanceTests.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc == 1 || (argc == 2 && std::string(argv[1]) == "--test")) {
        return RunTests() == 0 ? 0 : 1;
    }
    if (argc == 2 && std::string(argv[1]) == "--performance") {return run_performance_tests();}

    std::cerr << "Использование: " << argv[0] << " [--test | --performance]\n";
    return 1;
}
