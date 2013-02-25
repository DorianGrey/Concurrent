#ifndef __CLONER_HPP 
#define __CLONER_HPP

namespace cow
{
    namespace internal 
    {
        template<typename T>
        struct defaultCloner
        {
            T* operator()(const T* const base)
            {
                return new T(*base);
            }
        };        
    }
}

#endif