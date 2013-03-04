#ifndef __TEST_ASYNC_OBJ_HPP__
#define __TEST_ASYNC_OBJ_HPP__

#include "concurrent/object.hpp"

#include <iostream>
#include <string>

namespace conc_test
{
    namespace async_object
    {
        void test_simple()
        {
            concurrent::async_object<std::string> blub ("Hello World!");
            auto res = blub <= ( [](std::string& s) -> std::string{
                return s + " omfg!";
            }); 
            std::cout << res.get() << std::endl;
        }

        void main()
        {
            test_simple();
        }
    }
}

#endif // __TEST_ASYNC_OBJ_HPP__
