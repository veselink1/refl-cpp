// The MIT License (MIT)
//
// Copyright (c) 2020 Veselin Karaganev (@veselink1) and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef REFL_INCLUDE_HPP
#define REFL_INCLUDE_HPP

#include <stddef.h> // size_t
#include <cstring>
#include <array>
#include <utility> // std::move, std::forward
#include <optional>
#include <tuple>
#include <type_traits>
#include <ostream>
#include <sstream>
#include <iomanip> // std::quoted

#if defined(__APPLE__)

#include <memory>
#include <complex>

#else

namespace std
{
    template <typename T, typename Deleter>
    class unique_ptr;

    template <typename T>
    class shared_ptr;

    template <typename T>
    class weak_ptr;

    template <typename T>
    class complex;
} // namespace std

#endif

#ifdef _MSC_VER
// Disable VS warning for "Not enough arguments for macro"
// (emitted when a REFL_ macro is not provided any attributes)
#pragma warning( disable : 4003 )
#endif

#if defined(__clang__)
  #if __has_feature(cxx_rtti)
    #define REFL_RTTI_ENABLED
  #endif
#elif defined(__GNUG__)
  #if defined(__GXX_RTTI)
    #define REFL_RTTI_ENABLED
  #endif
#elif defined(_MSC_VER)
  #if defined(_CPPRTTI)
    #define REFL_RTTI_ENABLED
  #endif
#endif

/**
 * @brief The top-level refl-cpp namespace
 * It contains a few core refl-cpp namespaces and directly exposes core classes and functions.
 * <ul>
 * <li>util - utility functions (for_each, map_to_tuple, etc.)</li>
 * <li>trait - type-traits and other operations on types (is_function_v, map_t, etc.)</li>
 * <li>runtime - utility functions and classes that always have a runtime overhead (proxy<T>, debug_str, etc.)</li>
 * <li>member - contains the empty classes member and function (used for tagging)</li>
 * <li>descriptor - contains the non-specialized member types (type|field_descriptor<T, N>, and operations on them (get_property, get_display_name, etc.))</li>
 * </ul>
 *
 * using util::type_list; <br>
 * using descriptor::type_descriptor; <br>
 * using descriptor::field_descriptor; <br>
 * using descriptor::function_descriptor; <br>
 * using util::const_string; <br>
 * using util::make_const_string; <br>
 */
namespace refl
{
    /**
     * @brief Contains utility types and functions for working with those types.
     */
    namespace util
    {
        /**
         * Converts a compile-time available const char* value to a const_string<N>.
         * The argument must be a *core constant expression* and be null-terminated.
         *
         * @see refl::util::const_string
         */
#define REFL_MAKE_CONST_STRING(CString) \
    (::refl::util::detail::copy_from_unsized<::refl::util::detail::strlen(CString)>(CString))

        /**
         * Represents a compile-time string. Used in refl-cpp
         * for representing names of reflected types and members.
         * Supports constexpr concatenation and substring,
         * and is explicitly-convertible to const char* and std::string.
         * REFL_MAKE_CONST_STRING can be used to create an instance from a literal string.
         *
         * @typeparam <N> The length of the string excluding the terminating '\0' character.
         * @see refl::descriptor::base_member_descriptor::name
         */
        template <size_t N>
        struct const_string
        {
            /** The largest positive value size_t can hold. */
            static constexpr size_t npos = static_cast<size_t>(-1);

            /** The length of the string excluding the terminating '\0' character. */
            static constexpr size_t size = N;

            /**
             * The statically-sized character buffer used for storing the string.
             */
            char data[N + 1];

            /**
             * Creates an empty const_string.
             */
            constexpr const_string() noexcept
                : data{}
            {
            }

            /**
             * Creates a copy of a const_string.
             */
            constexpr const_string(const const_string<N>& other) noexcept
                : const_string(other, std::make_index_sequence<N>())
            {
            }

            /**
             * Creates a const_string by copying the contents of data.
             */
            constexpr const_string(const char(&data)[N + 1]) noexcept
                : const_string(data, std::make_index_sequence<N>())
            {
            }

            /**
             * Explicitly converts to const char*.
             */
            explicit constexpr operator const char*() const noexcept
            {
                return data;
            }

            /**
             * Explicitly converts to std::string.
             */
            explicit operator std::string() const noexcept
            {
                return data;
            }

            /**
             * Returns a pointer to the contained zero-terminated string.
             */
            constexpr const char* c_str() const noexcept
            {
                return data;
            }

            /**
             * Returns the contained string as an std::string.
             */
            std::string str() const noexcept
            {
                return data;
            }

            /**
             * A constexpr version of std::string::substr.
             *
             * \code{.cpp}
             * make_const_string("Hello, World!").template substr<0, 4>() -> (const_string<4>) "Hell"
             * make_const_string("Hello, World!").template substr<1, 4>() -> (const_string<3>) "ell"
             * \endcode
             */
            template <size_t Pos, size_t Count = npos>
            constexpr auto substr() const noexcept
            {
                static_assert(Pos <= N);
                constexpr size_t NewSize = std::min(Count, N - Pos);

                char buf[NewSize + 1]{};
                for (size_t i = 0; i < NewSize; i++) {
                    buf[i] = data[Pos + i];
                }

                return const_string<NewSize>(buf);
            }

            /**
             * Searches the string for the first occurrence of the character and returns its position.
             *
             * \code{.cpp}
             * make_const_string("Hello, World!").find('e') -> 1
             * make_const_string("Hello, World!").find('z') -> static_cast<size_t>(-1)
             * \endcode
             */
            constexpr auto find(char ch, size_t pos = 0) const noexcept
            {
                for (size_t i = pos; i < N; i++) {
                    if (data[i] == ch) {
                        return i;
                    }
                }
                return npos;
            }

            /**
             * Searches the string for the last occurrence of the character and returns its position.
             *
             * \code{.cpp}
             * make_const_string("Hello, World!").rfind('o') -> 8
             * make_const_string("Hello, World!").rfind('z') -> static_cast<size_t>(-1)
             * \endcode
             */
            constexpr auto rfind(char ch, size_t pos = npos) const noexcept
            {
                for (size_t i = (pos == npos ? N - 1 : pos); i + 1 > 0; i--) {
                    if (data[i] == ch) {
                        return i;
                    }
                }
                return npos;
            }

        private:

            /**
             * Creates a copy of a const_string.
             */
            template <size_t... Idx>
            constexpr const_string(const const_string<N>& other, std::index_sequence<Idx...>) noexcept
                : data{ other.data[Idx]... }
            {
            }

            /**
             * Creates a const_string by copying the contents of data.
             */
            template <size_t... Idx>
            constexpr const_string(const char(&data)[sizeof...(Idx) + 1], std::index_sequence<Idx...>) noexcept
                : data{ data[Idx]... }
            {
            }

        };

        /**
         * Creates an empty instance of const_string<N>
         *
         * @see refl::util::const_string
         */
        constexpr const_string<0> make_const_string() noexcept
        {
            return {};
        }

        /**
         * Creates an instance of const_string<N>
         *
         * @see refl::util::const_string
         */
        template <size_t N>
        constexpr const_string<N - 1> make_const_string(const char(&str)[N]) noexcept
        {
            return str;
        }

        /**
         * Creates an instance of const_string<N>
         *
         * @see refl::util::const_string
         */
        constexpr const_string<1> make_const_string(char ch) noexcept
        {
            const char str[2]{ ch, '\0' };
            return make_const_string(str);
        }

        /**
         * Concatenates two const_strings together.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr const_string<N + M> operator+(const const_string<N>& a, const const_string<M>& b) noexcept
        {
            char data[N + M + 1] { };
            for (size_t i = 0; i < N; i++)
                data[i] = a.data[i];
            for (size_t i = 0; i < M; i++)
                data[N + i] = b.data[i];
            return data;
        }

        /**
         * Concatenates a const_string with a C-style string.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr const_string<N + M - 1> operator+(const const_string<N>& a, const char(&b)[M]) noexcept
        {
            return a + make_const_string(b);
        }

        /**
         * Concatenates a C-style string with a const_string.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr const_string<N + M - 1> operator+(const char(&a)[N], const const_string<M>& b) noexcept
        {
            return make_const_string(a) + b;
        }

        /**
         * Compares two const_strings for equality.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr bool operator==(const const_string<N>& a, const const_string<M>& b) noexcept
        {
            if constexpr (N != M) {
                return false;
            }
            else {
                for (size_t i = 0; i < M; i++) {
                    if (a.data[i] != b.data[i]) {
                        return false;
                    }
                }
                return true;
            }
        }

        /**
         * Compares two const_strings for equality.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr bool operator!=(const const_string<N>& a, const const_string<M>& b) noexcept
        {
            return !(a == b);
        }

        /**
         * Compares a const_string with a C-style string for equality.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr bool operator==(const const_string<N>& a, const char(&b)[M]) noexcept
        {
            return a == make_const_string(b);
        }

        /**
         * Compares a const_string with a C-style string for equality.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr bool operator!=(const const_string<N>& a, const char(&b)[M]) noexcept
        {
            return a != make_const_string(b);
        }

        /**
         * Compares a C-style string with a const_string for equality.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr bool operator==(const char(&a)[N], const const_string<M>& b) noexcept
        {
            return make_const_string(a) == b;
        }

        /**
         * Compares a C-style string with a const_string for equality.
         *
         * @see refl::util::const_string
         */
        template <size_t N, size_t M>
        constexpr bool operator!=(const char(&a)[N], const const_string<M>& b) noexcept
        {
            return make_const_string(a) != b;
        }

        template <size_t N>
        constexpr std::ostream& operator<<(std::ostream& os, const const_string<N>& str) noexcept
        {
            return os << str.c_str();
        }

        namespace detail
        {
            constexpr size_t strlen(const char* const str)
            {
                return *str ? 1 + strlen(str + 1) : 0;
            }

            template <size_t N>
            constexpr const_string<N> copy_from_unsized(const char* const str)
            {
                const_string<N> cstr;
                for (size_t i = 0; i < N; i++) {
                    cstr.data[i] = str[i];
                }
                return cstr;
            }
        } // namespace detail

        /**
         * Represents a compile-time list of types provided as variadic template parameters.
         * type_list is an empty TrivialType. Instances of it can freely be created to communicate
         * the list of represented types. type_lists support many standard operations that are
         * implicitly available with ADL-lookup. type_list is used by refl-cpp mostly to represent
         * the list of refl::field_descriptor, refl::function_descriptor specializations that
         * allow the compile-time reflection of a type's members.
         *
         * @see refl::util::for_each
         * @see refl::util::map_to_array
         * @see refl::util::map_to_tuple
         * @see refl::member_list
         *
         * # Examples
         * ```
         * for_each(type_list<int, float>(), [](auto) { ... });
         * ```
         */
        template <typename... Ts>
        struct type_list
        {
            /** The number of types in this type_list */
            static constexpr intptr_t size = sizeof...(Ts);
        };

        template <typename T>
        struct type_list<T>
        {
            typedef T type;
            static constexpr intptr_t size = 1;
        };

        template <typename T>
        using type_tag = type_list<T>;

    } // namespace util

    using util::const_string;
    using util::make_const_string;
    using util::type_list;
    using util::type_tag;

    /**
     * The contents of the refl::detail::macro_exports namespace
     * is implicitly available in the context of REFL_TYPE/FIELD/FUNC macros.
     * It is used to export the refl::attr:: standard attributes.
     */
    namespace detail
    {
        namespace macro_exports
        {
        }
    }

} // namespace refl

/**
 * refl_impl is an internal namespace that should not be used by the users of refl-cpp.
 */
namespace refl_impl
{
    /**
     * Contains the generated metadata types.
     * (i.e. type_info__)
     */
    namespace metadata
    {
        // Import everyting from macro_exports here to make it visible in REFL_ macro context.
        using namespace refl::detail::macro_exports;

        /**
         * The core reflection metadata type.
         * type_info__ holds data for a type T.
         *
         * The non-specialized type_info__ type has a member typedef invalid_marker
         * that can be used to detect it.
         *
         * Specializations of this type should provide all members of this
         * generic definition, except invalid_marker.
         *
         * @typeparam <T> The reflected type.
         */
        template <typename T>
        struct type_info__
        {
            /** Used for detecting this non-specialized type_info__ instance. */
            struct invalid_marker{};

            /**
             * This is a placeholder definition of which no type instances should be created.
             */
            template <size_t, typename>
            struct member;

            /** The number of reflected members of the target type T. */
            static constexpr size_t member_count{ 0 };

            /** This is a placeholder definition which shold not be referenced by well-formed programs. */
            static constexpr refl::const_string<0> name{ "" };

            /** This is a placeholder definition which shold not be referenced by well-formed programs. */
            static constexpr std::tuple<> attributes{ };
        };

        /**
         * Specializes type_info__ so that a type's const-qualification is effectively discarded.
         */
        template <typename T>
        struct type_info__<const T> : public type_info__<T> {};

        /**
         * Specializes type_info__ so that a type's volatile-qualification is effectively discarded.
         */
        template <typename T>
        struct type_info__<volatile T> : public type_info__<T> {};

        /**
         * Specializes type_info__ so that a type's const-volatile-qualification is effectively discarded.
         */
        template <typename T>
        struct type_info__<const volatile T> : public type_info__<T> {};

    } // namespace metadata

} // namespace refl_impl

namespace refl
{
    namespace detail
    {
        template <typename T>
        using type_info = refl_impl::metadata::type_info__<T>;

        template <typename T, size_t N>
        using member_info = typename type_info<T>::template member<N>;
    } // namespace detail

    /**
     * @brief Contains tag types denoting the different types of reflectable members.
     *
     * This namespace contains a number of empty types that correspond to
     * the different member types that refl-cpp supports reflection over.
     */
    namespace member
    {
        /**
         * An empty type which is equivalent to refl::member_descriptor_base::member_type
         * when the reflected member is a field.
         *
         * @see refl::descriptor::field_descriptor
         */
        struct field {};

        /**
         * An empty type which is equivalent to refl::member_descriptor_base::member_type
         * when the reflected member is a function.
         *
         * @see refl::descriptor::function_descriptor
         */
        struct function {};
    }

    namespace descriptor
    {
        template <typename>
        class type_descriptor;

        template <typename, size_t>
        class field_descriptor;

        template <typename, size_t>
        class function_descriptor;
    } // namespace descriptor

    /**
     * @brief Provides type-level operations for refl-cpp related use-cases.
     *
     * The refl::trait namespace provides type-level operations useful
     * for compile-time metaprogramming.
     */
    namespace trait
    {/**
         * Removes all reference and cv-qualifiers from T.
         * Equivalent to std::remove_cvref which is not currently
         * available on all C++17 compilers.
         */
        template <typename T>
        struct remove_qualifiers
        {
            typedef std::remove_cv_t<std::remove_reference_t<T>> type;
        };

        /**
         * Removes all reference and cv-qualifiers from T.
         * Equivalent to std::remove_cvref_t which is not currently
         * available on all C++17 compilers.
         */
        template <typename T>
        using remove_qualifiers_t = typename remove_qualifiers<T>::type;

        namespace detail
        {
            /** SFIANE support for detecting whether there is a type_info__ specialization for T. */
            template <typename T>
            decltype(typename refl::detail::type_info<T>::invalid_marker{}, std::false_type{}) is_reflectable_test(int);

            /** SFIANE support for detecting whether there is a type_info__ specialization for T. */
            template <typename T>
            std::true_type is_reflectable_test(...);
        } // namespace detail

        /**
         * Checks whether there is reflection metadata for the type T.
         * Inherits from std::bool_constant<>
         *
         * @see REFL_TYPE
         * @see REFL_AUTO
         * @see refl::is_reflectable
         */
        template <typename T>
        struct is_reflectable : decltype(detail::is_reflectable_test<T>(0))
        {
        };

        /**
         * Checks whether there is reflection metadata for the type T.
         * Inherits from std::bool_constant<>
         *
         * @see refl::trait::is_reflectable
         */
        template <typename T>
        [[maybe_unused]] static constexpr bool is_reflectable_v{ is_reflectable<T>::value };

        namespace detail
        {
            /** SFIANE support for detecting whether the type T supports member .begin() and .end() operations. */
            template <typename U>
            [[maybe_unused]] static auto is_container_test(int) -> decltype(std::declval<U>().begin(), std::declval<U>().end(), std::true_type{});

            /** SFIANE support for detecting whether the type T supports member .begin() and .end() operations. */
            template <typename U>
            [[maybe_unused]] static std::false_type is_container_test(...);
        }

        /**
         * Checks whether objects of the type T support member .begin() and .end() operations.
         */
        template <typename T>
        struct is_container : decltype(detail::is_container_test<T>(0))
        {
        };

        /**
         * Checks whether objects of the type T support member .begin() and .end() operations.
         */
        template <typename T>
        [[maybe_unused]] static constexpr bool is_container_v{ is_container<T>::value };

        namespace detail
        {
            template <size_t N, typename... Ts>
            struct get;

