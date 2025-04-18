#include "fish/helpers.hpp"
#include "fish/runtime.hpp"
#include <cpptrace/from_current.hpp>
#include <format>

int main()
{
    CPPTRACE_TRY {
        fish::Runtime runtime;
        runtime.run();
    } CPPTRACE_CATCH(std::exception& e) {
        cpptrace::from_current_exception().print();
        fish::helpers::fatalError(std::format("Error caught: {}", e.what()));
    }
}