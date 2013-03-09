#ifndef __CONCURRENT_SYNC_OBJECT_HPP__ 
#define __CONCURRENT_SYNC_OBJECT_HPP__

#include <atomic>
#include <exception>
#include <functional>
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

#endif  // !__CONCURRENT_SYNC_OBJECT_HPP__