#include "concurrent/object.hpp"
#include "concurrent/queue.hpp"

#include <iostream>
#include <queue>
#include <string>
#include "util/detect.hpp"

int main (int argc, char** argv)
{
    {
        // Testing queue
        concurrent::queue<int, std::queue> myqueue;
        int a = 5;
        myqueue << a << 17;
    }

    {
        // Testing sync object
        concurrent::async_object<std::string> blub ("Hello World!");
        auto res = blub( [](std::string& s) -> std::string{
            return s + " omfg!";
        }); 
        std::cout << res.get() << std::endl;
    }

    {
        // Testing sync object
        concurrent::sync_object<std::string> blub("World Hello");
        blub( [](std::string& s) -> void {
            std::cout << s << std::endl;
        });
    }

    {
        std::string s;
        std::cout << detect::has_member_swap<std::string>::value << std::endl;
    }

    return 0;
}