            template <size_t N>
            struct get<N>
            {
                static_assert(N > 0, "Missing arguments list for get<N, Ts...>!");
            };

            template <size_t N, typename T, typename... Ts>
            struct get<N, T, Ts...> : public get<N - 1, Ts...>
            {
            };

            template <typename T, typename... Ts>
            struct get<0, T, Ts...>
            {
                typedef T type;
            };

            template <size_t N, typename...>
            struct skip;

            template <size_t N, typename T, typename... Ts>
            struct skip<N, T, Ts...> : skip<N - 1, Ts...>
            {
            };

            template <typename T, typename... Ts>
            struct skip<0, T, Ts...>
            {
                typedef type_list<T, Ts...> type;
            };

            template <>
            struct skip<0>
            {
                typedef type_list<> type;
            };
        }

        /// \private
        template <size_t, typename>
        struct get;

        /**
         * Provides a member typedef type which is the
         * N-th type in the provided type_list.
         *
         * \code{.cpp}
         * typename get<0, type_list<int, float>>::type -> int
         * typename get<1, type_list<int, float>>::type -> float
         * \endcode
         */
        template <size_t N, typename... Ts>
        struct get<N, type_list<Ts...>> : detail::get<N, Ts...>
        {
        };

        /**
         * The N-th type in the provided type_list.
         * @see get
         */
        template <size_t N, typename TypeList>
        using get_t = typename get<N, TypeList>::type;

        /// \private
        template <size_t, typename>
        struct skip;

        /**
         * Skips the first N types in the provided type_list.
         * Provides a member typedef equivalent to the resuling type_list.
         *
         * \code{.cpp}
         * typename skip<1, type_list<int, float, double>>::type -> type_list<float, double>
         * typename skip<0, type_list<int, float, double>>::type -> type_list<int, float, double>
         * \endcode
         */
        template <size_t N, typename... Ts>
        struct skip<N, type_list<Ts...>> : detail::skip<N, Ts...>
        {
        };

        /**
         * Skips the first N types in the provided type_list.
         * @see skip
         */
        template <size_t N, typename TypeList>
        using skip_t = typename skip<N, TypeList>::type;

        /// \private
        template <typename>
        struct as_type_list;

        /**
         * Provides a member typedef type which is a type_list with
         * template type parameters equivalent to the type parameters of the provided
         * type. The provided type must be a template instance.
         *
         * \code{.cpp}
         * typename as_type_list<std::tuple<int, float>>::type -> type_list<int, float>
         * \endcode
         */
        template <template <typename...> typename T, typename... Ts>
        struct as_type_list<T<Ts...>>
        {
            typedef type_list<Ts...> type;
        };

        /// \private
        template <typename T>
        struct as_type_list : as_type_list<remove_qualifiers_t<T>>
        {
        };

        /**
         * A typedef for a type_list with
         * template type parameters equivalent to the type parameters of the provided
         * type. The provided type must be a template instance.
         * @see as_type_list
         */
        template <typename T>
        using as_type_list_t = typename as_type_list<T>::type;

        /// \private
        template <typename>
        struct as_tuple;

        /**
         * Provides a member typedef which is a std::tuple specialization with
         * template type parameters equivalent to the type parameters of the provided
         * type. The provided type must be a template specialization.
         *
         * \code{.cpp}
         * typename as_tuple<type_list<int, float>>::type -> std::tuple<int, float>
         * \endcode
         */
        template <template <typename...> typename T, typename... Ts>
        struct as_tuple<T<Ts...>>
        {
            typedef std::tuple<Ts...> type;
        };

        /// \private
        template <typename T>
        struct as_tuple : as_tuple<remove_qualifiers_t<T>>
        {
        };

        /**
         * A typedef for a std::tuple specialization with
         * template type parameters equivalent to the type parameters of the provided
         * type. The provided type must be a template specialization.
         * @see as_tuple
         */
        template <typename T>
        using as_tuple_t = typename as_tuple<T>::type;

        /**
         * Accesses first type in the list.
         */
        template <typename TypeList>
        using first = get<0, TypeList>;

        /**
         * Accesses last type in the list.
         * @see last
         */
        template <typename TypeList>
        using first_t = typename first<TypeList>::type;

        /**
         * Accesses last type in the list.
         */
        template <typename TypeList>
        using last = get<TypeList::size - 1, TypeList>;

        /**
         * Accesses last type in the list.
         * @see last
         */
        template <typename TypeList>
        using last_t = typename last<TypeList>::type;

        /**
         * Returns all but the first element of the list.
         */
        template <typename TypeList>
        using tail = skip<1, TypeList>;

        /**
         * Returns all but the first element of the list.
         * @see tail
         */
        template <typename TypeList>
        using tail_t = typename tail<TypeList>::type;

        namespace detail
        {
            template <typename, size_t, typename>
            struct take;

            template <typename... Us>
            struct take<type_list<Us...>, 0, type_list<>>
            {
                using type = type_list<Us...>;
            };

            template <typename... Us, typename T, typename... Ts>
            struct take<type_list<Us...>, 0, type_list<T, Ts...>>
            {
                using type = type_list<Us...>;
            };

            template <size_t N, typename... Us, typename T, typename... Ts>
            struct take<type_list<Us...>, N, type_list<T, Ts...>>
            {
                using type = typename take<type_list<Us..., T>, N - 1, type_list<Ts...>>::type;
            };
        }

        /**
         * Returns the first N elements of the list.
         */
        template <size_t N, typename TypeList>
        using take = detail::take<type_list<>, N, TypeList>;

        /**
         * Returns the first N elements of the list.
         */
        template <size_t N, typename TypeList>
        using take_t = typename take<N, TypeList>::type;

        /**
         * Returns all but the last element of the list.
         */
        template <typename TypeList>
        using init = take<TypeList::size - 1, TypeList>;

        /**
         * Returns all but the last element of the list.
         * @see tail
         */
        template <typename TypeList>
        using init_t = typename init<TypeList>::type;

        namespace detail
        {
            template <typename, typename>
            struct reverse_impl;

            template <typename... Us>
            struct reverse_impl<type_list<Us...>, type_list<>>
            {
                using type = type_list<Us...>;
            };

            template <typename... Us, typename T, typename... Ts>
            struct reverse_impl<type_list<Us...>, type_list<T, Ts...>>
            {
                using type = typename reverse_impl<type_list<T, Us...>, type_list<Ts...>>::type;
            };
        } // namespace detail

        /**
         * Reverses a list of types.
         *
         * \code{.cpp}
         * typename reverse<type_list<int, float>>::type -> type_list<float, int>
         * \endcode
         */
        template <typename TypeList>
        struct reverse : detail::reverse_impl<type_list<>, TypeList>
        {
        };

        /**
         * Reverses a list of types.
         * @see reverse
         */
        template <typename TypeList>
        using reverse_t = typename reverse<TypeList>::type;

        /**
         * Concatenates N lists together.
         *
         * \code{.cpp}
         * typename concat<type_list<int, float>, type_list<double>, type_list<long>>::type -> type_list<int, float, double, long>
         * \endcode
         */
        template <typename...>
        struct concat;

        /// \private
        template <>
        struct concat<>
        {
            using type = type_list<>;
        };

        /// \private
        template <typename... Ts>
        struct concat<type_list<Ts...>>
        {
            using type = type_list<Ts...>;
        };

        /**
         * Concatenates two lists together.
         */
        /// \private
        template <typename... Ts, typename... Us>
        struct concat<type_list<Ts...>, type_list<Us...>>
        {
            using type = type_list<Ts..., Us...>;
        };

        /**
         * Concatenates N lists together.
         */
        /// \private
        template <typename TypeList1, typename TypeList2, typename... TypeLists>
        struct concat<TypeList1, TypeList2, TypeLists...> : concat<typename concat<TypeList1, TypeList2>::type, TypeLists...>
        {
        };

        /**
         * Concatenates two lists together.
         * @see concat
         */
        template <typename... Ts>
        using concat_t = typename concat<Ts...>::type;

        /**
         * Appends a type to the list.
         */
        template <typename T, typename TypeList>
        struct append : concat<TypeList, type_list<T>>
        {
        };

        /**
         * Appends a type to the list.
         * @see prepend
         */
        template <typename T, typename TypeList>
        using append_t = typename append<T, TypeList>::type;

        template <typename, typename>
        struct prepend;

        /**
         * Prepends a type to the list.
         */
        template <typename T, typename TypeList>
        struct prepend : concat<type_list<T>, TypeList>
        {
        };

        /**
         * Prepends a type to the list.
         * @see prepend
         */
        template <typename T, typename TypeList>
        using prepend_t = typename prepend<T, TypeList>::type;

        namespace detail
        {
            template <template<typename> typename, typename...>
            struct filter_impl;

            template <template<typename> typename Predicate>
            struct filter_impl<Predicate>
            {
                using type = type_list<>;
            };

            template <template<typename> typename Predicate, typename Head, typename... Tail>
            struct filter_impl<Predicate, Head, Tail...>
            {
                using type = std::conditional_t<Predicate<Head>::value,
                    prepend_t<Head, typename filter_impl<Predicate, Tail...>::type>,
                    typename filter_impl<Predicate, Tail...>::type
                >;
            };

            template <template<typename> typename, typename...>
            struct map_impl;

            template <template<typename> typename Mapper>
            struct map_impl<Mapper>
            {
                using type = type_list<>;
            };

            template <template<typename> typename Mapper, typename Head, typename ...Tail>
            struct map_impl<Mapper, Head, Tail...>
            {
                using type = typename prepend<typename Mapper<Head>::type,
                    typename map_impl<Mapper, Tail...>::type>::type;
            };
        }

        /// \private
        template <template<typename> typename, typename>
        struct filter;

        /**
         * Filters a type_list according to a predicate template.
         *
         * \code{.cpp}
         * typename filter<std::is_reference, type_list<int, float&, double>>::type -> type_list<float&>
         * \endcode
         */
        template <template<typename> typename Predicate, typename... Ts>
        struct filter<Predicate, type_list<Ts...>>
        {
            using type = typename detail::filter_impl<Predicate, Ts...>::type;
        };

        /**
         * Filters a type_list according to a predicate template
         * with a static boolean member named "value" (e.g. std::is_trivial)
         * @see filter
         */
        template <template<typename> typename Predicate, typename TypeList>
        using filter_t = typename filter<Predicate, TypeList>::type;

        /// \private
        template <template<typename> typename, typename>
        struct map;

        /**
         * Transforms a type_list according to a predicate template.
         *
         * \code{.cpp}
         * typename map<std::add_reference, type_list<int, float&, double>>::type -> type_list<int&, float&, double&>
         * \endcode
         */
        template <template<typename> typename Mapper, typename... Ts>
        struct map<Mapper, type_list<Ts...>>
        {
            using type = typename detail::map_impl<Mapper, Ts...>::type;
        };

        /**
         * Transforms a type_list according to a predicate template
         * with a typedef named "type" (e.g. std::remove_reference)
         * @see map
         */
        template <template<typename> typename Mapper, typename... Ts>
        using map_t = typename map<Mapper, Ts...>::type;

        namespace detail
        {
            template <typename T>
            struct is_instance : public std::false_type {};

            template <template<typename...> typename T, typename... Args>
            struct is_instance<T<Args...>> : public std::true_type {};
        } // namespace detail

        /**
         * Detects whether T is a template specialization.
         * Inherits from std::bool_constant<>.
         *
         * \code{.cpp}
         * is_instance<type_list<>>::value -> true
         * is_instance<int>::value -> false
         * \endcode
         */
        template <typename T>
        struct is_instance : detail::is_instance<T>
        {
        };

        /**
         * Detects whether T is a template specialization.
         * @see is_instance
         */
        template <typename T>
        [[maybe_unused]] static constexpr bool is_instance_v{ is_instance<T>::value };

        namespace detail
        {
            /**
             * Checks if T == U<Args...>.
             * If U<Args...> != T or is invalid the result is false.
             */
            template <typename T, template<typename...> typename U, typename... Args>
            struct is_same_template
            {
                template <template<typename...> typename V, typename = V<Args...>>
                static auto test(int) -> std::is_same<V<Args...>, T>;

                template <template<typename...> typename V>
                static std::false_type test(...);

                static constexpr bool value{decltype(test<U>(0))::value};
            };

            template <template<typename...> typename T, typename U>
            struct is_instance_of : public std::false_type {};

            template <template<typename...> typename T, template<typename...> typename U, typename... Args>
            struct is_instance_of<T, U<Args...>> : public is_same_template<U<Args...>, T, Args...>
            {
            };
        }

        /**
         * Detects whther the type U is a template specialization of T.
         * (e.g. is_instance_of<std::vector<>, std::vector<int>>)
         * Inherits from std::bool_constant<>.
         *
         * \code{.cpp}
         * is_instance_of<type_list, type_list<int>>::value -> true
         * is_instance_of<type_list, std::tuple<int>>::value -> false
         * \endcode
         */
        template <template<typename...>typename T, typename U>
        struct is_instance_of : detail::is_instance_of<T, std::remove_cv_t<U>>
        {
        };

        /**
         * Detects whther the type U is a template specialization of T.
         * @see is_instance_of_v
         */
        template <template<typename...>typename T, typename U>
        [[maybe_unused]] static constexpr bool is_instance_of_v{ is_instance_of<T, U>::value };

        /// \private
        template <typename, typename>
        struct contains;

        /**
         * Checks whether T is contained in the list of types.
         * Inherits from std::bool_constant<>.
         *
         * \code{.cpp}
         * contains<int, type_list<int, float>>::value -> true
         * contains<double, type_list<int, float>>::value -> false
         * \endcode
         */
        template <typename T, typename... Ts>
        struct contains<T, type_list<Ts...>> : std::disjunction<std::is_same<std::remove_cv_t<T>, std::remove_cv_t<Ts>>...>
        {
        };

        /**
         * Checks whether T is contained in the list of types.
         * @see contains
         */
        template <typename T, typename TypeList>
        [[maybe_unused]] static constexpr bool contains_v = contains<T, TypeList>::value;

        /// \private
        template <template<typename...> typename, typename>
        struct contains_instance;

        /**
         * Checks whether an instance of the template T is contained in the list of types.
         * Inherits from std::bool_constant<>.
         *
         * \code{.cpp}
         * contains_instance<std::tuple, type_list<int, float, std::tuple<short, double>>>::value -> true
         * contains_instance<std::vector, type_list<int, float, std::tuple<short, double>>>::value -> false
         * \endcode
         */
        template <template<typename...> typename T, typename... Ts>
        struct contains_instance<T, type_list<Ts...>> : std::disjunction<trait::is_instance_of<T, std::remove_cv_t<Ts>>...>
        {
        };

        /**
         * Checks whether an instance of the template T is contained in the list of types.
         * @see contains_instance
         */
        template <template<typename...> typename T, typename TypeList>
        [[maybe_unused]] static constexpr bool contains_instance_v = contains_instance<T, TypeList>::value;

        /// \private
        template <typename, typename>
        struct contains_base;

        /**
         * Checks whether a type deriving from T is contained in the list of types.
         * Inherits from std::bool_constant<>.
         *
         * \code{.cpp}
         * struct Base {};
         * struct Derived : Base {};
         * contains_base<Base, type_list<int, float, Derived>>::value -> true
         * contains_base<Base, type_list<int, float, Base>>::value -> true
         * contains_base<int, type_list<int, float, Derived>>::value -> false
         * \endcode
         */
        template <typename T, typename... Ts>
        struct contains_base<T, type_list<Ts...>> : std::disjunction<std::is_base_of<std::remove_cv_t<T>, std::remove_cv_t<Ts>>...>
        {
        };

        /**
         * Checks whether a type deriving from T is contained in the list of types.
         * @see contains_base
         */
        template <typename T, typename TypeList>
        [[maybe_unused]] static constexpr bool contains_base_v = contains_base<T, TypeList>::value;

        namespace detail
        {
            template <typename T, ptrdiff_t N, typename... Ts>
            constexpr ptrdiff_t index_of() noexcept
            {
                if constexpr (sizeof...(Ts) <= N) return -1;
                else if constexpr (std::is_same_v<T, trait::get_t<N, type_list<Ts...>>>) return N;
                else return index_of<T, N + 1, Ts...>();
            }

            template <typename T, ptrdiff_t N, typename... Ts>
            constexpr ptrdiff_t index_of_base() noexcept
            {
                if constexpr (sizeof...(Ts) <= N) return -1;
                else if constexpr (std::is_base_of_v<T, trait::get_t<N, type_list<Ts...>>>) return N;
                else return index_of_base<T, N + 1, Ts...>();
            }

            template <template<typename...> typename T, ptrdiff_t N, typename... Ts>
            constexpr ptrdiff_t index_of_instance() noexcept
            {
                if constexpr (sizeof...(Ts) <= N) return -1;
                else if constexpr (is_instance_of_v<T, trait::get_t<N, type_list<Ts...>>>) return N;
                else return index_of_instance<T, N + 1, Ts...>();
            }

