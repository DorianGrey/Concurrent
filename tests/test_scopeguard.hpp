#ifndef __TEST_SCOPE_GUARD_HPP__
#define __TEST_SCOPE_GUARD_HPP__

#include "error_handling/scope_guard.hpp"

#include <iostream>

namespace conc_test
{
    namespace scope_guards
    {
        void test_do_undo()
        {
            auto scoper = scope_guard::make([](){ std::cout << "Scoper activated!" << std::endl; }, [](){ std::cout << "Scoper dead!" << std::endl; });
            auto res = ~scoper;         // Executes the scoper
            if (res.valid()) !scoper;   // Commits the action  
        }

        void test_simple()
        {
            auto scoper = scope_guard::make([]() -> void { std::cout << "Scoper down!" << std::endl; });
            !scoper;  // Disable the guard!
            !scoper;  // Enable the guard again!
        }

        void main()
        {
            test_simple();
            test_do_undo();
        }
    }
}

#endif // __TEST_SCOPE_GUARD_HPP__
