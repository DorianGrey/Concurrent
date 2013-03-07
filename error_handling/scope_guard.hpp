#ifndef __SCOPE_GUARD_HPP__
#define __SCOPE_GUARD_HPP__

#include "expected.hpp"

#define WITH_CPP_11__CTOR_DISABLING 0  // explicit deletion is preferable, but not yet supported by VS

/**
 * The hierarchy is intended as:
 * - the namespace which indicates that everything inside is related to scope-guards
 * - a simple guard class that relies on a given functor
 * - a make function for it, so that the template parameter does not need to be given explicitly
 * - [TODO] Support other guard-types ?                                             
 */
namespace scope_guard
{
    template<typename Functor>
    class simple
    {
        public:
            simple(Functor&& fu) : __isActive(true), __f(fu) {}
            simple(simple&& rhs) : __isActive(rhs.__isActive), __f(rhs.__f)
            {
                rhs.__isActive = false; // disable the other to prevent its execution!
            }

            ~simple()
            {
                if (this->__isActive)
                {
                    try
                    {
                        this->__f();
                    }
                    catch (...) {}  // don't like any exception here
                }
            }

            void operator!() // Uses toggle mode, might be useful in several cases
            {
                this->__isActive = !this->__isActive;
            }

        private:
            bool __isActive;
            Functor __f;
        #if WITH_CPP_11__CTOR_DISABLING != 0
            simple(const simple& rhs) = delete;
            simple& operator=(const simple& rhs) = delete;
        #else
            simple(const simple& rhs);
            simple& operator=(const simple& rhs);
        #endif    
    };

    template<typename Functor1, typename Functor2>
    class do_undo
    {
        private:    
            Functor1 __do;
            Functor2 __undo;

        public:
            do_undo(Functor1&& f1, Functor2&& f2) : __isActive(true), __do(f1), __undo(f2) {}
            do_undo(do_undo&& rhs) : __isActive(rhs.__isActive), __do(rhs.__do), __undo(rhs.__undo)
            {
                rhs.__isActive = false; // disable the other to prevent its execution!
            }

            ~do_undo()
            {
                if (this->__isActive)
                {
                    try
                    {
                        this->__undo();
                    }
                    catch (...) {}  // don't like any exception here
                }
            }
            
            auto operator~() -> decltype(expected::result_of<Functor1>(__do))
            {
                return expected::result_of<Functor1>(__do);   
            } 

            void operator!() // Uses toggle mode, might be useful in several cases
            {
                this->__isActive = !this->__isActive;
            }

        private:
            bool __isActive;
            
        #if WITH_CPP_11__CTOR_DISABLING != 0
            do_undo(const do_undo& rhs) = delete;
            do_undo& operator=(const do_undo& rhs) = delete;
        #else
            do_undo(const do_undo& rhs);
            do_undo& operator=(const do_undo& rhs);
        #endif
    };

    template<typename Functor>
    simple<Functor> make(Functor&& f)
    {
        return simple<Functor>(std::forward<Functor&&>(f));
    }

    template<typename Functor1, typename Functor2>
    do_undo<Functor1, Functor2> make(Functor1&& f1, Functor2&& f2)
    {
        return do_undo<Functor1, Functor2>(std::forward<Functor1&& >(f1), std::forward<Functor2&&>(f2));
    }
}

#undef WITH_CPP_11__CTOR_DISABLING

#endif // __SCOPE_GUARD_HPP__