            // This variable template was introduced to fix the build on VS2017, which
            // chokes when invoking index_of_instance() directly from struct index_of_instance.
            template <template<typename...> typename T, ptrdiff_t N, typename... Ts>
            static constexpr ptrdiff_t index_of_instance_v = index_of_instance<T, N, Ts...>();
        } // namespace detail

        /// \private
        template <typename, typename>
        struct index_of;

        /**
         * The index of the type in the type list, -1 if it doesn't exist.
         * @see contains
         */
        template <typename T, typename... Ts>
        struct index_of<T, type_list<Ts...>> : std::integral_constant<ptrdiff_t, detail::index_of<T, 0, Ts...>()>
        {
        };

        /**
         * The index of the type in the type list, -1 if it doesn't exist.
         * @see index_of
         */
        template <typename T, typename TypeList>
        static constexpr ptrdiff_t index_of_v = index_of<T, TypeList>::value;

        /// \private
        template <typename, typename>
        struct index_of_base;

        /**
         * The index of the type in the type list that is derived from T, -1 if it doesn't exist.
         * @see contains_base
         */
        template <typename T, typename... Ts>
        struct index_of_base<T, type_list<Ts...>> : std::integral_constant<ptrdiff_t, detail::index_of_base<T, 0, Ts...>()>
        {
        };

        /**
         * The index of the type in the type list that is derived from T, -1 if it doesn't exist.
         * @see index_of_base
         */
        template <typename T, typename TypeList>
        static constexpr ptrdiff_t index_of_base_v = index_of_base<T, TypeList>::value;

        /// \private
        template <template<typename...> typename, typename>
        struct index_of_instance;

        /**
         * The index of the type in the type list that is a template instance of T, -1 if it doesn't exist.
         * @see contains_instance
         */
        template <template<typename...> typename T, typename... Ts>
        struct index_of_instance<T, type_list<Ts...>> : std::integral_constant<ptrdiff_t, detail::index_of_instance_v<T, 0, Ts...>>
        {
        };

        /**
         * The index of the type in the type list that is a template instance of T, -1 if it doesn't exist.
         * @see index_of_instance
         */
        template <template<typename...> typename T, typename TypeList>
        static constexpr ptrdiff_t index_of_instance_v = index_of_instance<T, TypeList>::value;

        namespace detail
        {
            template <typename, typename>
            struct unique_impl;

            template <typename UniqueList>
            struct unique_impl<UniqueList, type_list<>>
            {
                using type = UniqueList;
            };

            template <typename UniqueList, typename T, typename... Ts>
            struct unique_impl<UniqueList, type_list<T, Ts...>> :
                std::conditional_t<contains_v<T, UniqueList>,
                    unique_impl<UniqueList, type_list<Ts...>>,
                    unique_impl<append_t<T, UniqueList>, type_list<Ts...>>>
            {
            };
        } // namespace detail

        /**
         * Creates a new list containing the repeating elements in the source list only once.
         *
         * \code{.cpp}
         * typename unique<type_list<int, float, int>>::type -> type_list<int, float>
         * \endcode
         */
        template <typename T>
        struct unique : detail::unique_impl<type_list<>, T>
        {
        };

        /**
         * Creates a new list containing the repeating elements in the source list only once.
         */
        template <typename T>
        using unique_t = typename unique<T>::type;

    } // namespace trait

    namespace util
    {
        /**
         * Ignores all parameters. Can take an optional template parameter
         * specifying the return type of ignore. The return object is iniailized by {}.
         */
        template <typename T = int, typename... Ts>
        constexpr int ignore(Ts&&...) noexcept
        {
            return {};
        }

        /**
         * Returns the input paratemeter as-is. Useful for expanding variadic
         * template lists when only one arguments is known to be present.
         */
        template <typename T>
        constexpr decltype(auto) identity(T&& t) noexcept
        {
            return std::forward<T>(t);
        }

        /**
         * Adds const to the input reference.
         */
        template <typename T>
        constexpr const T& make_const(const T& value) noexcept
        {
            return value;
        }

        /**
         * Adds const to the input reference.
         */
        template <typename T>
        constexpr const T& make_const(T& value) noexcept
        {
            return value;
        }

        /**
        * Creates an array of type 'T' from the provided tuple.
        * The common type T needs to be specified, in order to prevent any
        * errors when using the overload taking an empty std::tuple (as there is no common type then).
        */
        template <typename T, typename... Ts>
        constexpr std::array<T, sizeof...(Ts)> to_array(const std::tuple<Ts...>& tuple) noexcept
        {
            return std::apply([](auto&& ... args) -> std::array<T, sizeof...(Ts)> { return { std::forward<decltype(args)>(args)... }; }, tuple);
        }

        /**
         * Creates an empty array of type 'T.
         */
        /// \private
        template <typename T>
        constexpr std::array<T, 0> to_array(const std::tuple<>&) noexcept
        {
            return {};
        }

        namespace detail
        {
            template <typename T, size_t... Idx>
            constexpr auto to_tuple([[maybe_unused]] const std::array<T, sizeof...(Idx)>& array, std::index_sequence<Idx...>) noexcept
            {
                if constexpr (sizeof...(Idx) == 0) return std::tuple<>{};
                else return std::make_tuple(std::get<Idx>(array)...);
            }
        }

        /**
         * Creates a tuple from the provided array.
         */
        template <typename T, size_t N>
        constexpr auto to_tuple(const std::array<T, N>& array) noexcept
        {
            return detail::to_tuple<T>(array, std::make_index_sequence<N>{});
        }

        /**
         * Creates a matching std::tuple from a type_list.
         * Types in the type_list must be Trivial.
         */
        template <typename... Ts>
        constexpr std::tuple<Ts...> as_tuple(type_list<Ts...>) noexcept
        {
            static_assert((... && std::is_trivial_v<Ts>), "Non-trivial types in type_list as not allowed!");
            return {};
        }

        /**
         * Creates a matching type_list from a std::tuple.
         */
        template <typename... Ts>
        constexpr type_list<Ts...> as_type_list(const std::tuple<Ts...>&) noexcept
        {
            return {};
        }

        namespace detail
        {
            template <typename F, typename T>
            constexpr auto invoke_optional_index(F&& f, T&& t, size_t idx, int) -> decltype(f(std::forward<T>(t), idx))
            {
                return f(std::forward<T>(t), idx);
            }

            template <typename F, typename T>
            constexpr auto invoke_optional_index(F&& f, T&& t, size_t, ...) -> decltype(f(std::forward<T>(t)))
            {
                return f(std::forward<T>(t));
            }

            template <typename F, typename... Carry>
            constexpr auto eval_in_order_to_tuple(type_list<>, std::index_sequence<>, F&&, Carry&&... carry)
            {
                if constexpr (sizeof...(Carry) == 0) return std::tuple<>{};
                else return std::make_tuple(std::forward<Carry>(carry)...);
            }
            // This workaround is needed since C++ does not specify
            // the order in which function arguments are evaluated and this leads
            // to incorrect order of evaluation (noticeable when using indexes).
            // Otherwise we could simply do std::make_tuple(f(Ts{}, Idx)...).
            template <typename F, typename T, typename... Ts, size_t I, size_t... Idx, typename... Carry>
            constexpr auto eval_in_order_to_tuple(type_list<T, Ts...>, std::index_sequence<I, Idx...>, F&& f, Carry&&... carry)
            {
                static_assert(std::is_trivial_v<T>, "Argument is a non-trivial type!");

                auto&& result = invoke_optional_index(f, T{}, I, 0);
                return eval_in_order_to_tuple(
                    type_list<Ts...>{},
                    std::index_sequence<Idx...>{},
                    std::forward<F>(f),
                    std::forward<Carry>(carry)..., // carry the previous results over
                    std::forward<decltype(result)>(result) // pass the current result after them
                );
            }
        }

        /**
         * Applies function F to each type in the type_list, aggregating
         * the results in a tuple. F can optionally take an index of type size_t.
         *
         * \code{.cpp}
         * map_to_tuple(reflect_types(type_list<int, float, double>{}), [](auto td) {
         *   return get_name(td);
         * })
         *   -> std::tuple{const_string{"int"}, const_string{"float"}, const_string{"double"}}
         * \endcode
         */
        template <typename F, typename... Ts>
        constexpr auto map_to_tuple(type_list<Ts...> list, F&& f)
        {
            return detail::eval_in_order_to_tuple(list, std::make_index_sequence<sizeof...(Ts)>{}, std::forward<F>(f));
        }

        /**
         * Applies function F to each type in the type_list, aggregating
         * the results in an array. F can optionally take an index of type size_t.
         *
         * \code{.cpp}
         * map_to_array<std::string>(reflect_types(type_list<int, float, double>{}), [](auto td) {
         *   return get_name(td).str();
         * })
         *   -> std::array{std::string{"int"}, std::string{"float"}, std::string{"double"}}
         * \endcode
         */
        template <typename T, typename F, typename... Ts>
        constexpr auto map_to_array(type_list<Ts...> list, F&& f)
        {
            return to_array<T>(map_to_tuple(list, std::forward<F>(f)));
        }

        /**
         * Applies function F to each type in the type_list.
         * F can optionally take an index of type size_t.
         *
         * \code{.cpp}
         * for_each(reflect_types(type_list<int, float, double>{}), [](auto td) {
         *   std::cout << get_name(td) << '\n';
         * });
         * \endcode
         */
        template <typename F, typename... Ts>
        constexpr void for_each(type_list<Ts...> list, F&& f)
        {
            map_to_tuple(list, [&](auto&& val, size_t idx)
            {
                detail::invoke_optional_index(f, std::forward<decltype(val)>(val), idx, 0);
                return 0;
            });
        }

        /*
         * Returns the initial_value unchanged.
         */
        /// \private
        template <typename R, typename F, typename... Ts>
        constexpr R accumulate(type_list<>, F&&, R&& initial_value)
        {
            return std::forward<R>(initial_value);
        }

        /*
        * Applies an accumulation function F to each type in the type_list.
        * Note: Breaking changes introduced in v0.7.0:
        *   Behaviour changed to imitate std::accumulate.
        *   F can now no longer take a second index argument.
        */
        template <typename R, typename F, typename T, typename... Ts>
        constexpr auto accumulate(type_list<T, Ts...>, F&& f, R&& initial_value)
        {
            static_assert(std::is_trivial_v<T>, "Argument is a non-trivial type!");

            return accumulate(type_list<Ts...> {},
                std::forward<F>(f),
                std::forward<std::invoke_result_t<F&&, R&&, T&&>>(
                    f(std::forward<R>(initial_value), T {})));
        }

        /**
         * Counts the number of times the predicate F returns true.
        * Note: Breaking changes introduced in v0.7.0:
        *   F can now no longer take a second index argument.
         */
        template <typename F, typename... Ts>
        constexpr size_t count_if(type_list<Ts...> list, F&& f)
        {
            return accumulate<size_t>(list,
                [&](size_t acc, const auto& t) -> size_t { return acc + (f(t) ? 1 : 0); },
                0);
        }

        namespace detail
        {
            template <typename F, typename... Carry>
            constexpr auto filter(const F&, type_list<>, type_list<Carry...> carry)
            {
                return carry;
            }

            template <typename F, typename T, typename... Ts, typename... Carry>
            constexpr auto filter(F f, type_list<T, Ts...>, type_list<Carry...>)
            {
                static_assert(std::is_trivial_v<T>, "Argument is a non-trivial type!");
                if constexpr (f(T{})) {
                    return filter(f, type_list<Ts...>{}, type_list<Carry..., T>{});
                }
                else {
                    return filter(f, type_list<Ts...>{}, type_list<Carry...>{});
                }
            }
        }

        /**
         * Filters the list according to a *constexpr* predicate.
         * Calling f(Ts{})... should be valid in a constexpr context.
         *
         * \code{.cpp}
         * filter(reflect_types(type_list<int, long, float>{}), [](auto td) {
         *   return std::is_integral_v<typename decltype(td)::type>;
         * })
         *   -> type_list<type_descriptor<int>, type_descriptor<long>>
         * \endcode
         */
        template <typename F, typename... Ts>
        constexpr auto filter(type_list<Ts...> list, F&& f)
        {
            return decltype(detail::filter(std::forward<F>(f), list, type_list<>{}))();
        }

        /**
         * Returns the first instance that matches the *constexpr* predicate.
         * Calling f(Ts{})... should be valid in a constexpr context.
         */
        template <typename F, typename... Ts>
        constexpr auto find_first(type_list<Ts...> list, F&& f)
        {
            using result_list = decltype(detail::filter(std::forward<F>(f), list, type_list<>{}));
            static_assert(result_list::size != 0, "find_first did not match anything!");
            return trait::get_t<0, result_list>{};
        }

        /**
         * Returns the only instance that matches the *constexpr* predicate.
         * If there is no match or multiple matches, fails with static_assert.
         * Calling f(Ts{})... should be valid in a constexpr context.
         */
        template <typename F, typename... Ts>
        constexpr auto find_one(type_list<Ts...> list, F&& f)
        {
            using result_list = decltype(detail::filter(std::forward<F>(f), list, type_list<>{}));
            static_assert(result_list::size != 0, "find_one did not match anything!");
            static_assert(result_list::size == 1, "Cannot resolve multiple matches in find_one!");
            return trait::get_t<0, result_list>{};
        }

        /**
         * Returns true if any item in the list matches the predicate.
         * Calling f(Ts{})... should be valid in a constexpr context.
         */
        template <typename F, typename T, typename... Ts>
        constexpr bool contains(type_list<T, Ts...> list, F&& f)
        {
            using result_list = decltype(detail::filter(std::forward<F>(f), list, type_list<>{}));
            return result_list::size > 0;
        }

        /**
         * Returns true if the type_list contains the specified type.
         * @see refl::trait::contains
         */
        template <typename T, typename... Ts>
        constexpr bool contains(type_list<Ts...>)
        {
            return trait::contains_v<T, type_list<Ts...>>;
        }

        /**
         * Returns true if the tuple contains the specified type or a supertype.
         * @see refl::trait::contains_base
         */
        template <typename T, typename... Ts>
        constexpr bool contains_base(const std::tuple<Ts...>&)
        {
            return trait::contains_base_v<T, type_list<Ts...>>;
        }

        /**
         * Returns true if the tuple contains an instance of the specified type.
         * @see refl::trait::contains_instance
         */
        template <template <typename...> typename T, typename... Ts>
        constexpr bool contains_instance(const std::tuple<Ts...>&)
        {
            return trait::contains_instance_v<T, type_list<Ts...>>;
        }

        /**
         * Applies a function to the elements of the type_list.
         *
         * \code{.cpp}
         * apply(reflect_types(type_list<int, long, float>{}), [](auto td_int, auto td_long, auto td_float) {
         *   return get_name(td_int) + " " +get_name(td_long) + " " + get_name(td_float);
         * })
         *   -> "int long float"
         * \endcode
         */
        template <typename... Ts, typename F>
        constexpr auto apply(type_list<Ts...>, F&& f)
        {
            return f(Ts{}...);
        }

        /** A synonym for std::get<N>(tuple). */
        template <size_t N, typename... Ts>
        constexpr auto& get(std::tuple<Ts...>& ts) noexcept
        {
            return std::get<N>(ts);
        }

        /** A synonym for std::get<N>(tuple). */
        template <size_t N, typename... Ts>
        constexpr const auto& get(const std::tuple<Ts...>& ts) noexcept
        {
            return std::get<N>(ts);
        }

        /** A synonym for std::get<T>(tuple). */
        template <typename T, typename... Ts>
        constexpr T& get(std::tuple<Ts...>& ts) noexcept
        {
            return std::get<T>(ts);
        }

        /** A synonym for std::get<T>(tuple). */
        template <typename T, typename... Ts>
        constexpr const T& get(const std::tuple<Ts...>& ts) noexcept
        {
            return std::get<T>(ts);
        }

        /** Returns the value of type U, where U is a template instance of T. */
        template <template<typename...> typename T, typename... Ts>
        constexpr auto& get_instance(std::tuple<Ts...>& ts) noexcept
        {
            static_assert((... || trait::is_instance_of_v<T, Ts>), "The tuple does not contain a type that is a template instance of T!");
            constexpr size_t idx = static_cast<size_t>(trait::index_of_instance_v<T, type_list<Ts...>>);
            return std::get<idx>(ts);
        }

        /** Returns the value of type U, where U is a template instance of T. */
        template <template<typename...> typename T, typename... Ts>
        constexpr const auto& get_instance(const std::tuple<Ts...>& ts) noexcept
        {
            static_assert((... || trait::is_instance_of_v<T, Ts>), "The tuple does not contain a type that is a template instance of T!");
            constexpr size_t idx = static_cast<size_t>(trait::index_of_instance_v<T, type_list<Ts...>>);
            return std::get<idx>(ts);
        }

