#ifndef __CONCURRENT_OBJECT_HPP__ 
#define __CONCURRENT_OBJECT_HPP__

#include <atomic>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

#include "queue.hpp"

/************************************************************************/
/* TODOS                                                                */
/* - Deal with copy + move c'tor                                        */
/* - more tests                                                         */
/* - documentation                                                      */
/************************************************************************/

namespace concurrent
{
    /**
     *
     *
     *
     */
    template<typename T>
    class async_object
    {
        public:
            async_object(T t = T{}) : __myT(t), __workerThread([=]() -> void { __done = false; while (!__done) { this->__innerqueue.pop()(); }}) // Little complicated, but that's the horror we face due to s.th. like a pop() ...
            {

            }

            ~async_object() 
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
            mutable T __myT;
            mutable concurrent::queue<std::function<void()>, std::queue> __innerqueue;
            std::atomic_bool __done;
            std::thread __workerThread;
            

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

    template<typename T>
    class sync_object
    {
        public:
            /*

            */
            sync_object(T t = T{}) : __myT(t)
            {

            }

            ~sync_object() 
            {

            }

            template<typename F>
            auto operator()(F f) const -> decltype(f(__myT))
            {
                std::lock_guard<std::mutex> guard(this->__lock);
                return f(__myT);                
            }  

        private:
            mutable T __myT;
            mutable std::mutex __lock;
    };
}

#endif  // !__CONCURRENT_OBJECT_HPP__