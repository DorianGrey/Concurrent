<A small package for object concurrency using C++11>

Several parts of this package are based on presentations from C++ and Beyond 2012, whereas some of them are available
on Channel9 - MSDN.

The basic object concurrency (except the CoW-stuff) is based on Herb Sutter's presentation:
http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism

The error handling is based on the behavior of `std::future` and Andrei Alexandrescu's presentation:   
http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C

I've considered synchronous object concurrency to be a nice spot to use this kind of error handling, since it eases up 
exception-handling in that use-case. Thus, it is also included.

<===>
A further description will follow up, when the package is more complete... and has past most of the test cases ;)
