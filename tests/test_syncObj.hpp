#ifndef __TEST_SYNC_OBJ_HPP__
#define __TEST_SYNC_OBJ_HPP__

#include "concurrent/object.hpp"

#include <iostream>
#include <string>

namespace conc_test
{
    namespace sync_object
    {
        void test_simple()
        {
            concurrent::sync_object<std::string> blub("World Hello");
            blub <= ( [](std::string& s) -> void {
                std::cout << s << std::endl;
            });
        }

        void main()
        {
           test_simple();
        }
    }
}

#endif // __TEST_SYNC_OBJ_HPP__
