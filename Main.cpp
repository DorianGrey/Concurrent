#include "tests/test_channel.hpp"
#include "tests/test_scopeguard.hpp"
#include "tests/test_syncObj.hpp"
#include "tests/test_asyncObj.hpp"

#include <iostream>

int main (int argc, char** argv)
{
    std::cout << "[:: Performing channel test ::]" << std::endl;
    conc_test::channels::main();
    std::cout << std::endl;

    std::cout << "[:: Performing scope guard test ::]" << std::endl;
    conc_test::scope_guards::main();
    std::cout << std::endl;
    
    std::cout << "[:: Performing synchronous object test ::]" << std::endl;
    conc_test::sync_object::main();
    std::cout << std::endl;
    
    std::cout << "[:: Performing asynchronous object test ::]" << std::endl;
    conc_test::async_object::main();
    std::cout << std::endl;

#ifndef _MSC_VER
    auto re1 = expected::result_of( [](const std::string& val1)-> int {std::cout << val1 << std::endl; return 0;}, std::string("Hello"));
    auto res = expected::result_of( [](const std::string& val1, const std::string& val2)-> void {std::cout << val1 << ""<< val2 << std::endl;}, std::string("Hello"), std::string("World"));
#endif     

    std::cout << "[:: Complete ::]" << std::endl;

    return 0;
}