        /**
         * Converts a type_list of types to a type_list of the type_descriptors for these types.
         *
         * \code{.cpp}
         * reflect_types(type_list<int, float>{}) -> type_list<type_descriptor<int>, type_descriptor<float>>{}
         * \endcode
         */
        template <typename... Ts>
        constexpr type_list<descriptor::type_descriptor<Ts>...> reflect_types(type_list<Ts...>) noexcept
        {
            return {};
        }

        /**
         * Converts a type_list of type_descriptors to a type_list of the target types.
         *
         * \code{.cpp}
         * unreflect_types(type_list<type_descriptor<int>, type_descriptor<float>>{}) -> type_list<int, float>{}
         * \endcode
         */
        template <typename... Ts>
        constexpr type_list<Ts...> unreflect_types(type_list<descriptor::type_descriptor<Ts>...>) noexcept
        {
            return {};
        }
    } // namespace util

    /**
     * @brief Contains the definitions of the built-in attributes
     *
     * Contains the definitions of the built-in attributes which
     * are implicitly available in macro context as well as the
     * attr::usage namespace which contains constraints
     * for user-provieded attributes.
     *
     * # Examples
     * ```
     * REFL_TYPE(Point, debug(custom_printer))
     *     REFL_FIELD(x)
     *     REFL_FIELD(y)
     * REFL_END
     * ```
     */
    namespace attr
    {
        /**
         * @brief Contains a number of constraints applicable to refl-cpp attributes.
         *
         * Contains base types which create compile-time constraints
         * that are verified by refl-cpp. These base-types must be inherited
         * by custom attribute types.
         */
        namespace usage
        {
            /**
             * Specifies that an attribute type inheriting from this type can
             * only be used with REFL_TYPE()
             */
            struct type {};

            /**
             * Specifies that an attribute type inheriting from this type can
             * only be used with REFL_FUNC()
             */
            struct function {};

            /**
             * Specifies that an attribute type inheriting from this type can
             * only be used with REFL_FIELD()
             */
            struct field {};

            /**
             * Specifies that an attribute type inheriting from this type can
             * only be used with REFL_FUNC or REFL_FIELD.
             */
            struct member : public function, public field{};

            /**
             * Specifies that an attribute type inheriting from this type can
             * only be used with any one of REFL_TYPE, REFL_FIELD, REFL_FUNC.
             */
            struct any : public member, public type {};
        }

        /**
         * Used to decorate a function that serves as a property.
         * Takes an optional friendly name.
         */
        struct property : public usage::function
        {
            const std::optional<const char*> friendly_name;

            constexpr property() noexcept
                : friendly_name{}
            {
            }

            constexpr property(const char* friendly_name) noexcept
                : friendly_name(friendly_name)
            {
            }
        };

        /**
         * Used to specify how a type should be displayed in debugging contexts.
         */
        template <typename F>
        struct debug : public usage::any
        {
            const F write;

            constexpr debug(F write)
                : write(write)
            {
            }
        };

        /**
         * Used to specify the base types of the target type.
         */
        template <typename... Ts>
        struct base_types : usage::type
        {
            /** An alias for a type_list of the base types. */
            typedef type_list<Ts...> list_type;

            /** An instance of a type_list of the base types. */
            static constexpr list_type list{ };
        };

        /**
         * Used to specify the base types of the target type.
         */
        template <typename... Ts>
        [[maybe_unused]] static constexpr base_types<Ts...> bases{ };
    } // namespace attr


    namespace detail
    {
        namespace macro_exports
        {
            using attr::property;
            using attr::debug;
            using attr::bases;
        }
    }

    namespace trait
    {
        namespace detail
        {
            template <typename T>
            auto member_type_test(int) -> decltype(typename T::member_type{}, std::true_type{});

            template <typename T>
            std::false_type member_type_test(...);
        }

        /**
         * A trait for detecting whether the type 'T' is a member descriptor.
         */
        template <typename T>
        struct is_member : decltype(detail::member_type_test<T>(0))
        {
        };

        /**
         * A trait for detecting whether the type 'T' is a member descriptor.
         */
        template <typename T>
        [[maybe_unused]] static constexpr bool is_member_v{ is_member<T>::value };

        namespace detail
        {
            template <typename T>
            struct is_field_2 : std::is_base_of<typename T::member_type, member::field>
            {
            };
        }

        /**
         * A trait for detecting whether the type 'T' is a field descriptor.
         */
        template <typename T>
        struct is_field : std::conjunction<is_member<T>, detail::is_field_2<T>>
        {
        };

        /**
         * A trait for detecting whether the type 'T' is a field descriptor.
         */
        template <typename T>
        [[maybe_unused]] static constexpr bool is_field_v{ is_field<T>::value };

        namespace detail
        {
            template <typename T>
            struct is_function_2 : std::is_base_of<typename T::member_type, member::function>
            {
            };
        }

        /**
         * A trait for detecting whether the type 'T' is a function descriptor.
         */
        template <typename T>
        struct is_function : std::conjunction<is_member<T>, detail::is_function_2<T>>
        {
        };

        /**
         * A trait for detecting whether the type 'T' is a function descriptor.
         */
        template <typename T>
        [[maybe_unused]] static constexpr bool is_function_v{ is_function<T>::value };

        /**
         * Detects whether the type T is a type_descriptor.
         * Inherits from std::bool_constant<>.
         */
        template <typename T>
        struct is_type : is_instance_of<descriptor::type_descriptor, T>
        {
        };

        /**
         * Detects whether the type T is a type_descriptor.
         * @see is_type
         */
        template <typename T>
        [[maybe_unused]] constexpr bool is_type_v{ is_type<T>::value };

        /**
         * A trait for detecting whether the type 'T' is a refl-cpp descriptor.
         */
        template <typename T>
        struct is_descriptor : std::disjunction<is_type<T>, is_member<T>>
        {
        };

        /**
         * A trait for detecting whether the type 'T' is a refl-cpp descriptor.
         */
        template <typename T>
        [[maybe_unused]] static constexpr bool is_descriptor_v{ is_descriptor<T>::value };


        /** Checks whether T is marked as a property. */
        template <typename T>
        struct is_property : std::bool_constant<
            trait::is_function_v<T> && trait::contains_v<attr::property, typename T::attribute_types>>
        {
        };

        /** Checks whether T is marked as a property. */
        template <typename T>
        [[maybe_unused]] static constexpr bool is_property_v{ is_property<T>::value };
    } // namespace trait

    /**
     * @brief Contains the basic reflection primitives
     * as well as functions operating on those primitives
     */
    namespace descriptor
    {
        namespace detail
        {
            template <typename Member>
            struct static_field_invoker
            {
                static constexpr auto invoke() -> decltype(*Member::pointer)
                {
                    return *Member::pointer;
                }

                template <typename U, typename M = Member, std::enable_if_t<M::is_writable, int> = 0>
                static constexpr auto invoke(U&& value) -> decltype(*Member::pointer = std::forward<U>(value))
                {
                    return *Member::pointer = std::forward<U>(value);
                }
            };

            template <typename Member>
            struct instance_field_invoker
            {
                template <typename T>
                static constexpr auto invoke(T&& target) -> decltype(target.*(Member::pointer))
                {
                    return target.*(Member::pointer);
                }

                template <typename T, typename U, typename M = Member, std::enable_if_t<M::is_writable, int> = 0>
                static constexpr auto invoke(T&& target, U&& value) -> decltype(target.*(Member::pointer) = std::forward<U>(value))
                {
                    return target.*(Member::pointer) = std::forward<U>(value);
                }
            };

            template <typename Member>
            static_field_invoker<Member> field_type_switch(std::true_type);

            template <typename Member>
            instance_field_invoker<Member> field_type_switch(std::false_type);

            template <typename Member>
            constexpr decltype(nullptr) get_function_pointer(...)
            {
                return nullptr;
            }

            template <typename Member>
            constexpr auto get_function_pointer(int) -> decltype(Member::pointer())
            {
                return Member::pointer();
            }

            template <typename Member, typename Pointer>
            constexpr decltype(nullptr) resolve_function_pointer(...)
            {
                return nullptr;
            }

            template <typename Member, typename Pointer>
            constexpr auto resolve_function_pointer(int) -> decltype(Member::template resolve<Pointer>())
            {
                return Member::template resolve<Pointer>();
            }

            template <typename T, size_t N>
            using make_descriptor = std::conditional_t<refl::trait::is_field_v<refl::detail::member_info<T, N>>,
                field_descriptor<T, N>,
                std::conditional_t<refl::trait::is_function_v<refl::detail::member_info<T, N>>,
                    function_descriptor<T, N>,
                    void
                >>;

            template <typename T>
            type_list<> enumerate_members(std::index_sequence<>);

            template <typename T, size_t... Idx>
            type_list<make_descriptor<T, Idx>...> enumerate_members(std::index_sequence<Idx...>);

            template <typename T>
            struct declared_member_list
            {
                static_assert(refl::trait::is_reflectable_v<T>, "This type does not support reflection!");
                using type = decltype(enumerate_members<T>(std::make_index_sequence<refl::detail::type_info<T>::member_count>{}));
            };

            template <typename T>
            using attribute_types = trait::as_type_list_t<std::remove_cv_t<decltype(refl::detail::type_info<T>::attributes)>>;

            template <typename>
            struct flatten;

            template <typename... TypeLists>
            struct flatten<type_list<TypeLists...>> : trait::concat<TypeLists...>
            {
            };

            template <typename T, typename Base>
            static constexpr void validate_base()
            {
                static_assert(std::is_base_of_v<Base, T>, "Base is not a base type of T!");
            }

            template <typename T, typename... Bases>
            static constexpr void validate_bases(type_list<Bases...>)
            {
                util::ignore((validate_base<T, Bases>(), 0)...);
            }

            template <typename T>
            static constexpr auto get_declared_base_type_list()
            {
                if constexpr (trait::contains_instance_v<attr::base_types, attribute_types<T>>) {
                    using base_types_type = trait::remove_qualifiers_t<decltype(util::get_instance<attr::base_types>(refl::detail::type_info<T>::attributes))>;
                    validate_bases<T>(base_types_type::list);
                    return typename base_types_type::list_type{};
                }
                else {
                    return type_list<>{};
                }
            }

            template <typename T>
            struct declared_base_type_list
            {
                using type = decltype(get_declared_base_type_list<T>());
            };

            template <typename T>
            struct base_type_list;

            template <typename T>
            static constexpr auto get_base_type_list()
            {
                if constexpr (trait::contains_instance_v<attr::base_types, attribute_types<T>>) {
                    using declared_bases = typename declared_base_type_list<T>::type;
                    using rec_bases = typename flatten<trait::map_t<base_type_list, declared_bases>>::type;
                    return trait::unique_t<trait::concat_t<declared_bases, rec_bases>>{};
                }
                else {
                    return type_list<>{};
                }
            }

            template <typename T>
            struct base_type_list
            {
                using type = decltype(get_base_type_list<T>());
            };

            template <typename T>
            struct member_list : flatten<trait::map_t<declared_member_list, trait::prepend_t<T, typename base_type_list<T>::type>>>
            {
            };

        } // namespace detail

        /** A type_list of the declared member descriptors of the target type T. */
        template <typename T>
        using declared_member_list = typename detail::declared_member_list<T>::type;

        /** A type_list of the declared and inherited member descriptors of the target type T. */
        template <typename T>
        using member_list = typename detail::member_list<T>::type;

        /**
         * @brief The base type for member descriptors.
         */
        template <typename T, size_t N>
        class member_descriptor_base
        {
        protected:

            typedef refl::detail::member_info<T, N> member;

        public:

            /**
             * An alias for the declaring type of the reflected member.
             *
             * \code{.cpp}
             * struct Foo { const int* x; };
             * REFL_AUTO(type(Foo), field(x))
             *
             * get_t<0, member_list<Foo>>::declaring_type -> Foo
             * \endcode
             */
            typedef T declaring_type;

            /** An alias specifying the member type of member. */
            typedef typename member::member_type member_type;

            /**
             * An alias specifying the types of the attributes of the member. (Removes CV-qualifiers.)
             * \copydetails refl::descriptor::get_attribute_types
             */
            typedef trait::as_type_list_t<std::remove_cv_t<decltype(member::attributes)>> attribute_types;

            /**
             * The type_descriptor of the declaring type.
             * \copydetails refl::descriptor::get_declarator
             */
            static constexpr type_descriptor<T> declarator{ };

            /**
             * The name of the reflected member.
             * \copydetails refl::descriptor::get_name
             */
            static constexpr auto name{ member::name };

            /**
             * The attributes of the reflected member.
             * \copydetails refl::descriptor::get_attributes
             */
            static constexpr auto attributes{ member::attributes };

        };

        /**
         * @brief Represents a reflected field.
         */
        template <typename T, size_t N>
        class field_descriptor : public member_descriptor_base<T, N>
        {
            using typename member_descriptor_base<T, N>::member;
            static_assert(trait::is_field_v<member>);

        public:

            /**
             * Type value type of the member.
             *
             * \code{.cpp}
             * struct Foo { const int* x; };
             * REFL_AUTO(type(Foo), field(x))
             *
             * get_t<0, member_list<Foo>>::value_type -> const int*
             * \endcode
             */
            typedef typename member::value_type value_type;

            /**
             * Whether the field is static or not.
             * \copydetails refl::descriptor::is_static
             */
            static constexpr bool is_static{ !std::is_member_object_pointer_v<decltype(member::pointer)> };

            /**
             * Whether the field is const or not.
             * @see refl::descriptor::is_const
             */
            static constexpr bool is_writable{ !std::is_const_v<value_type> };

            /**
             * A member pointer to the reflected field of the appropriate type.
             * \copydetails refl::descriptor::get_pointer
             */
            static constexpr auto pointer{ member::pointer };

        private:

            using invoker = decltype(detail::field_type_switch<field_descriptor>(std::bool_constant<is_static>{}));

        public:

            /**
             * Returns the value of the field. (for static fields).
             * \copydetails refl::descriptor::invoke
             */
            template <decltype(nullptr) = nullptr>
            static constexpr decltype(auto) get() noexcept
            {
                return *member::pointer;
            }

            /**
             * Returns the value of the field. (for instance fields).
             * \copydetails refl::descriptor::invoke
             */
            template <typename U>
            static constexpr decltype(auto) get(U&& target) noexcept
            {
                return target.*(member::pointer);
            }

            /**
             * A synonym for get().
             * \copydetails refl::descriptor::invoke
             */
            template <typename... Args>
            constexpr auto operator()(Args&&... args) const noexcept -> decltype(invoker::invoke(std::forward<Args>(args)...))
            {
                return invoker::invoke(std::forward<Args>(args)...);
            }

        };

        /**
         * @brief Represents a reflected function.
         */
        template <typename T, size_t N>
        class function_descriptor : public member_descriptor_base<T, N>
        {
            using typename member_descriptor_base<T, N>::member;
            static_assert(trait::is_function_v<member>);

        public:

            /**
             * Invokes the function with the given arguments.
             * If the function is an instance function, a reference
             * to the instance is provided as first argument.
             * \copydetails refl::descriptor::invoke
             */
            template <typename... Args>
            static constexpr auto invoke(Args&&... args) -> decltype(member::invoke(std::declval<Args>()...))
            {
                return member::invoke(std::forward<Args>(args)...);
            }

            /**
             * The return type of an invocation of this member with Args... (as if by invoke(...)).
             * \copydetails refl::descriptor::return_type
             */
            template <typename... Args>
            using return_type = decltype(member::invoke(std::declval<Args>()...));

            /**
             * A synonym for invoke(args...).
             * \copydetails refl::descriptor::invoke
             */
            template <typename... Args>
            constexpr auto operator()(Args&&... args) const -> decltype(invoke(std::declval<Args>()...))
            {
                return invoke(std::forward<Args>(args)...);
            }

            /**
             * Returns a pointer to a non-overloaded function.
             * \copydetails refl::descriptor::get_pointer
             */
            static constexpr auto pointer{ detail::get_function_pointer<member>(0) };

            /**
             * Whether the pointer member was correctly resolved to a concrete implementation.
             * If this field is false, resolve() would need to be called instead.
             * \copydetails refl::descriptor::is_resolved
             */
            static constexpr bool is_resolved{ !std::is_same_v<decltype(pointer), const decltype(nullptr)> };

            /**
             * Whether the pointer can be resolved as with the specified type.
             * \copydetails refl::descriptor::can_resolve
             */
            template <typename Pointer>
            static constexpr bool can_resolve()
            {
                return !std::is_same_v<decltype(resolve<Pointer>()), decltype(nullptr)>;
            }

            /**
             * Resolves the function pointer as being of type Pointer.
             * Required when taking a pointer to an overloaded function.
             *
             * \copydetails refl::descriptor::resolve
             */
            template <typename Pointer>
            static constexpr auto resolve()
            {
                return detail::resolve_function_pointer<member, Pointer>(0);
            }

        };

        /** Represents a reflected type. */
        template <typename T>
        class type_descriptor
        {
        private:

            static_assert(refl::trait::is_reflectable_v<T>, "This type does not support reflection!");

            typedef refl::detail::type_info<T> type_info;

