Concurrent
==========
A small package for (object) concurrency using C++11.

Content
-------
Several parts of this package are based on presentations from C++ and Beyond 2012, whereas some of them are available
on Channel9 - MSDN.

The basic object concurrency (except the CoW-stuff) is based on Herb Sutter's presentation:
http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism

The error handling is based on the behavior of `std::future` and Andrei Alexandrescu's presentation:   
http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C

I've considered synchronous object concurrency to be a nice spot to use this kind of error handling, since it eases up 
exception-handling in that use-case. Thus, it is also included.

Requirements
------------
Currently (2013/03/04), I've only tested it with VS2012, using the CTP from november 2012 (especially for variadic 
templates). But it should work with any compiler that supports the following features from C++11:
* [type traits](http://en.cppreference.com/w/cpp/header/type_traits)
* [atomics](http://en.cppreference.com/w/cpp/atomic)
* uniform initialization (the {} thing)
* [thread/future/mutex](http://en.cppreference.com/w/cpp/thread)
* lambda functions
* [`std::function`](http://en.cppreference.com/w/cpp/utility/functional)
* [memory](http://en.cppreference.com/w/cpp/memory) for std::shared_ptr
* variadic templates
* `decltype` (including tests that rely on members of an object)

<===>
A further description will follow up, when the package is more complete... and has past most of the test cases ;)
