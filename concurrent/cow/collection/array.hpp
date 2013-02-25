#ifndef __COWARRAY_HPP 
#define __COWARRAY_HPP

#include "../CoW.hpp"

namespace cow
{
    template<typename T, std::size_t Size, typename Cloner = cow::internal::defaultCloner<T>>
    class array
    {
    public:
        template<typename... Args>
        void operator()(const std::size_t position, Args&&... params)
        {
            this->__values[position] = cow::make_cow<T, Cloner>(std::forward<Args&&...>(params...));
        }

        const cow::ptr<T, Cloner>& operator[](std::size_t position) const
        {
            return this->__values[position]; // Calls "const T& operator*() const" in cow::ptr
        }

        cow::ptr<T, Cloner>& operator[](std::size_t position) 
        {
            return this->__values[position]; // Calls "T& operator*()" in cow::ptr
        }

    private:
        cow::ptr<T, Cloner> __values[Size];
    };
}

#endif