        public:

            /**
             * The reflected type T.
             *
             * \code{.cpp}
             * struct Foo {};
             * REFL_AUTO(type(Foo))
             *
             * type_descriptor<Foo>::type -> Foo
             * \endcode
             */
            typedef T type;

            /**
             * The declared base types (via base_types<Ts...> attribute) of T.
             * \copydetails refl::descriptor::get_declared_base_types
             */
            typedef typename detail::declared_base_type_list<T>::type declared_base_types;

            /**
             * The declared and inherited base types of T.
             * \copydetails refl::descriptor::get_base_types
             */
            typedef typename detail::base_type_list<T>::type base_types;

            /**
             * A synonym for declared_member_list<T>.
             * \copydetails refl::descriptor::declared_member_list
             */
            typedef declared_member_list<T> declared_member_types;

            /**
             * A synonym for member_list<T>.
             * \copydetails refl::descriptor::member_list
             */
            typedef member_list<T> member_types;

            /**
             * An alias specifying the types of the attributes of the member. (Removes CV-qualifiers.)
             * \copydetails refl::descriptor::get_attribute_types
             */
            typedef detail::attribute_types<T> attribute_types;

            /**
             * The declared base types (via base_types<Ts...> attribute) of T.
             * \copydetails refl::descriptor::get_declared_base_types
             */
            static constexpr declared_base_types declared_bases{};

            /**
             * The declared  and inherited base types of T.
             * \copydetails refl::descriptor::get_base_types
             */
            static constexpr base_types bases{};

            /**
             * The list of declared member descriptors.
             * \copydetails refl::descriptor::get_declared_members
             */
            static constexpr declared_member_types declared_members{  };

            /**
             * The list of declared and inherited member descriptors.
             * \copydetails refl::descriptor::get_members
             */
            static constexpr member_types members{  };

            /**
             * The name of the reflected type.
             * \copydetails refl::descriptor::get_name
             */
            static constexpr const auto name{ type_info::name };

            /**
             * The attributes of the reflected type.
             * \copydetails refl::descriptor::get_attributes
              */
            static constexpr const auto attributes{ type_info::attributes };

        };

        /**
         * Returns the full name of the descriptor
         *
         * \code{.cpp}
         * namespace ns {
         *   struct Foo {
         *     int x;
         *   };
         * }
         * REFL_AUTO(type(ns::Foo), field(x))
         *
         * get_name(reflect<Foo>()) -> "ns::Foo"
         * get_name(get_t<0, member_list<Foo>>()) -> "x"
         * \endcode
         */
        template <typename Descriptor>
        constexpr auto get_name(Descriptor d) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return d.name;
        }

        /**
         * Returns a const reference to the descriptor's attribute tuple.
         *
         * \code{.cpp}
         * struct Foo {};
         * REFL_AUTO(type(Foo, bases<>, ns::serializable()))
         *
         * get_attributes(reflect<Foo>()) -> const std::tuple<attr::base_types<>, ns::serializable>&
         * \endcode
         */
        template <typename Descriptor>
        constexpr const auto& get_attributes(Descriptor d) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return d.attributes;
        }

        /**
         * Returns a type_list of the descriptor's attribute types.
         *
         * \code{.cpp}
         * struct Foo {};
         * REFL_AUTO(type(Foo, bases<>, ns::serializable()))
         *
         * get_attribute_types(reflect<Foo>()) -> type_list<attr::base_types<>, ns::serializable>
         * \endcode
         */
        template <typename Descriptor>
        constexpr auto get_attribute_types(Descriptor d) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return trait::as_type_list_t<std::remove_cv_t<decltype(d.attributes)>>{};
        }

        /**
         * Returns a type_list of the declared base types of the type.
         * Combine with reflect_types to obtain type_descriptors for those types.
         * @see reflect_types
         *
         * \code{.cpp}
         * struct Animal {};
         * REFL_AUTO(type(Animal))
         * struct Mammal : Animal {};
         * REFL_AUTO(type(Mammal, bases<Animal>))
         * struct Dog : Mammal {}:
         * REFL_AUTO(type(Dog, bases<Mammal>))
         *
         * get_base_types(reflect<Dog>()) -> type_list<Mammal>
         * \endcode
         */
        template <typename TypeDescriptor>
        constexpr auto get_declared_base_types(TypeDescriptor t) noexcept
        {
            static_assert(trait::is_type_v<TypeDescriptor>);
            return t.declared_bases;
        }

        /**
         * Returns a type_list of the declared and inherited base types of the type.
         * Combine with reflect_types to obtain type_descriptors for those types.
         * @see reflect_types
         *
         * \code{.cpp}
         * struct Animal {};
         * REFL_AUTO(type(Animal))
         * struct Mammal : Animal {};
         * REFL_AUTO(type(Mammal, bases<Animal>))
         * struct Dog : Mammal {}:
         * REFL_AUTO(type(Dog, bases<Mammal>))
         *
         * get_base_types(reflect<Dog>()) -> type_list<Mammal, Animal>
         * \endcode
         */
        template <typename TypeDescriptor>
        constexpr auto get_base_types(TypeDescriptor t) noexcept
        {
            static_assert(trait::is_type_v<TypeDescriptor>);
            return t.bases;
        }

        /**
         * Returns a type_list of the declared members of the type.
         *
         * \code{.cpp}
         * struct Base {
         *  int val;
         * };
         * struct Foo : Base {
         *   int bar, baz;
         * };
         * REFL_AUTO(type(Foo, bases<Base>), field(bar), field(baz))
         * get_declared_members(reflect<Foo>()) -> type_list<field_descriptor<Foo, 0> /bar/, field_descriptor<Foo, 1> /baz/>
         * \endcode
         */
        template <typename TypeDescriptor>
        constexpr auto get_declared_members(TypeDescriptor t) noexcept
        {
            static_assert(trait::is_type_v<TypeDescriptor>);
            return t.declared_members;
        }

        /**
         * Returns a type_list of the declared and inherited members of the type.
         *
         * \code{.cpp}
         * struct Base {
         *  int val;
         * };
         * struct Foo : Base {
         *   int bar, baz;
         * };
         * REFL_AUTO(type(Foo, bases<Base>), field(bar), field(baz))
         * get_members(reflect<Foo>()) -> type_list<field_descriptor<Foo, 0> /bar/, field_descriptor<Foo, 1> /baz/, field_descriptor<Base, 0> /val/>
         * \endcode
         */
        template <typename TypeDescriptor>
        constexpr auto get_members(TypeDescriptor t) noexcept
        {
            static_assert(trait::is_type_v<TypeDescriptor>);
            return t.members;
        }

        /**
         * Returns the type_descriptor of declaring type of the member.
         *
         * \code{.cpp}
         * struct Foo {
         *   int bar;
         * };
         * REFL_AUTO(type(Foo), field(bar)
         * get_declarator(get_t<0, member_list<Foo>>()) -> type_descriptor<Foo>{}
         * \endcode
         */
        template <typename MemberDescriptor>
        constexpr auto get_declarator(MemberDescriptor d) noexcept
        {
            static_assert(trait::is_member_v<MemberDescriptor>);
            return d.declarator;
        }

        /**
         * Returns a pointer to the reflected field/function.
         * When the member is a function, the return value might be nullptr
         * if the type of the function pointer cannot be resolved.
         * @see is_resolved
         * @see can_resolve
         * @see resolve
         *
         * \code{.cpp}
         * struct Foo {
         *   int bar;
         *   static int baz;
         * };
         * REFL_AUTO(type(Foo), field(bar), field(baz))
         * get_pointer(get_t<0, member_list<Foo>>()) -> (int Foo::*) &Foo::bar
         * get_pointer(get_t<1, member_list<Foo>>()) -> (int*) &Foo::baz
         * \endcode
         */
        template <typename MemberDescriptor>
        constexpr auto get_pointer(MemberDescriptor d) noexcept
        {
            static_assert(trait::is_member_v<MemberDescriptor>);
            return d.pointer;
        }

        /**
         * Invokes the member with the specified arguments.
         *
         * \code{.cpp}
         * struct Foo {
         *   int bar = 1;
         *   static int baz = 5;
         *   void foobar(int x) { return x * 2; }
         *   static void foobaz(int x) { return x * 3; }
         * };
         * REFL_AUTO(type(Foo), field(bar), field(baz), func(foobar), func(foobaz))
         * invoke(get_t<0, member_list<Foo>(), Foo()) -> 1 (Foo().bar)
         * invoke(get_t<1, member_list<Foo>>()) -> 5 (Foo::baz)
         * invoke(get_t<2, member_list<Foo>(), Foo(), 10) -> 20 (Foo().foobar())
         * invoke(get_t<3, member_list<Foo>>()) -> 30 (Foo::foobaz())
         * \endcode
         */
        template <typename MemberDescriptor, typename... Args>
        constexpr auto invoke(MemberDescriptor d, Args&&... args) noexcept -> decltype(d(std::forward<Args>(args)...))
        {
            return d(std::forward<Args>(args)...);
        }

        /**
         * Checks whether the field is declared as static.
         *
         * \code{.cpp}
         * struct Foo {
         *   int bar;
         *   static int baz;
         * };
         * REFL_AUTO(type(Foo), field(bar), field(baz))
         * is_static(get_t<0, member_list<Foo>>()) -> false
         * is_static(get_t<1, member_list<Foo>>()) -> true
         * \endcode
         */
        template <typename FieldDescriptor>
        constexpr auto is_static(FieldDescriptor d) noexcept
        {
            static_assert(trait::is_field_v<FieldDescriptor>);
            return d.is_static;
        }

        /**
         * Checks whether the value type of the field is const-qualified.
         *
         * \code{.cpp}
         * struct Foo {
         *   int bar;
         *   const int baz;
         * };
         * REFL_AUTO(type(Foo), field(bar), field(baz))
         * is_const(get_t<0, member_list<Foo>>()) -> false
         * is_const(get_t<1, member_list<Foo>>()) -> true
         * \endcode
         */
        template <typename FieldDescriptor>
        constexpr auto is_const(FieldDescriptor d) noexcept
        {
            static_assert(trait::is_field_v<FieldDescriptor>);
            return d.is_const;
        }

        /**
         * The return type when invoking the specified descriptor using the provided argument types.
         * Argument coversion will be applied as per C++ rules.
         */
        template <typename FunctionDescriptor, typename... Args>
        using result_type = typename FunctionDescriptor::template result_type<Args...>;

        /**
         * Checks whether the function pointer was automatically resolved.
         *
         * \code{.cpp}
         * struct Foo {
         *   void bar();
         *   void bar(int);
         *   void baz();
         * };
         * REFL_AUTO(type(Foo), func(bar), func(baz))
         * is_resolved(get_t<0, member_list<Foo>>()) -> false
         * is_resolved(get_t<1, member_list<Foo>>()) -> true
         * \endcode
         */
        template <typename FunctionDescriptor>
        constexpr auto is_resolved(FunctionDescriptor d) noexcept
        {
            static_assert(trait::is_function_v<FunctionDescriptor>);
            return d.is_resolved;
        }

        /**
         * Checks whether the function pointer can be resolved as
         * a pointer of the specified type.
         *
         * \code{.cpp}
         * struct Foo {
         *   void bar();
         *   void bar(int);
         * };
         * REFL_AUTO(type(Foo), func(bar))
         * can_resolve<void(Foo::*)()>(get_t<0, member_list<Foo>>()) -> true
         * can_resolve<void(Foo::*)(int)>(get_t<0, member_list<Foo>>()) -> true
         * can_resolve<void(Foo::*)(std::string)>(get_t<0, member_list<Foo>>()) -> false
         * \endcode
         */
        template <typename Pointer, typename FunctionDescriptor>
        constexpr auto can_resolve(FunctionDescriptor d) noexcept
        {
            static_assert(trait::is_function_v<FunctionDescriptor>);
            return d.template can_resolve<Pointer>();
        }

        /**
         * Resolves the function pointer as a pointer of the specified type.
         *
         * \code{.cpp}
         * struct Foo {
         *   void bar();
         *   void bar(int);
         * };
         * REFL_AUTO(type(Foo), func(bar))
         * resolve<void(Foo::*)()>(get_t<0, member_list<Foo>>()) -> <&Foo::bar()>
         * resolve<void(Foo::*)(int)>(get_t<0, member_list<Foo>>()) -> <&Foo::bar(int)>
         * resolve<void(Foo::*)(std::string)>(get_t<0, member_list<Foo>>()) -> nullptr
         * \endcode
         */
        template <typename Pointer, typename FunctionDescriptor>
        constexpr auto resolve(FunctionDescriptor d) noexcept
        {
            static_assert(trait::is_function_v<FunctionDescriptor>);
            return d.template resolve<Pointer>();
        }

        /**
         * Checks whether T is a field descriptor.
         *
         * @see refl::descriptor::field_descriptor
         *
         * \code{.cpp}
         * REFL_AUTO(type(Foo), func(bar), field(baz))
         * is_function(get_t<0, member_list<Foo>>()) -> false
         * is_function(get_t<1, member_list<Foo>>()) -> true
         * \endcode
         */
        template <typename Descriptor>
        constexpr bool is_field(Descriptor) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return trait::is_field_v<Descriptor>;
        }

        /**
         * Checks whether T is a function descriptor.
         *
         * @see refl::descriptor::function_descriptor
         *
         * \code{.cpp}
         * REFL_AUTO(type(Foo), func(bar), field(baz))
         * is_function(get_t<0, member_list<Foo>>()) -> true
         * is_function(get_t<1, member_list<Foo>>()) -> false
         * \endcode
         */
        template <typename Descriptor>
        constexpr bool is_function(Descriptor) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return trait::is_function_v<Descriptor>;
        }

        /**
         * Checks whether T is a type descriptor.
         *
         * @see refl::descriptor::type_descriptor
         *
         * \code{.cpp}
         * REFL_AUTO(type(Foo))
         * is_type(reflect<Foo>>()) -> true
         * \endcode
         */
        template <typename Descriptor>
        constexpr bool is_type(Descriptor) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return trait::is_type_v<Descriptor>;
        }

        /**
         * Checks whether T has an attribute of type A.
         *
         * \code{.cpp}
         * REFL_AUTO(type(User), func(get_name, property()), func(set_name, property()))
         * has_attribute<attr::property>(get_t<0, member_list<User>>{}) -> true
         * \endcode
         */
        template <typename A, typename Descriptor>
        constexpr bool has_attribute(Descriptor) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return trait::contains_base_v<A, typename Descriptor::attribute_types>;
        }

        /**
         * Checks whether T has an attribute of that is a template instance of A.
         *
         * \code{.cpp}
         * REFL_AUTO(type(Random, debug{ [](auto os, auto){ os << "[Random]"; } }))
         * has_attribute<attr::debug>(reflect<Random>()) -> true
         * \endcode
         */
        template <template<typename...> typename A, typename Descriptor>
        constexpr bool has_attribute(Descriptor) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return trait::contains_instance_v<A, typename Descriptor::attribute_types>;
        }

        /**
         * Returns the value of the attribute A on T.
         *
         * \code{.cpp}
         * REFL_AUTO(type(User), func(get_name, property()), func(set_name, property()))
         * get_attribute<attr::property>(get_t<0, member_list<User>>{}) -> property{ friendly_name = nullopt }
         * \endcode
         */
        template <typename A, typename Descriptor>
        constexpr const A& get_attribute(Descriptor d) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return util::get<A>(d.attributes);
        }

        /**
         * Returns the value of the attribute A on T.
         *
         * \code{.cpp}
         * REFL_AUTO(type(Random, debug{ [](auto os, auto){ os << "[Random]"; } }))
         * get_attribute<attr::debug>(reflect<Random>()) -> instance of debug<LambdaType>
         * \endcode
         */
        template <template<typename...> typename A, typename Descriptor>
        constexpr const auto& get_attribute(Descriptor d) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return util::get_instance<A>(d.attributes);
        }

        /**
         * Checks whether T is a member descriptor marked with the property attribute.
         *
         * @see refl::attr::property
         * @see refl::descriptor::get_property
         *
         * \code{.cpp}
         * REFL_AUTO(type(User), func(get_name, property("user_name")), func(set_name, property()))
         * is_property(get_t<0, member_list<User>>{}) -> true
         * \endcode
         */
        template <typename MemberDescriptor>
        constexpr bool is_property(MemberDescriptor d) noexcept
        {
            static_assert(trait::is_member_v<MemberDescriptor>);
            return has_attribute<attr::property>(d);
        }

        /**
         * Gets the property attribute.
         *
         * @see refl::attr::property
         * @see refl::descriptor::is_property
         *
         * \code{.cpp}
         * REFL_AUTO(type(User), func(get_name, property("user_name")), func(set_name, property()))
         * *get_property(get_t<0, member_list<User>>{}).friendly_name -> "user_name"
         * \endcode
         */
        template <typename FunctionDescriptor>
        constexpr attr::property get_property(FunctionDescriptor d) noexcept
        {
            static_assert(trait::is_function_v<FunctionDescriptor>);
            return get_attribute<attr::property>(d);
        }

        namespace detail
        {
            struct placeholder
            {
                template <typename T>
                operator T() const;
            };
        } // namespace detail

        /**
         * Checks if T is a 0-arg const-qualified member function with a property attribute or a field.
         *
         * \code{.cpp}
         * REFL_AUTO(type(User), func(get_name, property()), func(set_name, property()))
         * is_readable(get_t<0, member_list<User>>{}) -> true
         * is_readable(get_t<1, member_list<User>>{}) -> false
         * \endcode
         */
        template <typename MemberDescriptor>
        constexpr bool is_readable(MemberDescriptor) noexcept
        {
            static_assert(trait::is_member_v<MemberDescriptor>);
            if constexpr (trait::is_property_v<MemberDescriptor>) {
                if constexpr (std::is_invocable_v<MemberDescriptor, const typename MemberDescriptor::declaring_type&>) {
                    using return_type = typename MemberDescriptor::template return_type<const typename MemberDescriptor::declaring_type&>;
                    return !std::is_void_v<return_type>;
                }
                else {
                    return false;
                }
            }
            else {
                return trait::is_field_v<MemberDescriptor>;
            }
        }

        /**
         * Checks if T is a 1-arg non-const-qualified member function with a property attribute or a non-const field.
         *
         * \code{.cpp}
         * struct User { std::string get_name() const; }
         * REFL_AUTO(type(User), func(get_name, property()), func(set_name, property()))
         * is_writable(get_t<0, member_list<User>>{}) -> false
         * is_writable(get_t<1, member_list<User>>{}) -> true
         * \endcode
         */
        template <typename MemberDescriptor>
        constexpr bool is_writable(MemberDescriptor) noexcept
        {
            static_assert(trait::is_member_v<MemberDescriptor>);
            if constexpr (trait::is_property_v<MemberDescriptor>) {
                return std::is_invocable_v<MemberDescriptor, typename MemberDescriptor::declaring_type&, detail::placeholder>;
            }
            else if constexpr (trait::is_field_v<MemberDescriptor>) {
                return !std::is_const_v<typename trait::remove_qualifiers_t<MemberDescriptor>::value_type>;
            }
            else {
                return false;
            }
        }

        namespace detail
        {
            template <typename T>
            struct get_type_descriptor
            {
                typedef type_descriptor<T> type;
            };
        } // namespace detail

        /**
         * Checks if a type has a bases attribute.
         *
         * @deprecated Use has_base_types in combination with reflect_types instead.
         * @see refl::attr::bases
         * @see refl::descriptor::get_bases
         *
         * \code{.cpp}
         * REFL_AUTO(type(Dog, bases<Animal>))
         * has_bases(reflect<Dog>()) -> true
         * \endcode
         */
        template <typename TypeDescriptor>
        [[deprecated]] constexpr auto has_bases(TypeDescriptor t) noexcept
        {
            static_assert(trait::is_type_v<TypeDescriptor>);
            return has_attribute<attr::base_types>(t);
        }

        /**
         * Returns a list of the type_descriptor<T>s of the base types of the target,
         * as specified by the bases<A, B, ...> attribute.
         *
         * @deprecated Use get_base_types in combination with reflect_types instead.
         * @see refl::attr::bases
         * @see refl::descriptor::has_bases
         *
         * \code{.cpp}
         * REFL_AUTO(type(Dog, bases<Animal>))
         * get_bases(reflect<Dog>()) -> type_list<type_descriptor<Animal>>
         * \endcode
         */
        template <typename TypeDescriptor>
        [[deprecated]] constexpr auto get_bases(TypeDescriptor t) noexcept
        {
            static_assert(trait::is_type_v<TypeDescriptor>);
            static_assert(has_bases(t), "Target type does not have a bases<A, B, ...> attribute.");

            constexpr auto bases = get_attribute<attr::base_types>(t);
            using base_types = typename decltype(bases)::list_type;
            return trait::map_t<detail::get_type_descriptor, base_types>{};
        }

        /**
         * Returns the unqualified name of the type, discarding the namespace and typenames (if a template type).
         *
         * \code{.cpp}
         * get_simple_name(reflect<std::vector<float>>()) -> "vector"
         * \endcode
         */
        template <typename TypeDescriptor>
        constexpr auto get_simple_name(TypeDescriptor t)
        {
            static_assert(trait::is_type_v<TypeDescriptor>);
            constexpr size_t template_start = t.name.find('<');
            constexpr size_t scope_last = t.name.rfind(':', template_start);
            if constexpr (scope_last == const_string<0>::npos) {
                return t.name;
            }
            else {
                return t.name.template substr<scope_last + 1, template_start - scope_last - 1>();
            }
        }

        /**
         * Returns the debug name of T (In the form of 'declaring_type::member_name') as a const_string.
         *
         * \code{.cpp}
         * REFL_AUTO(type(Point), field(x), field(y))
         * get_debug_name_const(get_t<0, member_list<Point>>{}) -> "Point::x"
         * \endcode
         */
        template <typename MemberDescriptor>
        constexpr auto get_debug_name_const(MemberDescriptor d)
        {
            static_assert(trait::is_member_v<MemberDescriptor>);
            return d.declarator.name + "::" + d.name;
        }

        /**
         * Returns the debug name of T. (In the form of 'declaring_type::member_name').
         * \code{.cpp}
         * REFL_AUTO(type(Point), field(x), field(y))
         * get_debug_name(get_t<0, member_list<Point>>{}) -> "Point::x"
         * \endcode
         */
        template <typename MemberDescriptor>
        const char* get_debug_name(MemberDescriptor d)
        {
            static_assert(trait::is_member_v<MemberDescriptor>);
            static const std::string name(get_debug_name_const(d).str());
            return name.c_str();
        }

        namespace detail
        {
            constexpr bool is_upper(char ch)
            {
                return ch >= 'A' && ch <= 'Z';
            }

            constexpr char to_upper(char ch)
            {
                return ch >= 'a' && ch <= 'z'
                    ? char(ch + ('A' - 'a'))
                    : ch;
            }

            constexpr char to_lower(char ch)
            {
                return ch >= 'A' && ch <= 'Z'
                    ? char(ch + ('a' - 'A'))
                    : ch;
            }

            template <typename T, bool PreferUpper>
            constexpr auto normalize_bare_accessor_name()
            {
                constexpr auto str = T::name.template substr<3>();
                if constexpr (str.data[0] == '_') {
                    return str.template substr<1>();
                }
                else if constexpr (!PreferUpper && str.data[0] >= 'A' && str.data[0] <= 'Z') {
                    return make_const_string(to_lower(str.data[0])) + str.template substr<1>();
                }
                else if constexpr (PreferUpper) {
                    return make_const_string(to_upper(str.data[0])) + str.template substr<1>();
                }
                else {
                    return str;
                }
            }

            template <typename T>
            constexpr auto normalize_accessor_name(const T)
            {
                constexpr T t{};
                if constexpr (t.name.size > 3) {
                    constexpr auto prefix = t.name.template substr<0, 3>();
                    constexpr bool cont_snake_or_camel = (t.name.size > 4 && t.name.data[3] == '_' && !is_upper(t.name.data[4])) || is_upper(t.name.data[3]);
                    constexpr bool cont_pascal = is_upper(t.name.data[3]);

                    if constexpr ((is_readable(T{}) && ((prefix == "Get" && cont_pascal) || (prefix == "get" && cont_snake_or_camel)))
                        || (is_writable(T{}) && ((prefix == "Set" && cont_pascal) || (prefix == "set" && cont_snake_or_camel)))) {
                        constexpr bool prefer_upper = is_upper(prefix.data[0]);
                        return normalize_bare_accessor_name<T, prefer_upper>();
                    }
                    else {
                        return t.name;
                    }
                }
                else {
                    return t.name;
                }
            }

            template <typename T>
            constexpr auto get_display_name(const T t) noexcept
            {
                if constexpr (trait::is_property_v<T>) {
                    if constexpr (util::get<attr::property>(t.attributes).friendly_name) {
                        return REFL_MAKE_CONST_STRING(*util::get<attr::property>(t.attributes).friendly_name);
                    }
                    else {
                        return detail::normalize_accessor_name(t);
                    }
                }
                else {
                    return t.name;
                }
            }
        } // namespace detail

        /**
         * Returns the display name of T.
         * Uses the friendly_name of the property attribute, or the normalized name if no friendly_name was provided.
         *
         * \code{.cpp}
         * struct Foo {
         *   int get_foo() const;
         *   int GetFoo() const;
         *   int get_non_const() /missing const/;
         *   int get_custom() const;
         * };
         * REFL_AUTO(
         *   type(Foo),
         *   func(get_foo, property()),
         *   func(GetFoo, property()),
         *   func(get_non_const, property()),
         *   func(get_custom, property("value")),
         * )
         *
         * get_display_name(get_t<0, member_list<Foo>>{}) -> "foo"
         * get_display_name(get_t<1, member_list<Foo>>{}) -> "Foo"
         * get_display_name(get_t<2, member_list<Foo>>{}) -> "get_non_const"
         * get_display_name(get_t<3, member_list<Foo>>{}) -> "value"
         * \endcode
         */
        template <typename Descriptor>
        const char* get_display_name(Descriptor d) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            static const std::string name(detail::get_display_name(d));
            return name.c_str();
        }

        /**
         * Returns the display name of T as a const_string<N>.
         * Uses the friendly_name of the property attribute, or the normalized name if no friendly_name was provided.
         * @see get_display_name
         */
        template <typename Descriptor>
        constexpr auto get_display_name_const(Descriptor d) noexcept
        {
            static_assert(trait::is_descriptor_v<Descriptor>);
            return detail::get_display_name(d);
        }

        /**
         * Checks if there exists a member that has the same display name as the one provied and is writable.
         * For getter methods with a property attribute, the return value will be true if there exists a
         * reflected setter method with a property with the same display name (property name normalization applies automatically).
         * For fields, returns true only if the field is writable.
         */
        template <typename ReadableMember>
        constexpr auto has_writer(ReadableMember member)
        {
            static_assert(is_writable(member) || is_property(member));
            if constexpr (is_writable(member)) {
                return true;
            }
            else {
                return contains(get_declarator(member).members, [](auto m) {
                    return is_property(m) && is_writable(m) && get_display_name_const(m) == get_display_name_const(ReadableMember{});
                });
            }
        }

        /**
         * Checks if there exists a member that has the same display name as the one provied and is readable.
         * For setter methods with a property attribute, the return value will be true if there exists a
         * reflected getter method with a property with the same display name (property name normalization applies automatically).
         * For fields, returns true.
         */
        template <typename WritableMember>
        constexpr auto has_reader(WritableMember member)
        {
            static_assert(is_readable(member) || is_property(member));
            if constexpr (is_readable(member)) {
                return true;
            }
            else {
                return contains(get_declarator(member).members, [](auto m) {
                    return is_property(m) && is_readable(m) && get_display_name_const(m) == get_display_name_const(WritableMember{});
                });
            }
        }

        /**
         * Returns a member that has the same display name as the one provied and is writable.
         * For getter methods with a property attribute, the return value will the
         * reflected setter method with a property with the same display name (property name normalization applies automatically).
         * For fields, returns the same descriptor if writable.
         */
        template <typename ReadableMember>
        constexpr auto get_writer(ReadableMember member)
        {
            static_assert(is_writable(member) || is_property(member));
            if constexpr (is_writable(member)) {
                return member;
            }
            else {
                static_assert(has_writer(member));
                return find_one(get_declarator(member).members, [](auto m) {
                    return is_property(m) && is_writable(m) && get_display_name_const(m) == get_display_name_const(ReadableMember{});
                });
            }
        }

        /**
         * Returns a member that has the same display name as the one provied and is readable.
         * For setter methods with a property attribute, the return value will be a
         * reflected getter method with a property with the same display name (property name normalization applies automatically).
         * For fields, returns the same descriptor.
         */
        template <typename WritableMember>
        constexpr auto get_reader(WritableMember member)
        {
            static_assert(is_readable(member) || is_property(member));
            if constexpr (is_readable(member)) {
                return member;
            }
            else {
                static_assert(has_reader(member));
                return find_one(get_declarator(member).members, [](auto m) {
                    return is_property(m) && is_readable(m) && get_display_name_const(m) == get_display_name_const(WritableMember{});
                });
            }
        }

    } // namespace descriptor

    using descriptor::member_list;
    using descriptor::declared_member_list;
    using descriptor::field_descriptor;
    using descriptor::function_descriptor;
    using descriptor::type_descriptor;

    /** Returns true if the type T is reflectable. */
    template <typename T>
    constexpr bool is_reflectable() noexcept
    {
        return trait::is_reflectable_v<T>;
    }

    /** Returns true if the non-qualified type T is reflectable.*/
    template <typename T>
    constexpr bool is_reflectable(const T&) noexcept
    {
        return trait::is_reflectable_v<T>;
    }

    /** Returns the type descriptor for the type T. */
    template<typename T>
    constexpr type_descriptor<T> reflect() noexcept
    {
        return {};
    }

    /** Returns the type descriptor for the non-qualified type T. */
    template<typename T>
    constexpr type_descriptor<T> reflect(const T&) noexcept
    {
        return {};
    }

