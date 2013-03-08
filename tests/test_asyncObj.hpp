#ifndef __TEST_ASYNC_OBJ_HPP__
#define __TEST_ASYNC_OBJ_HPP__

#include "concurrent/object.hpp"

#include <iostream>
#include <string>

namespace conc_test
{
    namespace async_object
    {
        void test_move()
        {
            concurrent::async_object<std::string> blub ("Hello World!");
            concurrent::async_object<std::string> blub2 ("Hello Ape!");
            blub2 = std::move(blub);
            blub2 <= ( [](std::string& s) -> void {
                std::cout <<  s << std::endl;
            });
        }

        void test_copy()
        {
            concurrent::async_object<std::string> blub ("Hello World!");
            concurrent::async_object<std::string> blub2 ("Hello Ape!");
            blub = blub2;
            blub <= ( [](std::string& s) -> void {
                std::cout <<  s << std::endl;
            });
        }

        struct foo_functor_side
        {
            void operator()(std::string& s) const 
            {
                s += " Moreover, it's crappy!";
            }
        };

        void func_functor_side(std::string& s)
        {
            s += " Its not awesome plz.";
        }

        void test_side_effects()
        {
            concurrent::async_object<std::string> blub ("Hello World!");
            // Testing with lambda expression.
            auto res = blub <= ( [](std::string& s) -> void {
                 s += " omfg!";
            });
            // Testing with free function.
            auto res2 = blub <= &func_functor_side; // [Note] std::addressof cannot be used here.
            // Testing with functor struct.
            auto res3 = blub <= foo_functor_side();

            blub <= ( [](std::string& s) -> void {
                std::cout << "<Test result> " << s << std::endl;
            }); 
        }

        struct foo_functor
        {
            std::string operator()(std::string& s) const 
            {
                return s + " Moreover, it's crappy!";
            }
        };

        std::string func_functor(std::string& s)
        {
            return s + " Its not awesome plz.";
        }

        void test_simple()
        {
            concurrent::async_object<std::string> blub ("Hello World!");
            // Testing with lambda expression.
            auto res = blub <= ( [](std::string& s) -> std::string{
                return s + " omfg!";
            });
            // Testing with free function.
            auto res2 = blub <= &func_functor; // [Note] std::addressof cannot be used here.
            // Testing with functor struct.
            auto res3 = blub <= foo_functor();

            std::cout << res.get() << std::endl;
            std::cout << res2.get() << std::endl;
            std::cout << res3.get() << std::endl;
        }

        void main()
        {
            std::cout << "[:: Test 1: Call with no side effects. ::]" << std::endl;
            test_simple();

            std::cout << "[:: Test 2: Call with side effects. ::]" << std::endl;
            test_side_effects();

            std::cout << "[:: Test 3: Copy stuff. ::]" << std::endl;
            test_copy();

            std::cout << "[:: Test 4: Move stuff. ::]" << std::endl;
            test_move();
        }
    }
}

#endif // __TEST_ASYNC_OBJ_HPP__
