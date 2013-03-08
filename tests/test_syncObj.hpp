#ifndef __TEST_SYNC_OBJ_HPP__
#define __TEST_SYNC_OBJ_HPP__

#include "concurrent/object.hpp"

#include <iostream>
#include <string>

namespace conc_test
{
    namespace sync_object
    {
        void test_move()
        {
            concurrent::sync_object<std::string> blub ("Hello World!");
            concurrent::sync_object<std::string> blub2 ("Hello Ape!");
            blub2 = std::move(blub);
            blub2 <= ( [](std::string& s) -> void {
                std::cout <<  s << std::endl;
            });
        }

        void test_copy()
        {
            concurrent::sync_object<std::string> blub ("Hello World!");
            concurrent::sync_object<std::string> blub2 ("Hello Ape!");
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
            concurrent::sync_object<std::string> blub("World Hello");
            // Testing with lambda expression.
            blub <= ( [](std::string& s) -> void {
                std::cout << s << std::endl;
            });
            // Testing with free function.
            blub <= &func_functor_side; // [Note] std::addressof cannot be used here.
            // testing with functor struct.
            blub <= foo_functor_side();

            blub <= ( [](std::string& s) -> void {
                std::cout << "<Test result> " << s << std::endl;
            }); 
        }

        struct foo_functor
        {
            void operator()(std::string& s) const 
            {
                std::cout << s << std::endl;
            }
        };

        void func_functor(std::string& s)
        {
            std::cout << s << std::endl;
        }

        void test_simple()
        {
            concurrent::sync_object<std::string> blub("World Hello");
            // Testing with lambda expression.
            blub <= ( [](std::string& s) -> void {
                std::cout << s << std::endl;
            });
            // Testing with free function.
            blub <= &func_functor; // [Note] std::addressof cannot be used here.
            // testing with functor struct.
            blub <= foo_functor();
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

#endif // __TEST_SYNC_OBJ_HPP__
