#ifndef __UTIL_MEMBER_SWAP_HPP__
#define __UTIL_MEMBER_SWAP_HPP__

#include <algorithm>

template<bool, typename T>
struct if_member_swap
{
    static void exec(T& lhs, T& rhs)
    {
        lhs = rhs;
    }
};

template<typename T>
struct if_member_swap<true, T>
{
    static void exec(T& lhs, T& rhs)
    {
        T temp(rhs);
        lhs.swap(temp);
    }
};

#endif // __UTIL_MEMBER_SWAP_HPP__
