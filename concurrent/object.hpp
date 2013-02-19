#ifndef __CONCURRENT_OBJECT_HPP__ 
#define __CONCURRENT_OBJECT_HPP__

#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <iostream>

#include "queue.hpp"

namespace concurrent
{
    /**
     *
     *
     *
     */
    template<typename T>
    class object
    {
        public:
            /*
                Ok, I must admit that I don't understand what happened here...
                Without a line of code that is forced to be executed (since it modifies external streams), in this case "std::cerr << std::endl;",
                the thread gets ID -1 assigned, which crashes the lock and thus the whole application itself.
                Otherwise, it gets a valid ID and runs perfectly... but why ? 

            */
            object(T t = T{}) : __myT(t), __done(false), __workerThread([=]() -> void { std::cerr << std::endl; while (!__done) { this->__innerqueue.pop()(); }}) // Little complicated, but that's the horror we face due to s.th. like a pop() ...
            {

            }

            ~object() 
            {
                this->__innerqueue << ([=]() { __done = true; });
                this->__workerThread.join();
            }

            template<typename F>
            auto operator()(F f) const -> std::future<decltype(f(__myT))>
            {
                auto promisedRes = std::make_shared< std::promise<decltype(f(__myT))> >();
                auto ret = promisedRes->get_future();

                this->__innerqueue << ([=]() -> void { 
                    try
                    {
                        this->__set_value(*promisedRes, f, this->__myT);
                    }
                    catch (...)
                    {
                        promisedRes->set_exception(std::make_exception_ptr("Uh oh..."));
                    }
                });
                return ret;
            }


        private:
            bool __done;
            mutable T __myT;
            std::thread __workerThread;
            mutable concurrent::queue<std::function<void()>, std::queue> __innerqueue;

            template<typename Fut, typename F, typename T>
            void __set_value(std::promise<Fut>& p, F& f, T& t) const 
            {
                p.set_value(f(t));
            }
            template<typename F, typename T>
            void __set_value(std::promise<void>& p, F& f, T& t) const 
            {
                f(t);
                p.set_value();
            }
    };
}

#endif  // !__CONCURRENT_OBJECT_HPP__