#ifndef REFL_DETAIL_FORCE_EBO
#ifdef _MSC_VER
#define REFL_DETAIL_FORCE_EBO __declspec(empty_bases)
#else
#define REFL_DETAIL_FORCE_EBO
#endif
#endif

    /**
     * @brief Contains utilities that can have runtime-overhead (like proxy, debug, invoke)
     */
    namespace runtime
    {
        template <typename Derived, typename Target>
        struct REFL_DETAIL_FORCE_EBO proxy;

        namespace detail
        {
            template <typename T>
            struct get_member_info;

            template <typename T, size_t N>
            struct get_member_info<refl::function_descriptor<T, N>>
            {
                using type = refl::detail::member_info<T, N>;
            };

            template <typename T, size_t N>
            struct get_member_info<refl::field_descriptor<T, N>>
            {
                using type = refl::detail::member_info<T, N>;
            };

            template <typename T, typename U>
            constexpr T& static_ref_cast(U& value) noexcept
            {
                return static_cast<T&>(value);
            }

            template <typename T, typename U>
            constexpr const T& static_ref_cast(const U& value) noexcept
            {
                return static_cast<const T&>(value);
            }

            template <typename... Results>
            constexpr type_list<Results...> get_members_skip_shadowed(type_list<>, type_list<Results...>)
            {
                return {};
            }

            template <typename Member, typename... Members, typename... Results>
            constexpr auto get_members_skip_shadowed(type_list<Member, Members...>, type_list<Results...>)
            {
                if constexpr ((... || (Results::name == Member::name))) {
                    return get_members_skip_shadowed(type_list<Members...>{}, type_list<Results...>{});
                }
                else {
                    return get_members_skip_shadowed(type_list<Members...>{}, type_list<Results..., Member>{});
                }
            }

            template <typename T>
            using members_skip_shadowed = decltype(get_members_skip_shadowed(member_list<T>{}, type_list<>{}));

            /** Implements a proxy for a reflected function. */
            template <typename Derived, typename Func>
            struct REFL_DETAIL_FORCE_EBO function_proxy : public get_member_info<Func>::type::template remap<function_proxy<Derived, Func>>
            {
                function_proxy()
                {
                }

                template <typename Self, typename... Args>
                static constexpr decltype(auto) invoke_impl(Self&& self, Args&& ... args)
                {
                    return Derived::template invoke_impl<Func>(static_ref_cast<Derived>(self), std::forward<Args>(args)...);
                }
            };

            template <typename, typename>
            struct REFL_DETAIL_FORCE_EBO function_proxies;

            /** Implements a proxy for all reflected functions. */
            template <typename Derived, typename... Members>
            struct REFL_DETAIL_FORCE_EBO function_proxies<Derived, type_list<Members...>> : public function_proxy<Derived, Members>...
            {
            };

            /** Implements a proxy for a reflected field. */
            template <typename Derived, typename Field>
            struct REFL_DETAIL_FORCE_EBO field_proxy : public get_member_info<Field>::type::template remap<field_proxy<Derived, Field>>
            {
                field_proxy()
                {
                }

                template <typename Self, typename... Args>
                static constexpr decltype(auto) invoke_impl(Self&& self, Args&& ... args)
                {
                    return Derived::template invoke_impl<Field>(static_ref_cast<Derived>(self), std::forward<Args>(args)...);
                }
            };


            template <typename, typename>
            struct REFL_DETAIL_FORCE_EBO field_proxies;

            /** Implements a proxy for all reflected fields. */
            template <typename Derived, typename... Members>
            struct REFL_DETAIL_FORCE_EBO field_proxies<Derived, type_list<Members...>> : public field_proxy<Derived, Members>...
            {
            };

            template <typename T>
            using functions = trait::filter_t<trait::is_function, members_skip_shadowed<T>>;

            template <typename T>
            using fields = trait::filter_t<trait::is_field, members_skip_shadowed<T>>;

        } // namespace detail

        /**
         * @brief A proxy object that has a static interface identical to the reflected functions and fields of the target.
         *
         * A proxy object that has a static interface identical to the reflected functions and fields of the target.
         * Users should inherit from this class and specify an invoke_impl(Member member, Args&&... args) function.
         *
         * # Examples:
         * \code{.cpp}
         * template <typename T>
         * struct dummy_proxy : refl::runtime::proxy<dummy_proxy<T>, T> {
         *     template <typename Member, typename Self, typename... Args>
         *     static int invoke_impl(Self&& self, Args&&... args) {
         *          std::cout << get_debug_name(Member()) << " called with " << sizeof...(Args) << " parameters!\n";
         *          return 0;
         *     }
         * };
         * \endcode
         */
        template <typename Derived, typename Target>
        struct REFL_DETAIL_FORCE_EBO proxy
            : public detail::function_proxies<proxy<Derived, Target>, detail::functions<Target>>
            , public detail::field_proxies<proxy<Derived, Target>, detail::fields<Target>>
        {
            static_assert(
                sizeof(detail::function_proxies<proxy<Derived, Target>, detail::functions<Target>>) == 1 &&
                sizeof(detail::field_proxies<proxy<Derived, Target>, detail::fields<Target>>) == 1,
                "Multiple inheritance EBO did not kick in! "
                "You could try defining the REFL_DETAIL_FORCE_EBO macro appropriately to enable it on the required types. "
                "Default for MSC is `__declspec(empty_bases)`.");

            static_assert(
                trait::is_reflectable_v<Target>,
                "Target type must be reflectable!");

            typedef Target target_type;

            constexpr proxy() noexcept {}

        private:

            template <typename P, typename F>
            friend struct detail::function_proxy;

            template <typename P, typename F>
            friend struct detail::field_proxy;

            // Called by one of the function_proxy/field_proxy bases.
            template <typename Member, typename Self, typename... Args>
            static constexpr decltype(auto) invoke_impl(Self&& self, Args&& ... args)
            {
                return Derived::template invoke_impl<Member>(detail::static_ref_cast<Derived>(self), std::forward<Args>(args)...);
            }

        };

    } // namespace runtime

    namespace trait
    {
        template <typename>
        struct is_proxy;

        template <typename T>
        struct is_proxy
        {
        private:
            template <typename Derived, typename Target>
            static std::true_type test(runtime::proxy<Derived, Target>*);
            static std::false_type test(...);
        public:
            static constexpr bool value{ !std::is_reference_v<T> && decltype(test(std::declval<remove_qualifiers_t<T>*>()))::value };
        };

        template <typename T>
        [[maybe_unused]] static constexpr bool is_proxy_v{ is_proxy<T>::value };
    }

    namespace runtime
    {
        template <typename CharT, typename T>
        void debug(std::basic_ostream<CharT>& os, const T& value, bool compact = false);

        namespace detail
        {
            template <typename CharT, typename T, typename = decltype(std::declval<std::basic_ostream<CharT>&>() << std::declval<T>())>
            std::true_type is_ostream_printable_test(int);

            template <typename CharT, typename T>
            std::false_type is_ostream_printable_test(...);

            template <typename CharT, typename T>
            constexpr bool is_ostream_printable_v{ decltype(is_ostream_printable_test<CharT, T>(0))::value };

            namespace
            {
                [[maybe_unused]] int next_depth(int depth)
                {
                    return depth == -1 || depth > 8
                        ? -1
                        : depth + 1;
                }
            }

            template <typename CharT>
            void indent(std::basic_ostream<CharT>& os, int depth)
            {
                for (int i = 0; i < depth; i++) {
                    os << "    ";
                }
            }

            template <typename CharT, typename T>
            void debug_impl(std::basic_ostream<CharT>& os, const T& value, [[maybe_unused]] int depth);

            template <typename CharT, typename T>
            void debug_detailed(std::basic_ostream<CharT>& os, const T& value, int depth)
            {
                using type_descriptor = type_descriptor<T>;
                bool compact = depth == -1;
                // print type with members enclosed in braces
                os << type_descriptor::name << " { ";
                if (!compact) os << '\n';

                constexpr auto readable_members = filter(type_descriptor::members, [](auto member) { return is_readable(member); });
                for_each(readable_members, [&](auto member, [[maybe_unused]] auto index) {
                    int new_depth = next_depth(depth);

                    indent(os, new_depth);
                    os << get_display_name(member) << " = ";

                    if constexpr (util::contains_instance<attr::debug>(member.attributes)) {
                        // use the debug attribute to print
                        auto debug_attr = util::get_instance<attr::debug>(member.attributes);
                        debug_attr.write(os, value);
                    }
                    else {
                        debug_impl(os, member(value), new_depth);
                    }

                    if (!compact || index + 1 != readable_members.size) {
                        os << ", ";
                    }
                    if (!compact) {
                        indent(os, depth);
                        os << '\n';
                    }
                });

                if (compact) os << ' ';
                indent(os, depth);
                os << '}';
            }

            template <typename CharT, typename T>
            void debug_reflectable(std::basic_ostream<CharT>& os, const T& value, [[maybe_unused]] int depth)
            {
                using type_descriptor = type_descriptor<T>;
                if constexpr (trait::contains_instance_v<attr::debug, typename type_descriptor::attribute_types>) {
                    // use the debug attribute to print
                    auto debug_attr = util::get_instance<attr::debug>(type_descriptor::attributes);
                    debug_attr.write(os, value);
                }
                else if constexpr (detail::is_ostream_printable_v<CharT, T>) {
                    // type supports printing natively, just use that
                    os << value;
                }
                else {
                    debug_detailed(os, value, depth);
                }
            }

            template <typename CharT, typename T>
            void debug_container(std::basic_ostream<CharT>& os, const T& value, int depth)
            {
                bool compact = depth == -1;
                os << "[";

                auto end = value.end();
                for (auto it = value.begin(); it != end; ++it)
                {
                    if (!compact) os << '\n';
                    int new_depth = next_depth(depth);
                    indent(os, new_depth);

                    debug_impl(os, *it, new_depth);
                    if (std::next(it, 1) != end) {
                        os << ", ";
                    }
                    else if (!compact) {
                        os << '\n';
                    }
                }

                indent(os, depth);
                os << "]";
            }

            template <typename CharT, typename T>
            void debug_impl(std::basic_ostream<CharT>& os, const T& value, [[maybe_unused]] int depth)
            {
                using no_pointer_t = std::remove_pointer_t<T>;

                if constexpr (std::is_same_v<bool, T>) {
                    os << (value ? "true" : "false");
                }
                else if constexpr (std::is_pointer_v<T> && !std::is_void_v<no_pointer_t> && trait::is_reflectable_v<no_pointer_t>) {
                    if (value == nullptr) {
                        os << "nullptr";
                    }
                    else {
                        os << '&';
                        debug_impl(os, *value, -1);
                    }
                }
                else if constexpr (trait::is_reflectable_v<T>) {
                    debug_reflectable(os, value, depth);
                }
                else if constexpr (detail::is_ostream_printable_v<CharT, T>) {
                    os << value;
                }
                else if constexpr (trait::is_container_v<T>) {
                    debug_container(os, value, depth);
                }
                else {
                    os << "(not printable)";
                }
            }
        }

        /**
         * Writes the debug representation of value to the given std::ostream.
         * Calls the function specified by the debug<F> attribute whenever possible,
         * before falling back to recursively interating the members and printing them.
         * Takes an optional arguments specifying whether to print a compact representation.
         * The compact representation contains no newlines.
         */
        template <typename CharT, typename T>
        void debug(std::basic_ostream<CharT>& os, const T& value, [[maybe_unused]] bool compact)
        {
            static_assert(trait::is_reflectable_v<T> || trait::is_container_v<T> || detail::is_ostream_printable_v<CharT, T>,
                "Type is not reflectable, not a container of reflectable types and does not support operator<<(std::ostream&, T)!");

            detail::debug_impl(os, value, compact ? -1 : 0);
        }

        /**
         * Writes the compact debug representation of the provided values to the given std::ostream.
         */
        template <typename CharT, typename... Ts>
        void debug_all(std::basic_ostream<CharT>& os, const Ts&... values)
        {
            refl::runtime::debug(os, std::forward_as_tuple(static_cast<const Ts&>(values)...), true);
        }

        /**
         * Writes the debug representation of the provided value to an std::string and returns it.
         * Takes an optional arguments specifying whether to print a compact representation.
         * The compact representation contains no newlines.
         */
        template <typename CharT = char, typename T>
        std::basic_string<CharT> debug_str(const T& value, bool compact = false)
        {
            std::basic_stringstream<CharT> ss;
            debug(ss, value, compact);
            return ss.str();
        }

        /**
         * Writes the compact debug representation of the provided values to an std::string and returns it.
         */
        template <typename CharT = char, typename... Ts>
        std::basic_string<CharT> debug_all_str(const Ts&... values)
        {
            return refl::runtime::debug_str(std::forward_as_tuple(static_cast<const Ts&>(values)...), true);
        }

        /**
         * Invokes the specified member with the provided arguments.
         * When used with a member that is a field, the function gets or sets the value of the field.
         * The list of members is initially filtered by the type of the arguments provided.
         * THe filtered list is then searched at runtime by member name for the specified member
         * and that member is then invoked by operator(). If no match is found,
         * an std::runtime_error is thrown.
         */
        template <typename U, typename T, typename... Args>
        U invoke(T&& target, const char* name, Args&&... args)
        {
            using type = std::remove_reference_t<T>;
            static_assert(refl::trait::is_reflectable_v<type>, "Unsupported type!");
            typedef type_descriptor<type> type_descriptor;

            std::optional<U> result;

            bool found{ false };
            for_each(type_descriptor::members, [&](auto member) {
                using member_t = decltype(member);
                if (found) return;

                if constexpr (std::is_invocable_r_v<U, decltype(member), T, Args...>) {
                    if constexpr (trait::is_member_v<member_t>) {
                        if (std::strcmp(member.name.c_str(), name) == 0) {
                            result.emplace(member(target, std::forward<Args>(args)...));
                            found = true;
                        }
                    }
                }
            });

            if (found) {
                return std::move(*result);
            }
            else {
                throw std::runtime_error(std::string("The member ")
                    + type_descriptor::name.str() + "::" + name
                    + " is not compatible with the provided parameters or return type, is not reflected or does not exist!");
            }
        }

    } // namespace runtime

} // namespace refl

