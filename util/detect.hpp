#ifndef __UTIL_DETECT_HPP__
#define __UTIL_DETECT_HPP__ 

namespace detect
{
    template<typename T>
    struct has_member_swap
    {
        template<typename U, void (U::*)(T&)> struct SFINAE {}; // Signature of member swap regularly is void swap(T& other)
        template<typename U> static char Test(SFINAE<U, &U::swap>*);
        template<typename U> static int Test(...);
        static const bool value = sizeof(Test<T>(0)) == sizeof(char);
    };
}



#endif // __UTIL_DETECT_HPP__
