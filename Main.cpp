#include "concurrent/object.hpp"
#include "concurrent/queue.hpp"

#include "tests/test_channel.hpp"
#include "tests/test_scopeguard.hpp"

#include <iostream>
#include <queue>
#include <string> 

int main (int argc, char** argv)
{
    {// Testing queue
        
        concurrent::queue<int, std::queue> myqueue;
        int a = 5;
        myqueue.push(a).push(17);
    }

    {// Testing sync object
        
        concurrent::async_object<std::string> blub ("Hello World!");
        auto res = blub <= ( [](std::string& s) -> std::string{
            return s + " omfg!";
        }); 
        std::cout << res.get() << std::endl;
    }

    {// Testing sync object
        
        concurrent::sync_object<std::string> blub("World Hello");
        blub <= ( [](std::string& s) -> void {
            std::cout << s << std::endl;
        });
    }

    conc_test::channels::main();
    conc_test::scope_guards::main();

    return 0;
}