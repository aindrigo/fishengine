#include "fish/helpers.hpp"
#include "fish/runtime.hpp"
#include <cpptrace/from_current.hpp>
#include <format>
#include <set>
#include <string>

int main(int argc, char** argv)
{
    std::set<std::string> arguments;

    for (int i = 0; i < argc; i++)
        arguments.insert(argv[i]);

    CPPTRACE_TRY {
        fish::Runtime runtime(arguments);
        runtime.run();
    } CPPTRACE_CATCH(std::exception& e) {
#ifndef NDEBUG
        cpptrace::from_current_exception().print();
#endif
        fish::helpers::fatalError(std::format("Error caught: {}", e.what()));
    }
}