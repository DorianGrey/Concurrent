#ifndef __COW_HPP__
#define __COW_HPP__

#include "internal/cloner.hpp"
#include <memory>
#include <type_traits>

namespace concurrent
{
    namespace cow
    {
        template<typename T, typename Cloner = cow::internal::defaultCloner<T>>
        class ptr
        {
            public:
                typedef T value_type;
                typedef std::shared_ptr<T> ptr_type;

            private:

                ptr_type __myVal;
                Cloner __cloner;

                void release()
                {
                    T* tmp = this->__myVal.get();
                    if (tmp != nullptr || this->__myVal.unique() )
                    {
                        this->__myVal = ptr_type(cloner(tmp)); // copy tmp, since it may be deleted unrecognized otherwise
                    }
                }

            public:

                // template<typename U, typename... Args> 
                // friend ptr<U> make_cow(Args&&... params);
                ptr() {}
                ptr(T* value) : __myVal(value) {}
                //ptr(T* value, Cloner cloner) : __myVal(value), __cloner(cloner) {}

                ptr(ptr_type value) : __myVal(value) {}
                //ptr(ptr_type value, Cloner cloner) : __myVal(value), __cloner(cloner) {}

                ptr(const ptr& rhs) : __myVal(rhs.__myVal) {}
                //ptr(const ptr& rhs, Cloner cloner) : __myVal(rhs.__myVal), __cloner(cloner) {}

                /*template<typename... Args>
                ptr(Args&&... params) : __myVal(std::make_shared<T>(std::forward<Args&&...>(params...)))
                {

                }*/

                const T& operator*() const
                {
                    return *this->__myVal;
                }

                T& operator*()
                {
                    this->detach();
                    return *this->__myVal;
                }

                const T* operator->() const
                {
                    return this->__myVal.operator->();
                }

                T* operator->()
                {
                    this->detach();
                    return this->__myVal.operator->();
                }

        };

        template<typename T, typename C = cow::internal::defaultCloner<T>, typename... Args> 
        ptr<T, C> make_cow(Args&&... params)
        {
            return ptr<T, C>(std::make_shared<T>(std::forward<Args&&...>(params...)));
        }
    }  
}



#endif // __COW_HPP__