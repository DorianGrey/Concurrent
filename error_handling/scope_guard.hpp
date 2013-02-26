#ifndef __SCOPE_GUARD_HPP__
#define __SCOPE_GUARD_HPP__

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
            simple(Functor fu) : __isActive(true), __f(fu) {}
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

    template<typename Functor>
    simple<Functor> make(Functor f)
    {
        return simple<Functor>(f);
    }
}

#undef WITH_CPP_11__CTOR_DISABLING

#endif // __SCOPE_GUARD_HPP__
