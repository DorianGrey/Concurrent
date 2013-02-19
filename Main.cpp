#include "concurrent/object.hpp"
#include "concurrent/queue.hpp"

#include <iostream>
#include <queue>
#include <string>

int main (int argc, char** argv)
{
    concurrent::queue<int, std::queue> myqueue;
    int a = 5;
    myqueue << a << 17;

    concurrent::object<std::string> blub ("Hello World!");

    auto res = blub( [](std::string& s) -> std::string{
        return s + " omfg!";
    });

    std::cout << res.get() << std::endl;
    return 0;
}