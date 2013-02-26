#ifndef __EXPECTED_HPP__
#define __EXPECTED_HPP__

#include <exception>
#include <type_traits>

#define WITH_CPP11__UNION 0 // Not yet supported by VS, thus disabled

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
            value(const T& rhs) : __data(rhs), __hasData(true) {}

            value(T&& rhs) : __data(std::move(rhs)), __hasData(true) {}

            value(const value& rhs) : __hasData(rhs.__hasData) 
            {
                if (this->__hasData) 
                    new(std::addressof(this->__data)) T(rhs.__data);
                else 
                    new(std::addressof(this->__excpt)) std::exception_ptr(rhs.__excpt);
            }

            value(value&& rhs) : __hasData(rhs.gotHam) 
            {
                if (this->__hasData) 
                    new(std::addressof(this->__data)) T(std::move(rhs.ham));
                else 
                    new(std::addressof(this->__excpt)) std::exception_ptr(std::move(rhs.__excpt));
            }

            ~value() {}

            void swap(value& rhs) {
                if (this->__hasData) {
                    if (rhs.__hasData) {
                        using std::swap;
                        swap(__data, rhs.__data); // may call swap again
                    } else {
                        auto t = std::move(rhs.__excpt);
                        new(std::addressof(rhs.__data)) T(std::move(__data));
                        new(std::addressof(this->__excpt)) std::exception_ptr(t);
                        std::swap(this->__hasData, rhs.__hasData);
                    }
                } else {
                    if (rhs.__hasData) {
                        rhs.swap(*this);
                    } else {
                        std::swap(this->__excpt, rhs.__excpt); // inline function in <exception>
                        //spam.swap(rhs.spam);
                        std::swap(this->__hasData, rhs.__hasData);
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
                result.__hasData = false;
                new(std::addressof(result.__excpt)) std::exception_ptr(std::move(p));
                return result;
            }

            static value<T> from_exception() {
                return from_exception(std::current_exception());
            }

            bool valid() const {
                return this->__hasData;
            }

            T& get() {
                if (!this->__hasData) std::rethrow_exception(this->__excpt);
                return this->__data;
            }

            const T& get() const {
                if (!this->__hasData) std::rethrow_exception(this->__excpt);
                return this->__data;
            }

            template <typename E>
            bool hasException() const {
                try {
                    if (!this->__hasData) std::rethrow_exception(this->__excpt);
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
        #if WITH_CPP11__UNION !=0
            union 
            {
        #endif
                T __data;
                std::exception_ptr __excpt;
        #if WITH_CPP11__UNION != 0
            };
        #endif
            bool __hasData;
            value() {}
    };

    template<>
    class value<void>
    {
        public:
            value(void) : __hasData(true) {}
            value(const value& rhs) : __hasData(rhs.__hasData) 
            {
                if (!this->__hasData) 
                    new(std::addressof(this->__excpt)) std::exception_ptr(rhs.__excpt);
            }

            value(value&& rhs) : __hasData(rhs.__hasData) 
            {
                if (!this->__hasData) 
                    new(std::addressof(this->__excpt)) std::exception_ptr(std::move(rhs.__excpt));
            }

            ~value() {}

            void swap(value& rhs) {
                if (this->__hasData) {
                    if (!rhs.__hasData) {
                        auto t = std::move(rhs.__excpt);
                        new(std::addressof(this->__excpt)) std::exception_ptr(t);
                        std::swap(this->__hasData, rhs.__hasData);
                    }
                } else {
                    if (rhs.__hasData) {
                        rhs.swap(*this);
                    } else {
                        std::swap(this->__excpt, rhs.__excpt); // inline function in <exception>
                        std::swap(this->__hasData, rhs.__hasData);
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
                result.__hasData = false;
                new(std::addressof(result.__excpt)) std::exception_ptr(std::move(p));
                return result;
            }

            static value<void> from_exception() {
                return from_exception(std::current_exception());
            }

            bool valid() const {
                return __hasData;
            }

            template <typename E>
            bool hasException() const {
                try {
                    if (!this->__hasData) std::rethrow_exception(this->__excpt);
                } catch (const E& object) {
                    return true;
                } catch (...) {
                }
                return false;
            }

        private:
            std::exception_ptr __excpt;
            bool __hasData;
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

#undef WITH_CPP11__UNION

#endif // __EXPECTED_HPP__