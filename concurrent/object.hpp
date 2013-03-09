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
#include "util/member_swap.hpp"

/************************************************************************/
/* TODOS                                                                */
/* - Check for potential race condition copy/move                       */
/* - more tests                                                         */
/* - documentation                                                      */
/************************************************************************/

namespace concurrent
{
    /** \brief This is an asynchronous monitor class. It handles a parameter of the particular type, granting concurrent-safe access by sending functor-messages asynchronously.
     *         As a result, all return values are futures.
     *         It is based upon the concurrent<T> class presented by Herb Sutter.
     *         http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism
     *         Moving in an asynchronous object is rather complicated. Theoretically, we should move any open jobs, but that's not possible, due to their boundaries.
     *         Besides, data cannot be moved as long as it is (theoretically) modified. Considering this, moving is only possible when an object is not currently 
     *         in modification. Thus, we have to let the other object run any job left (same as in its d'tor), and fetch the data object afterwards. 
     *
     *  \param Data-type that should be covered by this class.
     */
    template<typename T>
    class async_object
    {
        public:
            /** \brief Default c'tor. If T has a default c'tor or can be constructed by uniform initialization, there is no need to give a particular instance.
             *
             * \param t T Value to handle inside this class.
             */
            async_object(T t = T{}) : __myT(t), __workerThread([=]() -> void { __done = false; while (!__done) { this->__innerqueue.pop()(); }}) // Little complicated, but that's the horror we face due to s.th. like a pop() ...
            {

            }

            /** \brief Copy c'tor. It sends a message to the rhs-object that ends up in a copy of its T value (either by copy-and-swap or not).
             *
             * \param rhs const async_object& Asynchronized object to copy from.
             * \note If this c'tor can be used depends on T having a copy-c'tor or not - you will get a compile-error if it does not!
             *       This c'tor will block until the rhs instance copied the requested value (i.e., it waits for the future value), so handle it with care!
             */
            async_object(const async_object& rhs) : __workerThread([=]() -> void { __done = false; while (!__done) { this->__innerqueue.pop()(); }})
            {
                static_assert( std::is_copy_constructible<T>::value, "T is not copy-constructible!" );
                if (this != std::addressof(rhs))
                {
                    auto res = rhs <= ([&](T& value) -> void {
                        // If T has a member-swap that takes another instance of T as a reference and returns void, it is considered to support the copy-and-swap-idiom.
                        // In this case, the call below will reach the function that uses this idiom to assign the rhs-T to the local one, otherwise, it will perform
                        // a simple assignment ( = ).
                        if_member_swap< detect::has_member_swap<T>::value, T >::exec(this->__myT, value);                               
                    });
                    res.wait(); // Wait for stable state
                    res.get(); // just to get the exception if one occurred!
                }
            }

            /** \brief Move c'tor. It sends a message to the rhs-object that ends up in a move of its T value.
             *
             * \param rhs async_object&& Asynchronized object to move from.
             * \note If this c'tor can be used depends on T having a move-c'tor or not - you will get a compile-error if it does not!
             *       This c'tor will block until the rhs instance move the requested value (i.e., it waits for the future value), so handle it with care!
             *       The effect on rhs depends on the effect defined on its T value's move-c'tor!
             */
            async_object(async_object&& rhs) : __workerThread([=]() -> void { __done = false; while (!__done) { this->__innerqueue.pop()(); }})
            {
                static_assert( std::is_move_constructible<T>::value, "T is not move-constructible!" );
                if (this != std::addressof(rhs))
                {
                    rhs.__innerqueue.push([&]() { rhs.__done = true; });
                    rhs.__workerThread.join();
                    this->__myT = std::move(rhs.__myT);
                }
            }

            /** \brief D'tor. It sends a message that causes the internal thread to run out and waits for it afterwards, thus, it may take some time until it is destructed correctly.
             *
             */
            ~async_object() 
            {
                this->__innerqueue.push([=]() { __done = true; });
                try
                {
                    this->__workerThread.join(); // [Note] We don't want any exception here. E.g., it may occur by joining an already finished thread.
                }
                catch (...) {}
            }

            /** \brief Assignment operator
             *
             * \param rhs const async_object& Synchronized object to copy from.
             * \return *this
             * \note Using this function depends on T having a defined assignment operator or not - you will get a compile-error if it does not have one!
             *       This assignment will block until the rhs instance copied the requested value (i.e., it waits for the future value), so handle it with care!
             *       The effect on rhs depends on the effect defined on its T value's move-c'tor!
             */
            async_object& operator=(const async_object& rhs)
            {
                static_assert( std::is_copy_assignable<T>::value, "T is not copy-assignable!" );
                if (this != std::addressof(rhs))
                {
                    auto res = rhs <= ([&](T& value) -> void {
                        // If T has a member-swap that takes another instance of T as a reference and returns void, it is considered to support the copy-and-swap-idiom.
                        // In this case, the call below will reach the function that uses this idiom to assign the rhs-T to the local one, otherwise, it will perform
                        // a simple assignment ( = ).
                        if_member_swap< detect::has_member_swap<T>::value, T >::exec(this->__myT, value);                               
                    });
                    res.wait(); // Wait for stable state
                    res.get(); // just to get the exception if one occurred!
                }                
                return *this;
            }

