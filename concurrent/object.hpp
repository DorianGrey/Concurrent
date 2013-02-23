#ifndef __CONCURRENT_OBJECT_HPP__ 
#define __CONCURRENT_OBJECT_HPP__

#include <atomic>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

#include "error_handling/expected.hpp"
#include "queue.hpp"
#include "util/detect.hpp"

/************************************************************************/
/* TODOS                                                                */
/* - Deal with copy + move c'tor                                        */
/* - more tests                                                         */
/* - documentation                                                      */
/************************************************************************/

namespace concurrent
{
    /** \brief This is an asynchronous monitor class. It handles a parameter of the particular type, granting concurrent-safe access by sending functor-messages asynchronously.
     *         As a result, all return values are futures.
     *         It is based upon the concurrent<T> class presented by Herb Sutter.
     *         http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism
     *
     *  \param Data-type that should be covered by this class.
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
            mutable T __myT;                                                           /**< Value that should be modifiable through any executed functor */
            mutable concurrent::queue<std::function<void()>, std::queue> __innerqueue; /**< Internally synchronized queue */
            std::atomic_bool __done;                                                   /**< Indicator for the thread to run out */
            std::thread __workerThread;                                                /**< Worker thread */
            

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
    
    /** \brief This is a synchronous monitor class. It handles a parameter of the particular type, granting concurrent-safe access by sending functor-messages synchronously.
     *         It is based upon the monitor<T> class presented by Herb Sutter.
     *         http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism
     *
     *  \param Data-type that should be covered by this class.
     */
    template<typename T>
    class sync_object
    {
        public:
            /** \brief Default c'tor. If T has a default c'tor or can be constructed by uniform initialization, there is no need to give a particular instance.
             *
             * \param t T Value to handle inside this class.
             */
            sync_object(T t = T{}) : __myT(t)
            {

            }

            /** \brief Copy c'tor. It acquires the remote lock to ensure that no-one is currently modifying the internal data.
             *
             * \param rhs const sync_object& Synchronized object to copy from.
             * \note If this c'tor can be used depends on T having a copy-c'tor or not - you will get a compile-error if it does not!
             */
            sync_object(const sync_object& rhs)
            {
                static_assert( std::is_copy_constructible<T>::value, "T is not copy-constructable!" );
                if (this != std::addressof(rhs))
                {
                    std::lock_guard<std::mutex> guard(rhs.__lock); 
                    this->__myT = rhs.__myT; // TODO: Hm... maybe there is another option: Potentially copy-and-swap-idiom! Should depend on member swap available for T.
                }
            }

            /** \brief D'tor. It acquires the local lock to ensure that no-one is still using the internal data.
             *
             */
            ~sync_object() 
            {
                std::lock_guard<std::mutex> guard(this->__lock); 
            }

            /** \brief Assignment operator
             *
             * \param rhs const sync_object& Synchronized object to copy from.
             * \return *this
             */
            sync_object& operator=(const sync_object& rhs)
            {
                if (this != std::addressof(rhs))
                {
                    std::lock_guard<std::mutex> guard(rhs.__lock); 
                    this->__myT = rhs.__myT; // TODO: Hm... maybe there is another option: Potentially copy-and-swap-idiom! Should depend on member swap available for T.
                }                
                return *this;
            }

            /** \brief Operator-function to post a functor that should be executed synchronously using the internally stored object. 
             *
             * \param f F Functor to execute.
             * \return Anything that the functor returns.
             * \note This function will block until the lock could be acquired.
             */
            template<typename F>
            auto operator()(F f) const -> expected::value<decltype(f(__myT))>
            {
                std::lock_guard<std::mutex> guard(this->__lock);
                auto res = expected::result_of([&]() { return f(__myT); } );
                return res;                
            }  

        private:
            mutable T __myT;           /**< Value that should be modifiable through any executed functor */
            mutable std::mutex __lock; /**< Internally synchronized lock */
    };
}

#endif  // !__CONCURRENT_OBJECT_HPP__