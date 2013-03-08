#include "tests/test_channel.hpp"
#include "tests/test_scopeguard.hpp"
#include "tests/test_syncObj.hpp"
#include "tests/test_asyncObj.hpp"

#include <iostream>

int main (int argc, char** argv)
{
    std::cout << "[:: Performing channel test ::]" << std::endl;
    conc_test::channels::main();

    std::cout << "[:: Performing scope guard test ::]" << std::endl;
    conc_test::scope_guards::main();
    
    std::cout << "[:: Performing synchronous object test ::]" << std::endl;
    conc_test::sync_object::main();
    
    std::cout << "[:: Performing asynchronous object test ::]" << std::endl;
    conc_test::async_object::main();

    std::cout << "[:: Complete ::]" << std::endl;

    return 0;
}