            /** \brief Assignment operator
             *
             * \param rhs async_object&& Synchronized object to move from.
             * \return *this
             * \note Using this function depends on T having a defined move-assignment operator or not - you will get a compile-error if it does not have one!
             *       This assignment will block until the rhs instance moved the requested value (i.e., it waits for the future value), so handle it with care!
             *       The effect on rhs depends on the effect defined on its T value's move-assignment operator!
             */
            async_object& operator=(async_object&& rhs)
            {
                static_assert( std::is_move_assignable<T>::value, "T is not move-assignable!" );
                if (this != std::addressof(rhs))
                {
                    rhs.__innerqueue.push([&]() { rhs.__done = true; }); // [Note] We're not using the "<=" operator here, but the worker queue.
                    rhs.__workerThread.join();
                    this->__myT = std::move(rhs.__myT);
                }                
                return *this;
            }

            /** \brief Operator-function to post a functor that should be executed asynchronously using the internally stored object. 
             *
             * \param f F Functor to execute.
             * \return Anything that the functor returns, as a std::future-value.
             * \note This function will not block, if you need the result to go on, you will have to wait on the future-value!
             */
            template<typename F>
            auto operator <= (F&& f) const -> std::future<decltype(f(__myT))>
            {              
                auto promisedRes = std::make_shared< std::promise<decltype(f(__myT))> >();
                auto ret = promisedRes->get_future();

                this->__innerqueue.push([=]() -> void { 
                    try
                    {
                        this->__set_value(*promisedRes, f);
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
            mutable concurrent::queue<std::function<void()>> __innerqueue;             /**< Internally synchronized queue */
            std::atomic_bool __done;                                                   /**< Indicator for the thread to run out */
            std::thread __workerThread;                                                /**< Worker thread */
            
            /** \brief Helper function that sets the value resulting from a functor if that result is not void.
             *         Separation is required due to std::future::set_value, which does not take a parameter when the result should be void.  
             *
             * \param Fut Type that is handled inside the given std::promise, i.e. the return-type of the functor.
             * \param F Type of the functor.
             * \param p std::promise<Fut>& Promise-value to write the result to; the result will be accessible to the future-value.
             * \param f F& Functor to apply on the local T instance.
             *
             */
            template<typename Fut, typename F>
            void __set_value(std::promise<Fut>& p, F& f) const 
            {
                p.set_value(f(this->__myT));
            }

            /** \brief Helper function that sets the value resulting from a functor if that result is void.
             *         Separation is required due to std::future::set_value, which does not take a parameter when the result should be void. 
             *
             * \param F Type of the functor.
             * \param p std::promise<void>& Promise-value to signal to.
             * \param f F& Functor to apply on the local T instance.
             */
            template<typename F>
            void __set_value(std::promise<void>& p, F& f) const 
            {
                f(this->__myT);
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

            /** \brief Copy c'tor. It acquires the remote lock to ensure that no-one is currently modifying the internal data. (either by copy-and-swap or not)
             *
             * \param rhs const sync_object& Synchronized object to copy from.
             * \note If this c'tor can be used depends on T having a copy-c'tor or not - you will get a compile-error if it does not!
             */
            sync_object(const sync_object& rhs)
            {
                static_assert( std::is_copy_constructible<T>::value, "T is not copy-constructible!" );
                if (this != std::addressof(rhs))
                {
                    std::lock_guard<std::mutex> guard(rhs.__lock);
                    std::lock_guard<std::mutex> guard2(this->__lock);
                    // If T has a member-swap that takes another instance of T as a reference and returns void, it is considered to support the copy-and-swap-idiom.
                    // In this case, the call below will reach the function that uses this idiom to assign the rhs-T to the local one, otherwise, it will perform
                    // a simple assignment ( = ).
                    if_member_swap< detect::has_member_swap<T>::value, T >::exec(this->__myT, rhs.__myT);
                }
            }

            /** \brief Move c'tor. It acquires the remote lock to ensure that no-one is currently modifying the internal data. 
             *
             * \param rhs sync_object&& Synchronized object to move from.
             * \note If this c'tor can be used depends on T having a copy-c'tor or not - you will get a compile-error if it does not!
             *       The effect on rhs depends on the effect defined on its T value's move-c'tor!   
             */
            sync_object(sync_object&& rhs)
            {
                static_assert( std::is_move_constructible<T>::value, "T is not move-constructible!" );
                if (this != std::addressof(rhs))
                {
                    std::lock_guard<std::mutex> guard(rhs.__lock);
                    std::lock_guard<std::mutex> guard2(this->__lock);
                    this->__myT = std::move(rhs.__myT);
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
             * \note Using this function depends on T having a defined assignment operator or not - you will get a compile-error if it does not have one!
             */
            sync_object& operator=(const sync_object& rhs)
            {
                static_assert( std::is_copy_assignable<T>::value, "T is not copy-assignable!" );
                if (this != std::addressof(rhs))
                {
                    std::lock_guard<std::mutex> guard(rhs.__lock); 
                    // If T has a member-swap that takes another instance of T as a reference and returns void, it is considered to support the copy-and-swap-idiom.
                    // In this case, the call below will reach the function that uses this idiom to assign the rhs-T to the local one, otherwise, it will perform
                    // a simple assignment ( = ).
                    if_member_swap< detect::has_member_swap<T>::value, T >::exec(this->__myT, rhs.__myT);
                }                
                return *this;
            }

            /** \brief Assignment operator
             *
             * \param rhs sync_object&& Synchronized object to move from.
             * \return *this
             * \note Using this function depends on T having a defined move-assignment operator or not - you will get a compile-error if it does not have one!
             */
            sync_object& operator=(sync_object&& rhs)
            {
                static_assert( std::is_move_assignable<T>::value, "T is not move-assignable!" );
                if (this != std::addressof(rhs))
                {
                    std::lock_guard<std::mutex> guard(rhs.__lock); 
                    this->__myT = std::move(rhs.__myT);
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
            auto operator <= (F&& f) const -> expected::value<decltype(f(__myT))>
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