namespace refl::detail
{
    constexpr bool validate_attr_unique(type_list<>) noexcept
    {
        return true;
    }

    /** Statically asserts that all types in Ts... are unique. */
    template <typename T, typename... Ts>
    constexpr bool validate_attr_unique(type_list<T, Ts...>) noexcept
    {
        constexpr bool cond = (... && (!std::is_same_v<T, Ts> && validate_attr_unique(type_list<Ts>{})));
        static_assert(cond, "Some of the attributes provided have duplicate types!");
        return cond;
    }

    template <typename Req, typename Attr>
    constexpr bool validate_attr_usage() noexcept
    {
        return std::is_base_of_v<Req, Attr>;
    }

    /**
     * Statically asserts that all arguments inherit
     * from the appropriate bases to be used with Req.
     * Req must be one of the types defined in attr::usage.
     */
    template <typename Req, typename... Args>
    constexpr auto make_attributes(Args&&... args) noexcept
    {
        constexpr bool check_unique = validate_attr_unique(type_list<Args...>{});
        static_assert(check_unique, "Some of the supplied attributes cannot be used on this declaration!");

        constexpr bool check_usage = (... && validate_attr_usage<Req, trait::remove_qualifiers_t<Args>>());
        static_assert(check_usage, "Some of the supplied attributes cannot be used on this declaration!");

        return std::make_tuple(std::forward<Args>(args)...);
    }

    template <typename T, typename...>
    struct head
    {
        typedef T type;
    };

    /**
     * Accesses the first type T of Ts...
     * Used to allow for SFIANE to kick in in the implementation of REFL_FUNC.
     */
    template <typename T, typename... Ts>
    using head_t = typename head<T, Ts...>::type;

    template <typename T, typename U>
    struct transfer_const
    {
        using type = std::conditional_t<std::is_const_v<T>, std::add_const_t<U>, U>;
    };

    template <typename T, typename U>
    struct transfer_volatile
    {
        using type = std::conditional_t<std::is_volatile_v<T>, std::add_volatile_t<U>, U>;
    };

    template <typename T, typename U>
    struct transfer_cv : transfer_const<T, typename transfer_volatile<T, U>::type>
    {
    };

    template <typename T, typename U>
    struct transfer_lvalue_ref
    {
        using type = std::conditional_t<std::is_lvalue_reference_v<T>, std::add_lvalue_reference_t<U>, U>;
    };

    template <typename T, typename U>
    struct transfer_rvalue_ref
    {
        using type = std::conditional_t<std::is_rvalue_reference_v<T>, std::add_rvalue_reference_t<U>, U>;
    };

    template <typename T, typename U>
    struct transfer_ref : transfer_rvalue_ref<T, typename transfer_lvalue_ref<T, U>::type>
    {
    };

    template <typename T, typename U>
    struct transfer_cvref : transfer_ref<T, typename transfer_cv<std::remove_reference_t<T>, U>::type>
    {
    };

    template <typename T, typename U>
    constexpr auto forward_cast(std::remove_reference_t<T>& t) -> decltype(static_cast<typename transfer_cvref<T, U>::type&&>(t))
    {
        return static_cast<typename transfer_cvref<T, U>::type&&>(t);
    }

    template <typename T, typename U>
    constexpr auto forward_cast(std::remove_reference_t<T>&& t) -> decltype(static_cast<typename transfer_cvref<T, U>::type&&>(t))
    {
        static_assert(!std::is_lvalue_reference_v<T>, "template argument substituting T is an lvalue reference type");
        return static_cast<typename transfer_cvref<T, U>::type&&>(t);
    }

    template <typename T>
    constexpr auto get_type_name()
    {
        if constexpr (trait::is_reflectable_v<T>) {
            return type_descriptor<T>::name;
        }
        else {
            return make_const_string("(unknown)");
        }
    }

} // namespace refl::detail

/********************************/
/*  Metadata-generation macros  */
/********************************/

#define REFL_DETAIL_STR_IMPL(...) #__VA_ARGS__
/** Used to stringify input separated by commas (e.g. template specializations with multiple types). */
#define REFL_DETAIL_STR(...) REFL_DETAIL_STR_IMPL(__VA_ARGS__)
/** Used to group input containing commas (e.g. template specializations with multiple types). */
#define REFL_DETAIL_GROUP(...) __VA_ARGS__

/**
 * Expands to the appropriate attributes static member variable.
 * DeclType must be the name of one of the constraints defined in attr::usage.
 * __VA_ARGS__ is the list of attributes.
 */
#define REFL_DETAIL_ATTRIBUTES(DeclType, ...) \
        static constexpr auto attributes{ ::refl::detail::make_attributes<::refl::attr::usage:: DeclType>(__VA_ARGS__) }; \

/**
 * Expands to the body of a type_info__ specialization.
 */
#define REFL_DETAIL_TYPE_BODY(TypeName, ...) \
        typedef REFL_DETAIL_GROUP TypeName type; \
        REFL_DETAIL_ATTRIBUTES(type, __VA_ARGS__) \
        static constexpr auto name{ ::refl::util::make_const_string(REFL_DETAIL_STR(REFL_DETAIL_GROUP TypeName)) }; \
        static constexpr size_t member_index_offset = __COUNTER__ + 1; \
        template <size_t, typename = void> \
        struct member {};

/**
 * Creates reflection information for a specified type. Takes an optional attribute list.
 * This macro must only be expanded in the global namespace.
 *
 * # Examples:
 * ```
 * REFL_TYPE(Point)
 * ...
 * REFL_END
 * ```
 */
