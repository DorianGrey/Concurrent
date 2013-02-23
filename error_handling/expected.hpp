#ifndef __EXPECTED_HPP__
#define __EXPECTED_HPP__

#include <exception>
#include <type_traits>

#define WITH_CPP11_UNION 0 // Not yet supported by VS, thus disabled

/**
 * Based on Systematic Error Handling in C++, Andrei Alexandrescu
 * http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
 * 
 * Modified regarding the free creator function (outside the class), some naming and structure stuff.
 * TODO: naming conventions inside the class will be modified to fit the other parts of this package.
 */
namespace expected
{
    template <typename T>
    class value
    {
        public:
            value(const T& rhs) : ham(rhs), gotHam(true) {}

            value(T&& rhs) : ham(std::move(rhs)), gotHam(true) {}

            value(const value& rhs) : gotHam(rhs.gotHam) 
            {
                if (gotHam) 
                    new(std::addressof(ham)) T(rhs.ham);
                else 
                    new(std::addressof(spam)) std::exception_ptr(rhs.spam);
            }

            value(value&& rhs) : gotHam(rhs.gotHam) 
            {
                if (gotHam) 
                    new(std::addressof(ham)) T(std::move(rhs.ham));
                else 
                    new(std::addressof(spam)) std::exception_ptr(std::move(rhs.spam));
            }

            ~value() {}

            void swap(value& rhs) {
                if (gotHam) {
                    if (rhs.gotHam) {
                        using std::swap;
                        swap(ham, rhs.ham); // may call swap again
                    } else {
                        auto t = std::move(rhs.spam);
                        new(std::addressof(rhs.ham)) T(std::move(ham));
                        new(std::addressof(spam)) std::exception_ptr(t);
                        std::swap(gotHam, rhs.gotHam);
                    }
                } else {
                    if (rhs.gotHam) {
                        rhs.swap(*this);
                    } else {
                        std::swap(spam, rhs.spam); // inline function in <exception>
                        //spam.swap(rhs.spam);
                        std::swap(gotHam, rhs.gotHam);
                    }
                }
            }

            template <typename E>
            static value<T> from_exception(const E& exception) {
                if (typeid(exception) != typeid(E)) {
                    throw std::invalid_argument("slicing detected");
                }
                return from_exception(std::make_exception_ptr(exception));
            }

            static value<T> from_exception(std::exception_ptr p) {
                value<T> result;
                result.gotHam = false;
                new(std::addressof(result.spam)) std::exception_ptr(std::move(p));
                return result;
            }

            static value<T> from_exception() {
                return from_exception(std::current_exception());
            }

            bool valid() const {
                return gotHam;
            }

            T& get() {
                if (!gotHam) std::rethrow_exception(spam);
                return ham;
            }

            const T& get() const {
                if (!gotHam) std::rethrow_exception(spam);
                return ham;
            }

            template <typename E>
            bool hasException() const {
                try {
                    if (!gotHam) std::rethrow_exception(spam);
                } catch (const E& object) {
                    return true;
                } catch (...) {
                }
                return false;
            }

            /*template <typename F>
            static expected fromCode(F fun) {
                try {
                    return expected(fun());
                } catch (...) {
                    return fromException();
                }
            }*/

        private:
        #if WITH_CPP11_UNION !=0
            union 
            {
        #endif
                T ham;
                std::exception_ptr spam;
        #if WITH_CPP11_UNION != 0
            };
        #endif
            bool gotHam;
            value() {}
    };

    template<>
    class value<void>
    {
        public:
            value(void) : gotHam(true) {}
            value(const value& rhs) : gotHam(rhs.gotHam) 
            {
                if (!gotHam) 
                    new(std::addressof(spam)) std::exception_ptr(rhs.spam);
            }

            value(value&& rhs) : gotHam(rhs.gotHam) 
            {
                if (!gotHam) 
                    new(std::addressof(spam)) std::exception_ptr(std::move(rhs.spam));
            }

            ~value() {}

            void swap(value& rhs) {
                if (gotHam) {
                    if (!rhs.gotHam) {
                        auto t = std::move(rhs.spam);
                        new(std::addressof(spam)) std::exception_ptr(t);
                        std::swap(gotHam, rhs.gotHam);
                    }
                } else {
                    if (rhs.gotHam) {
                        rhs.swap(*this);
                    } else {
                        std::swap(spam, rhs.spam); // inline function in <exception>
                        std::swap(gotHam, rhs.gotHam);
                    }
                }
            }

            template <typename E>
            static value<void> from_exception(const E& exception) {
                if (typeid(exception) != typeid(E)) {
                    throw std::invalid_argument("slicing detected");
                }
                return from_exception(std::make_exception_ptr(exception));
            }

            static value<void> from_exception(std::exception_ptr p) {
                value<void> result;
                result.gotHam = false;
                new(std::addressof(result.spam)) std::exception_ptr(std::move(p));
                return result;
            }

            static value<void> from_exception() {
                return from_exception(std::current_exception());
            }

            bool valid() const {
                return gotHam;
            }

            template <typename E>
            bool hasException() const {
                try {
                    if (!gotHam) std::rethrow_exception(spam);
                } catch (const E& object) {
                    return true;
                } catch (...) {
                }
                return false;
            }

        private:
            std::exception_ptr spam;
            bool gotHam;
    };

    namespace detail
    {
        template<bool, typename T, typename F>
        struct result_of
        {
            static value<T> get(F fun)
            {
                return value<T>(fun());
            }
        };

        template<typename F>
        struct result_of<true, void, F>
        {
            static value<void> get(F fun)
            {
                fun();
                return value<void>();
            }
        };
    }


    template <typename F>
    auto result_of(F fun) -> value<decltype(fun())> {
        try {
            // template-based selection between functions that return nothing or something valuable - conditional compile-time selection for runtime execution
            return detail::result_of< std::is_same< decltype(fun()),void >::value, decltype(fun()), F >::get(fun); 
        } catch (...) {
            return value<decltype(fun())>::from_exception();
        }
    }

}






#endif