#define REFL_TYPE(TypeName, ...) \
    namespace refl_impl::metadata { template<> struct type_info__<TypeName> { \
        REFL_DETAIL_TYPE_BODY((TypeName), __VA_ARGS__)

/**
 * Creates reflection information for a specified type template. Takes an optional attribute list.
 * TemplateDeclaration must be a panenthesis-enclosed list declaring the template parameters. (e.g. (typename A, typename B)).
 * TypeName must be the fully-specialized type name and should also be enclosed in panenthesis. (e.g. (MyType<A, B>))
 * This macro must only be expanded in the global namespace.
 *
 * # Examples:
 * ```
 * REFL_TEMPLATE((typename T), (std::vector<T>))
 * ...
 * REFL_END
 * ```
 */
#define REFL_TEMPLATE(TemplateDeclaration, TypeName, ...) \
    namespace refl_impl::metadata { template <REFL_DETAIL_GROUP TemplateDeclaration> struct type_info__<REFL_DETAIL_GROUP TypeName> { \
        REFL_DETAIL_TYPE_BODY(TypeName, __VA_ARGS__)

/**
 * Terminated the declaration of reflection metadata for a particular type.
 *
 * # Examples:
 * ```
 * REFL_TYPE(Point)
 * ...
 * REFL_END
 */
#define REFL_END \
        static constexpr size_t member_count{ __COUNTER__ - member_index_offset }; \
    }; }

#define REFL_DETAIL_MEMBER_HEADER template<typename Unused__> struct member<__COUNTER__ - member_index_offset, Unused__>

#define REFL_DETAIL_MEMBER_COMMON(MemberType_, MemberName_, ...) \
        typedef ::refl::member::MemberType_ member_type; \
        static constexpr auto name{ ::refl::util::make_const_string(REFL_DETAIL_STR(MemberName_)) }; \
        REFL_DETAIL_ATTRIBUTES(MemberType_, __VA_ARGS__)

/** Creates the support infrastructure needed for the refl::runtime::proxy type. */
/*
    There can be a total of 12 differently qualified member functions with the same name.
    Providing remaps for non-const and const-only strikes a balance between compilation time and usability.
    And even though there are many other remap implementation possibilities (like virtual, field variants),
    adding them was considered to not be efficient from a compilation-time point of view.
*/
#define REFL_DETAIL_MEMBER_PROXY(MemberName_) \
        template <typename Proxy> struct remap { \
            template <typename... Args> decltype(auto) MemberName_(Args&&... args) { \
                return Proxy::invoke_impl(static_cast<Proxy&>(*this), ::std::forward<Args>(args)...); \
            } \
            template <typename... Args> decltype(auto) MemberName_(Args&&... args) const { \
                return Proxy::invoke_impl(static_cast<const Proxy&>(*this), ::std::forward<Args>(args)...); \
            } \
        }

/**
 * Creates reflection information for a public field. Takes an optional attribute list.
 */
#define REFL_FIELD(FieldName_, ...) \
    REFL_DETAIL_MEMBER_HEADER { \
        REFL_DETAIL_MEMBER_COMMON(field, FieldName_, __VA_ARGS__) \
    public: \
        typedef decltype(type::FieldName_) value_type; \
        static constexpr auto pointer{ &type::FieldName_ }; \
        REFL_DETAIL_MEMBER_PROXY(FieldName_); \
    };

/**
 * Creates reflection information for a public functions. Takes an optional attribute list.
 */
#define REFL_FUNC(FunctionName_, ...) \
    REFL_DETAIL_MEMBER_HEADER { \
        REFL_DETAIL_MEMBER_COMMON(function, FunctionName_, __VA_ARGS__) \
        public: \
        template<typename Self, typename... Args> static constexpr auto invoke(Self&& self, Args&&... args) -> decltype(::refl::detail::forward_cast<Self, type>(self).FunctionName_(::std::forward<Args>(args)...)) {\
            return ::refl::detail::forward_cast<Self, type>(self).FunctionName_(::std::forward<Args>(args)...); \
        } \
        template<typename... Args> static constexpr auto invoke(Args&&... args) -> decltype(::refl::detail::head_t<type, Args...>::FunctionName_(::std::declval<Args>()...)) { \
            return ::refl::detail::head_t<type, Args...>::FunctionName_(::std::forward<Args>(args)...); \
        } \
        template <typename Dummy = void> \
        static constexpr auto pointer() -> decltype(&::refl::detail::head_t<type, Dummy>::FunctionName_) { return &::refl::detail::head_t<type, Dummy>::FunctionName_; } \
        template <typename Pointer> \
        static constexpr auto resolve() -> ::std::decay_t<decltype(Pointer(&type::FunctionName_))> { return Pointer(&type::FunctionName_); } \
        REFL_DETAIL_MEMBER_PROXY(FunctionName_); \
    };

/********************************/
/*  Default Reflection Metadata */
/********************************/

#define REFL_DETAIL_PRIMITIVE(TypeName) \
    REFL_TYPE(TypeName) \
    REFL_END

    // Char types.
    REFL_DETAIL_PRIMITIVE(char);
    REFL_DETAIL_PRIMITIVE(wchar_t);
    REFL_DETAIL_PRIMITIVE(char16_t);
    REFL_DETAIL_PRIMITIVE(char32_t);
#ifdef __cpp_lib_char8_t
    REFL_DETAIL_PRIMITIVE(char8_t);
#endif

    // Integral types.
    REFL_DETAIL_PRIMITIVE(bool);
    REFL_DETAIL_PRIMITIVE(signed char);
    REFL_DETAIL_PRIMITIVE(unsigned char);
    REFL_DETAIL_PRIMITIVE(signed short);
    REFL_DETAIL_PRIMITIVE(unsigned short);
    REFL_DETAIL_PRIMITIVE(signed int);
    REFL_DETAIL_PRIMITIVE(unsigned int);
    REFL_DETAIL_PRIMITIVE(signed long);
    REFL_DETAIL_PRIMITIVE(unsigned long);
    REFL_DETAIL_PRIMITIVE(signed long long);
    REFL_DETAIL_PRIMITIVE(unsigned long long);

    // Floating point types.
    REFL_DETAIL_PRIMITIVE(float);
    REFL_DETAIL_PRIMITIVE(double);
    REFL_DETAIL_PRIMITIVE(long double);

    // Other types.
    REFL_DETAIL_PRIMITIVE(decltype(nullptr));

    // Void type.
    REFL_TYPE(void)
    REFL_END

#undef REFL_DETAIL_PRIMITIVE

#define REFL_DETAIL_POINTER(Ptr) \
        template<typename T> \
        struct type_info__<T Ptr> { \
            typedef T Ptr type; \
            template <size_t N> \
            struct member {}; \
            static constexpr auto name{ ::refl::detail::get_type_name<T>() + ::refl::util::make_const_string(#Ptr) }; \
            static constexpr ::std::tuple<> attributes{ }; \
            static constexpr size_t member_count{ 0 }; \
        }

    namespace refl_impl
    {
        namespace metadata
        {
            REFL_DETAIL_POINTER(*);
            REFL_DETAIL_POINTER(&);
            REFL_DETAIL_POINTER(&&);
        }
    }

#undef REFL_DETAIL_POINTER

namespace refl::detail
{
    template <typename CharT>
    std::basic_string<CharT> convert(const std::string& str)
    {
        return std::basic_string<CharT>(str.begin(), str.end());
    }

#ifdef __cpp_lib_string_view
    struct write_basic_string_view
    {
        template <typename CharT, typename Traits>
        void operator()(std::basic_ostream<CharT>& os, std::basic_string_view<CharT, Traits> str) const
        {
            // some vers of clang dont have std::quoted(string_view) overload
            if (!str.back()) { // no copy needed when null-terminated
                os << std::quoted(str.data());
            }
            else {
                os << std::quoted(std::basic_string<CharT, Traits>(str.begin(), str.end()));
            }
        }
    };
#endif

    struct write_basic_string
    {
        template <typename CharT, typename Traits, typename Allocator>
        void operator()(std::basic_ostream<CharT>& os, const std::basic_string<CharT, Traits, Allocator>& str) const
        {
            os << std::quoted(str);
        }
    };

    struct write_exception
    {
        template <typename CharT>
        void operator()(std::basic_ostream<CharT>& os, const std::exception& e) const
        {
            os << convert<CharT>("Exception");
    #ifdef REFL_RTTI_ENABLED
            os << convert<CharT>(" (") << convert<CharT>(typeid(e).name()) << convert<CharT>(")");
    #endif
            os << convert<CharT>(": `") << e.what() << convert<CharT>("`");
        }
    };

    struct write_tuple
    {
        template <typename CharT, typename Tuple, size_t... Idx>
        void write(std::basic_ostream<CharT>& os, Tuple&& t, std::index_sequence<Idx...>) const
        {
            os << CharT('(');
            for_each(type_list<std::integral_constant<size_t, Idx>...>{}, [&](auto idx_c) {
                using idx_t = decltype(idx_c);
                runtime::debug(os, std::get<idx_t::value>(t));
                if constexpr (sizeof...(Idx) - 1 != idx_t::value) {
                    os << convert<CharT>(", ");
                }
            });
            os << CharT(')');
        }

        template <typename CharT, typename... Ts>
        void operator()(std::basic_ostream<CharT>& os, const std::tuple<Ts...>& t) const
        {
            write(os, t, std::make_index_sequence<sizeof...(Ts)>{});
        }
    };

    struct write_pair
    {
        template <typename CharT, typename K, typename V>
        void operator()(std::basic_ostream<CharT>& os, const std::pair<K, V>& t) const
        {
            os << CharT('(');
            runtime::debug(os, t.first);
            os << convert<CharT>(", ");
            runtime::debug(os, t.second);
            os << CharT(')');
        }
    };

    struct write_unique_ptr
    {
        template <typename CharT, typename T, typename D>
        void operator()(std::basic_ostream<CharT>& os, const std::unique_ptr<T, D>& t) const
        {
            runtime::debug(os, t.get(), true);
        }
    };

    struct write_shared_ptr
    {
        template <typename CharT, typename T>
        void operator()(std::basic_ostream<CharT>& os, const std::shared_ptr<T>& t) const
        {
            runtime::debug(os, t.get(), true);
        }
    };

    struct write_weak_ptr
    {
        template <typename CharT, typename T>
        void operator()(std::basic_ostream<CharT>& os, const std::weak_ptr<T>& t) const
        {
            runtime::debug(os, t.lock().get(), true);
        }
    };

    struct write_complex
    {
        template <typename CharT, typename T>
        void operator()(std::basic_ostream<CharT>& os, const std::complex<T>& t) const
        {
            runtime::debug(os, t.real());
            os << CharT('+');
            runtime::debug(os, t.imag());
            os << CharT('i');
        }
    };
} // namespace refl::detail

// Custom reflection information for
// some common built-in types (std::basic_string, std::tuple, std::pair).

#ifndef REFL_NO_STD_SUPPORT

REFL_TYPE(std::exception, debug{ refl::detail::write_exception() })
    REFL_FUNC(what, property{ })
REFL_END

REFL_TEMPLATE(
    (typename Elem, typename Traits, typename Alloc),
    (std::basic_string<Elem, Traits, Alloc>),
    debug{ refl::detail::write_basic_string() })
    REFL_FUNC(size, property{ })
    REFL_FUNC(data, property{ })
REFL_END

#ifdef __cpp_lib_string_view

REFL_TEMPLATE(
    (typename Elem, typename Traits),
    (std::basic_string_view<Elem, Traits>),
    debug{ refl::detail::write_basic_string_view() })
    REFL_FUNC(size, property{ })
    REFL_FUNC(data, property{ })
REFL_END

#endif

REFL_TEMPLATE(
    (typename... Ts),
    (std::tuple<Ts...>),
    debug{ refl::detail::write_tuple() })
REFL_END

REFL_TEMPLATE(
    (typename T, typename D),
    (std::unique_ptr<T, D>),
    debug{ refl::detail::write_unique_ptr() })
REFL_END

REFL_TEMPLATE(
    (typename T),
    (std::shared_ptr<T>),
    debug{ refl::detail::write_shared_ptr() })
REFL_END

REFL_TEMPLATE(
    (typename K, typename V),
    (std::pair<K, V>),
    debug{ refl::detail::write_pair() })
REFL_END

#ifndef REFL_NO_STD_COMPLEX

REFL_TEMPLATE(
    (typename T),
    (std::complex<T>),
    debug{ refl::detail::write_complex() })
REFL_END

#endif // !REFL_NO_STD_COMPLEX

#endif // !REFL_NO_STD_SUPPORT

#ifndef REFL_NO_AUTO_MACRO

#define REFL_DETAIL_EXPAND(x) x
#define REFL_DETAIL_FOR_EACH_0(...)
#define REFL_DETAIL_FOR_EACH_1(what, x, ...) what(x)
#define REFL_DETAIL_FOR_EACH_2(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_1(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_3(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_2(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_4(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_3(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_5(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_4(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_6(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_5(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_7(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_6(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_8(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_7(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_9(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_8(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_10(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_9(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_11(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_10(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_12(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_11(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_13(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_12(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_14(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_13(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_15(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_14(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_16(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_15(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_17(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_16(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_18(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_17(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_19(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_18(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_20(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_19(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_21(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_20(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_22(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_21(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_23(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_22(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_24(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_23(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_25(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_24(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_26(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_25(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_27(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_26(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_28(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_27(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_29(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_28(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_30(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_29(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_31(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_30(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_32(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_31(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_33(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_32(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_34(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_33(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_35(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_34(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_36(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_35(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_37(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_36(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_38(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_37(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_39(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_38(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_40(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_39(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_41(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_40(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_42(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_41(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_43(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_42(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_44(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_43(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_45(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_44(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_46(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_45(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_47(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_46(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_48(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_47(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_49(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_48(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_50(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_49(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_51(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_50(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_52(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_51(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_53(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_52(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_54(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_53(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_55(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_54(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_56(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_55(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_57(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_56(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_58(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_57(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_59(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_58(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_60(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_59(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_61(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_60(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_62(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_61(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_63(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_62(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_64(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_63(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_65(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_64(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_66(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_65(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_67(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_66(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_68(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_67(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_69(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_68(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_70(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_69(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_71(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_70(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_72(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_71(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_73(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_72(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_74(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_73(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_75(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_74(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_76(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_75(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_77(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_76(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_78(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_77(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_79(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_78(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_80(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_79(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_81(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_80(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_82(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_81(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_83(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_82(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_84(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_83(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_85(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_84(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_86(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_85(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_87(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_86(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_88(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_87(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_89(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_88(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_90(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_89(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_91(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_90(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_92(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_91(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_93(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_92(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_94(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_93(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_95(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_94(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_96(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_95(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_97(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_96(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_98(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_97(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_99(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_98(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_100(what, x, ...) what(x) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_99(what, __VA_ARGS__))

#define REFL_DETAIL_FOR_EACH_NARG(...) REFL_DETAIL_FOR_EACH_NARG_(__VA_ARGS__, REFL_DETAIL_FOR_EACH_RSEQ_N())
#define REFL_DETAIL_FOR_EACH_NARG_(...) REFL_DETAIL_EXPAND(REFL_DETAIL_FOR_EACH_ARG_N(__VA_ARGS__))
#define REFL_DETAIL_FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, _97, _98, _99, _100, N, ...) N
#define REFL_DETAIL_FOR_EACH_RSEQ_N() 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define REFL_DETAIL_CONCATENATE(x, y) x##y
#define REFL_DETAIL_FOR_EACH_(N, what, ...) REFL_DETAIL_EXPAND(REFL_DETAIL_CONCATENATE(REFL_DETAIL_FOR_EACH_, N)(what, __VA_ARGS__))
#define REFL_DETAIL_FOR_EACH(what, ...) REFL_DETAIL_FOR_EACH_(REFL_DETAIL_FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

// Intellisense does not work nicely with passing variadic parameters (for the attributes)
// through all of the macro expansions and causes differently named member declarations to be
// used during code inspection.
#ifdef __INTELLISENSE__

#define REFL_DETAIL_EX_1_type(X, ...) REFL_TYPE(X)
#define REFL_DETAIL_EX_1_template(X, Y, ...) REFL_TEMPLATE(X, Y)
#define REFL_DETAIL_EX_1_field(X, ...) REFL_FIELD(X)
#define REFL_DETAIL_EX_1_func(X, ...) REFL_FUNC(X)

#else // !defined(__INTELLISENSE__)

#define REFL_DETAIL_EX_1_type(...) REFL_DETAIL_EX_EXPAND(REFL_DETAIL_EX_DEFER(REFL_TYPE)(__VA_ARGS__))
#define REFL_DETAIL_EX_1_template(...) REFL_DETAIL_EX_EXPAND(REFL_DETAIL_EX_DEFER(REFL_TEMPLATE)(__VA_ARGS__))
#define REFL_DETAIL_EX_1_field(...) REFL_DETAIL_EX_EXPAND(REFL_DETAIL_EX_DEFER(REFL_FIELD)(__VA_ARGS__))
#define REFL_DETAIL_EX_1_func(...) REFL_DETAIL_EX_EXPAND(REFL_DETAIL_EX_DEFER(REFL_FUNC)(__VA_ARGS__))

#endif // __INTELLISENSE__

#define REFL_DETAIL_EX_(Specifier, ...) REFL_DETAIL_EX_1_##Specifier __VA_ARGS__

#define REFL_DETAIL_EX_EMPTY()
#define REFL_DETAIL_EX_DEFER(Id) Id REFL_DETAIL_EX_EMPTY()
#define REFL_DETAIL_EX_EXPAND(...)  __VA_ARGS__

#define REFL_DETAIL_EX_END() REFL_END

#define REFL_AUTO(...) REFL_DETAIL_FOR_EACH(REFL_DETAIL_EX_, __VA_ARGS__) REFL_DETAIL_EX_EXPAND(REFL_DETAIL_EX_DEFER(REFL_DETAIL_EX_END)())

#endif // !defined(REFL_NO_AUTO_MACRO)

#endif // REFL_INCLUDE_HPP
