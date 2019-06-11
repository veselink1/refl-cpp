/**
 * Created by veselink1.
 * Released under the MIT license.
 */

#ifndef REFL_INCLUDE_HPP
#define REFL_INCLUDE_HPP

#include <stddef.h> // size_t
#include <cstring>
#include <array>
#include <memory>
#include <utility> // std::move, std::forward
#include <optional>
#include <tuple>
#include <type_traits>
#include <ostream>
#include <sstream>
#include <iomanip> // std::quoted

#ifdef _MSC_VER
// Disable VS warning for "Not enough arguments for macro"
// (emitted when a REFL_ macro is not provided any attributes)
#pragma warning( disable : 4003 )
#endif

/// <summary>
/// using util::type_list; <br>
/// using descriptor::type_descriptor; <br>
/// using descriptor::field_descriptor; <br>
/// using descriptor::function_descriptor; <br>
/// using util::const_string; <br>
/// using util::make_const_string; <br>
/// </summary>
namespace refl
{
    namespace util
    {
        /// <summary>
        /// Represents a compile-time string. 
        /// </summary>
        template <size_t N>
        struct const_string
        {
            static constexpr size_t size = N; 

            char data[N + 1];

            constexpr const_string() noexcept = default;

            constexpr const_string(const char(&data)[N + 1]) noexcept
                : data{}
            {
                for (size_t i = 0; i < N; i++)
                    this->data[i] = data[i];
            }

            constexpr operator const char*() const noexcept
            {
                return data;
            }

            constexpr const char* c_str() const noexcept
            {
                return data;
            }

            std::string str() const noexcept
            {
                return data;
            }
            
        };

        /// <summary>
        /// Creates a compile-time string. 
        /// </summary>
        template <size_t N>
        constexpr const_string<N - 1> make_const_string(const char(&str)[N])
        {
            return str;
        }

        /// <summary>
        /// Concats two const_strings together.
        /// </summary>
        template <size_t N, size_t M>
        constexpr const_string<N + M> operator+(const const_string<N>& a, const const_string<M>& b)
        {
            char data[N + M] { };
            for (size_t i = 0; i < N; i++)
                data[i] = a.data[i];
            for (size_t i = 0; i < N; i++)
                data[N + i] = b.data[i];
            return data;
        }
        
        template <size_t N, size_t M>
        constexpr bool operator==(const const_string<N>& a, const const_string<M>& b)
        {
            if constexpr (N != M) {
                return false;
            }
            else {
                for (size_t i = 0; i < M; i++) {
                    if (a[i] != b[i]) {
                        return false;
                    }
                }
                return true;
            }
        }

        template <size_t N, size_t M>
        constexpr bool operator!=(const const_string<N>& a, const const_string<M>& b)
        {
            return !(a == b);
        }
        
    } // namespace util

    using util::const_string;
    using util::make_const_string;
    using util::operator+;
    using util::operator==;
    using util::operator!=;

    namespace detail::macro_exports
    {
    }
    
} // namespace refl

    namespace refl_impl
    {
        namespace metadata
        {
        using namespace refl::detail::macro_exports;

        /// <summary>
        /// The core reflection info type. 
        /// Should not be used by user code directly.
        /// </summary>
        template <typename T>
        struct type_info__
        {
            // Used for detecting this non-specialized type_info__. 
            struct invalid_marker{};

            // The names below are provided to support proper name-lookup. 
            // Otherwise, multiple errors will be generated, obscuring the actual
            // reason for the failiure.
            template <size_t, typename>
            struct member;
            
            static constexpr size_t member_count{ 0 };

            static constexpr refl::const_string<0> name{ "" };

            static constexpr std::tuple<> attributes{ };
        };

        // CV qualifiers are not taken into account when reflecting on a type.
        template <typename T>
        struct type_info__<const T> : public type_info__<T> {};

        template <typename T>
        struct type_info__<volatile T> : public type_info__<T> {};

        template <typename T>
        struct type_info__<const volatile T> : public type_info__<T> {};

        } // namespace metadata

    } // namespace refl_impl

namespace refl {

    namespace trait
    {
        /// <summary>
        /// Removes all reference and cv qualifiers of T.
        /// </summary>
        template <typename T>
        struct remove_qualifiers
        {
            typedef std::remove_cv_t<std::remove_reference_t<T>> type;
        };
        
        /// <summary>
        /// Removes all reference and cv qualifiers of T.
        /// </summary>
        template <typename T>
        using remove_qualifiers_t = typename remove_qualifiers<T>::type;

        namespace detail
        {
            template <typename T>
            decltype(typename refl_impl::metadata::type_info__<T>::invalid_marker{}, std::false_type{}) is_reflectable_test(int);
                
            template <typename T>
            std::true_type is_reflectable_test(...);
        }

        /// <summary>
        /// Checks whether the type T can be reflected.
        /// </summary>
        template <typename T>
        struct is_reflectable : decltype(detail::is_reflectable_test<T>(0))
        {
        };

        /// <summary>
        /// Checks whether the type T can be reflected.
        /// </summary>
        template <typename T>
        static constexpr bool is_reflectable_v{ is_reflectable<T>::value };

        namespace detail
        {
            template <typename U>
            static auto is_container_test(int) -> decltype(std::declval<U>().begin(), std::declval<U>().end(), std::true_type{});

            template <typename U>
            static std::false_type is_container_test(...);
        }

        /// <summary>
        /// Checks whether T supports begin() and end() member functions.
        /// </summary>
        template <typename T>
        struct is_container : decltype(detail::is_container_test<T>(0))
        {
        };

        /// <summary>
        /// Checks whether T supports begin() and end() member functions.
        /// </summary>
        template <typename T>
        static constexpr bool is_container_v{ is_container<T>::value };
    }

    /// <summary>
    /// An enumeration of the possible member types.
    /// </summary>
    namespace member
    {
        /// <summary>
        /// Represents a field.
        /// </summary>
        struct field {};

        /// <summary>
        /// Represents a function.
        /// </summary>
        struct function {};
    }

    namespace util
    {
        /// <summary>
        /// Represents a list of static types (implemented as variadic template parameters).
        /// </summary>
        template <typename... Ts>
        struct type_list
        {
            /// <summary>
            /// Represents the number of types in this type_list.
            /// </summary>
            static constexpr intptr_t size = sizeof...(Ts);
        };

    } // namespace util

    using util::type_list;

    namespace trait
    {
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

            static_assert(std::is_same_v<get<0, int>::type, int>, "Error!");
            static_assert(std::is_same_v<get<1, int, float>::type, float>, "Error!");

            template <size_t N, typename... Ts>
            struct skip;

            template <size_t N, typename T, typename... Ts>
            struct skip<N, T, Ts...> : public get<N - 1, Ts...>
            {
            };

            template <typename... Ts>
            struct skip<0, Ts...>
            {
                typedef type_list<Ts...> type;
            };
        }

        /// <summary>
        /// Accesses the N-th (0-based index) type in Ts...
        /// </summary>
        template <size_t N, typename... Ts>
        struct get : detail::get<N, Ts...>
        {
        };
        
        template <size_t N, typename... Ts>
        struct get<N, type_list<Ts...>> : detail::get<N, Ts...>
        {
        };

        /// <summary>
        /// Accesses the N-th (0-based index) type in Ts... (Ts may be a type_list<...> as well).
        /// </summary>
        template <size_t N, typename... Ts>
        using get_t = typename get<N, Ts...>::type;

        /// <summary>
        /// Skips the first N (0-based index) types in Ts... (Ts may be a type_list<...> as well).
        /// </summary>
        template <size_t N, typename... Ts>
        struct skip : detail::skip<N, Ts...>
        {
        };

        template <size_t N, typename... Ts>
        struct skip<N, type_list<Ts...>> : detail::skip<N, Ts...> 
        {
        };

        /// <summary>
        /// Skips the first N (0-based index) types in Ts... (Ts may be a type_list<...> as well).
        /// </summary>
        template <size_t N, typename... Ts>
        using skip_t = typename skip<N, Ts...>::type;

    } // namespace trait

	/// <summary>
	/// The built-in attributes as well as the attribute constraints in attr::usage.
	/// </summary>
    namespace attr
    {
		/// <summary>
		/// The attribute constraints.
		/// </summary>
        namespace usage
        {

            /// <summary>
            /// Specifies that an attribute type inheriting from this type can only be 
            /// used with REFL_TYPE(...).
            /// </summary>
            struct type {};

            /// <summary>
            /// Specifies that an attribute type inheriting from this type can only be 
            /// used with REFL_FUNC(...).
            /// </summary>
            struct function {};

            /// <summary>
            /// Specifies that an attribute type inheriting from this type can only be 
            /// used with REFL_FIELD(...).
            /// </summary>
            struct field {};

			/// <summary>
			/// Specifies that an attribute type inheriting from this type can only be 
			/// used with either REFL_FUNC(...), REFL_FIELD(...).
			/// </summary>
			struct member : public function, public field{};

			/// <summary>
			/// Specifies that an attribute type inheriting from this type can be used 
			/// with all three of REFL_FUNC, REFL_FIELD, REFL_FUNC.
			/// </summary>
			struct any : public member, public type {};
        }

        namespace detail
        {
            constexpr bool validate_unique(type_list<>) 
            { 
                return true; 
            }

            /// <summary>
            /// Statically asserts that all arguments types in Ts... are unique.
            /// </summary>
            template <typename T, typename... Ts>
            constexpr bool validate_unique(type_list<T, Ts...>)
            {
                constexpr bool cond = (... && (!std::is_same_v<T, Ts> && validate_unique(type_list<Ts>{})));
                static_assert(cond, "Some of the attributes provided have duplicate types!");
                return cond;
            }

			template <typename Req, typename Attr>
			constexpr bool validate_usage()
			{
				return std::is_base_of_v<Req, Attr>;
			}

            template <typename Usage, typename... Args>
            constexpr std::tuple<std::remove_reference_t<Args>...> make_attributes(Args&&... args) noexcept
            {
                constexpr bool check_unique = validate_unique(type_list<Args...>{});
                static_assert(check_unique, "Some of the supplied attributes cannot be used on this declaration!");

                constexpr bool check_usage = (... && validate_usage<Usage, trait::remove_qualifiers_t<Args>>());
                static_assert(check_usage, "Some of the supplied attributes cannot be used on this declaration!");

                return std::tuple<std::remove_reference_t<Args>...>{ std::forward<Args>(args)... };
            }
        }
    } // namespace attr
    
    namespace detail
    {
        template <typename R, typename... Args>
        auto resolve(R(*fn)(Args...), Args&&... args) -> decltype(fn);

        #define REFL_DEFINE_QUALIFIED_RESOLVE(...) \
            template <typename R, typename T, typename... Args> \
            auto resolve(R(trait::remove_qualifiers_t<T>::*fn)(Args...) __VA_ARGS__, T&& target, Args&&... args) -> decltype(fn)
            
        REFL_DEFINE_QUALIFIED_RESOLVE();
        REFL_DEFINE_QUALIFIED_RESOLVE(const);
        REFL_DEFINE_QUALIFIED_RESOLVE(volatile);
        REFL_DEFINE_QUALIFIED_RESOLVE(const volatile);
        REFL_DEFINE_QUALIFIED_RESOLVE(&);
        REFL_DEFINE_QUALIFIED_RESOLVE(const&);
        REFL_DEFINE_QUALIFIED_RESOLVE(volatile&);
        REFL_DEFINE_QUALIFIED_RESOLVE(const volatile&);
        REFL_DEFINE_QUALIFIED_RESOLVE(&&);
        REFL_DEFINE_QUALIFIED_RESOLVE(const&&);
        REFL_DEFINE_QUALIFIED_RESOLVE(volatile&&);
        REFL_DEFINE_QUALIFIED_RESOLVE(const volatile&&);

        #undef REFL_DEFINE_QUALIFIED_RESOLVE
    }

#define REFL_STRINGIFY_IMPL(...) #__VA_ARGS__
#define REFL_STRINGIFY(...) REFL_STRINGIFY_IMPL(__VA_ARGS__)
#define REFL_GROUP(...) __VA_ARGS__
#define REFL_EXPAND_FIRST(A, ...) A
#define REFL_EXPAND_SECOND(A, B, ...) B 

#define REFL_DETAIL_ATTRIBUTES(DeclType, ...) \
        static constexpr auto attributes = ::refl::attr::detail::make_attributes<::refl::attr::usage:: DeclType>(__VA_ARGS__); \

#define REFL_DETAIL_TYPE_BODY(TypeName, ...) \
        typedef REFL_GROUP TypeName type; \
        REFL_DETAIL_ATTRIBUTES(type, __VA_ARGS__) \
    public: \
        static inline constexpr auto name{ ::refl::util::make_const_string(REFL_STRINGIFY(REFL_GROUP TypeName)) }; \
        static inline constexpr size_t member_index_offset = __COUNTER__ + 1; \
        template <size_t N, typename = void> \
        struct member {}; 

/// <summary>
/// Creates reflection information for a specified type. Takes an optional attribute list. 
/// </summary>
#define REFL_TYPE(TypeName, ...) \
    namespace refl_impl::metadata { template<> struct type_info__<TypeName> { \
        REFL_DETAIL_TYPE_BODY((TypeName), __VA_ARGS__)

#define REFL_TEMPLATE(TemplateDeclaration, TypeName, ...) \
    namespace refl_impl::metadata { template <REFL_GROUP TemplateDeclaration> struct type_info__<REFL_GROUP TypeName> { \
        public: REFL_DETAIL_TYPE_BODY(TypeName, __VA_ARGS__)

#define REFL_END \
        static constexpr size_t member_count = __COUNTER__ - member_index_offset; \
    }; }
	

#define REFL_DETAIL_MEMBER_HEADER template<typename Unused__> struct member<__COUNTER__ - member_index_offset, Unused__>

#define REFL_DETAIL_MEMBER_COMMON(MemberTy, MemberName, ...) \
        typedef type declaring_type; \
        typedef ::refl::member::MemberTy member_type; \
        static inline constexpr auto name{ ::refl::util::make_const_string(REFL_STRINGIFY(MemberName)) }; \
        REFL_DETAIL_ATTRIBUTES(MemberTy, __VA_ARGS__) 

/// <summary>
/// Creates reflection information for a public field. Takes an optional attribute list. 
/// </summary>
#define REFL_FIELD(fieldName, ...) \
    REFL_DETAIL_MEMBER_HEADER { \
        REFL_DETAIL_MEMBER_COMMON(field, fieldName, __VA_ARGS__) \
    public: \
        typedef decltype(type::fieldName) value_type; \
        static inline constexpr auto pointer{ &type::fieldName }; \
    };

/// <summary>
/// Creates reflection information for a public function. Takes an optional *brace-enclosed* attribute list. 
/// </summary>
#define REFL_FUNC(functionName, ...) \
    REFL_DETAIL_MEMBER_HEADER { \
        REFL_DETAIL_MEMBER_COMMON(function, functionName, __VA_ARGS__) \
    public: \
        template <typename... Args> \
        static constexpr decltype(::refl::detail::resolve(&type::functionName, std::declval<Args>()...)) pointer{ &type::functionName }; \
        /* 
            There can be a total of 12 differently qualified member functions with the same name. 
            Providing remaps for non-const and const-only strikes a balance between compilation time and usability.
            And even though there are many other remap implementation possibilities (like virtual, field variants),
            adding them is considered to be non-efficient for the compiler.
        */ \
        template <typename Proxy> struct remap { \
            template <typename... Args> decltype(auto) functionName(Args&&... args) { \
                return (static_cast<Proxy*>(this))->call_impl(::std::forward<Args>(args)...); \
            } \
            template <typename... Args> decltype(auto) functionName(Args&&... args) const { \
                return (static_cast<const Proxy*>(this))->call_impl(::std::forward<Args>(args)...); \
            } \
        }; \
    };

        namespace detail
        {
            /// <summary>
            /// Ignores all arguments.
            /// </summary>
            template <typename T = int>
            constexpr int ignore(...) noexcept 
            {
                return {};
            }
        }

        namespace trait
        {
            namespace detail
            {
                template <typename, typename>
                struct cons;

                template <typename T, typename... Args>
                struct cons<T, type_list<Args...>>
                {
                    using type = type_list<T, Args...>;
                };

                template <template<typename> typename, typename...>
                struct filter_impl;

                template <template<typename> typename Predicate>
                struct filter_impl<Predicate>
                {
                    using type = type_list<>;
                };

                template <template<typename> typename Predicate, typename Head, typename ...Tail>
                struct filter_impl<Predicate, Head, Tail...>
                {
                    using type = std::conditional_t<Predicate<Head>::value,
                        typename cons<Head, typename filter_impl<Predicate, Tail...>::type>::type,
                        typename filter_impl<Predicate, Tail...>::type
                    >;
                };

                template <template<typename> typename Predicate, typename... Ts>
                struct filter_impl<Predicate, type_list<Ts...>> : public filter_impl<Predicate, Ts...>
                {
                };

                /// <summary>
                /// filters a type_list according to a Predicate (a type-trait-like template type).
                /// </summary>
                template <template<typename> typename Predicate>
                struct filter
                {
                    template <typename... Ts>
                    using apply = typename detail::filter_impl<Predicate, Ts...>::type;
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
                    using type = typename cons<typename Mapper<Head>::type, typename map_impl<Mapper, Tail...>::type>::type;
                };

                template <template<typename> typename Mapper, typename... Ts>
                struct map_impl<Mapper, type_list<Ts...>> : public map_impl<Mapper, Ts...>
                {
                };

                /// <summary>
                /// filters a type_list according to a Predicate (a type-trait-like template type).
                /// </summary>
                template <template<typename> typename Mapper>
                struct map
                {
                    template <typename... Ts>
                    using apply = typename detail::map_impl<Mapper, Ts...>::type;
                };
            }

            /// <summary>
            /// Filters a type_list according to a predicate template.
            /// </summary>
            template <template<typename> typename Predicate, typename... Ts>
            using filter = typename detail::filter<Predicate>::template apply<Ts...>;

            static_assert(std::is_same_v<filter<std::is_integral, int, float>, type_list<int>>);
            static_assert(std::is_same_v<filter<std::is_integral, type_list<int, float>>, type_list<int>>);

            /// <summary>
            /// Maps a type_list according to a predicate template.
            /// </summary>
            template <template<typename> typename Mapper, typename... Ts>
            using map = typename detail::map<Mapper>::template apply<Ts...>;

            static_assert(std::is_same_v<map<std::remove_reference, int&, float&>, type_list<int, float>>);
            static_assert(std::is_same_v<map<std::remove_reference, type_list<int&, float&>>, type_list<int, float>>);

        } // namespace trait

        namespace util
        {
            /// <summary>
            /// Creates an array of type 'T' from the provided tuple.
			/// The common type T needs to be specified, in order to prevent any 
			/// errors when using the overload taking an empty std::tuple (as there is no common type then). 
            /// </summary>
            template <typename T, typename... Ts>
            constexpr std::array<T, sizeof...(Ts)> to_array(const std::tuple<Ts...>& tuple)
            {
                return std::apply([](auto&& ... args) -> std::array<T, sizeof...(Ts)> { return { std::forward<decltype(args)>(args)... }; }, tuple);
            }

            static_assert(std::is_same_v<decltype(to_array<int>(std::make_tuple(0, 0))), std::array<int, 2>>);

            /// <summary>
            /// Creates an empty array of type 'T'.
            /// </summary>
            template <typename T>
            constexpr std::array<T, 0> to_array(const std::tuple<>& tuple)
            {
                return {};
            }

            static_assert(std::is_same_v<decltype(to_array<int>(std::tuple<>{})), std::array<int, 0>>);

			namespace detail
			{
				template <typename T, size_t... Idx>
				constexpr auto to_tuple(const std::array<T, sizeof...(Idx)>& array)
				{
					if constexpr (sizeof...(Idx) == 0) return std::tuple<>{};
					else return make_tuple(std::get<Idx>(array)...);
				}
			}

            /// <summary>
            /// Creates a tuple from the provided array.
            /// </summary>
            template <typename T, size_t N>
            constexpr auto to_tuple(const std::array<T, N>& array)
            {
                return detail::to_tuple<T, std::make_index_sequence<N>>(array);
            }

            namespace detail
            {
                
                template <typename F, typename... Carry>
                constexpr auto eval_in_order_to_tuple(type_list<>, std::index_sequence<>, F&& f, Carry&&... carry)
                {
                    if constexpr (sizeof...(Carry) == 0) return std::tuple<>{};
                    else return std::make_tuple(std::forward<Carry>(carry)...);
                }

                // This whole jazzy workaround is needed since C++ does not specify
                // the order in which function arguments are evaluated and this leads 
                // to incorrect order of evaluation (noticeable when using indexes).
                // Otherwise we could simply do std::make_tuple(f(Ts{}, Idx)...).
                template <typename F, typename T, typename... Ts, size_t I, size_t... Idx, typename... Carry>
                constexpr auto eval_in_order_to_tuple(type_list<T, Ts...>, std::index_sequence<I, Idx...>, F&& f, Carry&&... carry)
                {
                    if constexpr (std::is_invocable_v<F, T, size_t>) {
                        return eval_in_order_to_tuple(
                            type_list<Ts...>{},
                            std::index_sequence<Idx...>{}, 
                            std::forward<F>(f),
                            std::forward<Carry>(carry)..., // carry the previous results over
                            f(T{}, I) // pass the current result after them
                        );
                    }
                    else { 
                        return eval_in_order_to_tuple(
                            type_list<Ts...>{},
                            std::index_sequence<Idx...>{}, 
                            std::forward<F>(f),
                            std::forward<Carry>(carry)..., // carry the previous results over
                            f(T{}) // pass the current result after them
                        );
                    }
                }
            }

            /// <summary>
            /// Applies function F to each type in the type_list, aggregating
            /// the results in a tuple. F can optionally take an index of type size_t.
            /// </summary>
            template <typename F, typename... Ts>
            constexpr auto map_to_tuple(type_list<Ts...> list, F&& f)
            {
                return detail::eval_in_order_to_tuple(list, std::make_index_sequence<sizeof...(Ts)>{}, std::forward<F>(f));
            }

            static_assert(map_to_tuple(type_list<int>{}, [](auto t) { return t; }) == std::tuple<int>{ 0 });

            /// <summary>
            /// Applies function F to each type in the type_list, aggregating
            /// the results in an array. F can optionally take an index of type size_t.
            /// </summary>
            template <typename T, typename F, typename... Ts>
            constexpr auto map_to_array(type_list<Ts...> list, F&& f)
            {
                return to_array<T>(map_to_tuple(list, std::forward<F>(f)));
            }

            /// <summary>
            /// Applies function F to each type in the type_list.
            /// F can optionally take an index of type size_t.
            /// </summary>
            template <typename F, typename... Ts>
            constexpr void for_each(type_list<Ts...> list, F&& f)
            {
                map_to_tuple(list, [&](auto&&... args) -> decltype(f(std::forward<decltype(args)>(args)...), 0) 
                { 
                    f(std::forward<decltype(args)>(args)...); 
                    return 0; 
                });
            }
            
            /// <summary>
            /// Applies an accumulation function F to each type in the type_list.
            /// F can optionally take an index of type size_t.
            /// </summary>
            template <typename R, typename F, typename... Ts>
            constexpr R accumulate(type_list<Ts...> list, F&& f, R&& initial_value)
            {
                R r{ initial_value };
                for_each(list, [&](auto&&... args) -> decltype(f(r, std::forward<decltype(args)>(args)...), 0) 
                { 
                    f(r, std::forward<decltype(args)>(args)...); 
                    return 0; 
                });
                return r;
            }

            /// <summary>
            /// Applies an accumulation function F to each type in the type_list.
            /// F can optionally take an index of type size_t.
            /// </summary>
            template <typename F, typename... Ts>
            constexpr size_t count_if(type_list<Ts...> list, F&& f)
            {
                return accumulate<size_t>(list, [&](size_t& current, auto&&... args) -> decltype(f(std::forward<decltype(args)>(args)...), void(0))
                {
                    if (f(std::forward<decltype(args)>(args)...)) 
                    {
                        current += 1;
                    }
                }, 0);
            }

            namespace detail
            {
                template <typename F, typename... Carry>
                constexpr auto filter(F f, type_list<> list, type_list<Carry...> carry) 
                {
                    return carry;
                }

                template <typename F, typename T, typename... Ts, typename... Carry>
                constexpr auto filter(F f, type_list<T, Ts...> list, type_list<Carry...> carry) 
                {
                    if constexpr (f(T{})) {
                        return filter(f, type_list<Ts...>{}, type_list<T, Carry...>{});
                    } 
                    else {
                        return filter(f, type_list<Ts...>{}, type_list<Carry...>{});
                    }
                }
            }

            /// <summary>
            /// Filters the list according to a predicate.
            /// </summary>
            template <typename F, typename... Ts>
            constexpr auto filter(type_list<Ts...> list, F f) 
            {
                return detail::filter(f, list, type_list<>{});
            }
            
            /// <summary>
            /// Returns the first instance that matches the predicate. 
            /// </summary>
            template <typename F, typename... Ts>
            constexpr auto find_first(type_list<Ts...> list, F f) 
            {
                using result_list = decltype(detail::filter(f, list, type_list<>{}));
                return trait::get_t<0, result_list>{};
            }

            /// <summary>
            /// Returns the only instance that matches the predicate. If there is no match or multiple matches, fails with static_assert. 
            /// </summary>
            template <typename F, typename... Ts>
            constexpr auto find_one(type_list<Ts...> list, F f) 
            {
                using result_list = decltype(detail::filter(f, list, type_list<>{}));
                static_assert(result_list::size == 1, "Cannot resolve multiple matches in find_one!");
                return trait::get_t<0, result_list>{};
            }

        } // namespace util

        namespace trait
        {
            namespace detail
            {
                template <typename T>
                auto member_type_test(int) -> decltype(typename T::member_type{}, std::true_type{});

                template <typename T>
                std::false_type member_type_test(...);
            }

            template <typename T>
            struct is_member : decltype(detail::member_type_test<T>(0)) 
            {
            };

            template <typename T>
            static constexpr bool is_member_v{ is_member<T>::value };

            /// <summary>
            /// A trait for detecting whether the type 'T' is a field descriptor.
            /// </summary>
            template <typename T>
            struct is_field : std::conjunction<is_member<T>, std::is_base_of<typename T::member_type, member::field>>
            {
            };
            
            template <typename T>
            static constexpr bool is_field_v{ is_field<T>::value };

            /// <summary>
            /// A trait for detecting whether the type 'T' is a function descriptor.
            /// </summary>
            template <typename T>
            struct is_function : std::conjunction<is_member<T>, std::is_base_of<typename T::member_type, member::function>>
            {
            };

            template <typename T>
            static constexpr bool is_function_v{ is_function<T>::value };
        }
        
        namespace descriptor
        {
            template <typename T>
            class type_descriptor; 

            template <typename T, size_t N>
            class member_descriptor_base
            {
            protected:

                typedef typename refl_impl::metadata::type_info__<T>::template member<N> member;

            public:

                /// <summary>
                /// An alias for the declaring type of the reflected member.
                /// </summary>
                typedef T declaring_type;

                /// <summary>
                /// An alias, specifying the exact type of member.
                /// </summary>
                typedef typename member::member_type member_type;

                /// <summary>
                /// The type_descriptor of the declaring type.
                /// </summary>
                static constexpr type_descriptor<T> declarator{ };

                /// <summary>
                /// The name of the reflected member.
                /// </summary>
                static constexpr auto name{ member::name };

                /// <summary>
                /// The attributes of the reflected member.
                /// </summary>
                static constexpr auto attributes{ member::attributes };

            };
            

            /// <summary>
            /// Represents a reflected field.
            /// </summary>
            template <typename T, size_t N>
            class field_descriptor : public member_descriptor_base<T, N>
            {
                using typename member_descriptor_base<T, N>::member;
            public:

                /// <summary>
                /// Type value type of the member.
                /// </summary>
                typedef typename member::value_type value_type;
                
                static constexpr bool is_static{ !std::is_member_pointer_v<decltype(member::pointer)> };
                
                static constexpr auto pointer{ member::pointer };

                /// <summary>
                /// Returns the value of the field. (for static fields).
                /// </summary>
                static constexpr decltype(auto) get()
                {
                    return *member::pointer;
                }
                
                /// <summary>
                /// A synonym for get().
                /// </summary>
                constexpr decltype(auto) operator()() const
                {
                    return get();
                }

                /// <summary>
                /// Returns the value of the field. (for instance fields).
                /// </summary>
                template <typename U>
                static constexpr decltype(auto) get(U&& target)
                {
                    return target.*(member::pointer); 
                }
                
                /// <summary>
                /// A synonym for get(target).
                /// </summary>
                template <typename U>
                constexpr decltype(auto) operator()(U&& target) const
                {
                    return get(std::forward<U>(target));
                }

            };
            
            /// <summary>
            /// Represents a reflected function.
            /// </summary>
            template <typename T, size_t N>
            class function_descriptor : public member_descriptor_base<T, N>
            {
                using typename member_descriptor_base<T, N>::member;
                
                template <typename Fn, typename Self, typename... Args>
                static constexpr decltype(auto) invoke_impl(Fn trait::remove_qualifiers_t<Self>::* pointer, Self&& target, Args&&... args) 
                {
                    return (target.*pointer)(std::forward<Args>(args)...);
                }

            public:

                /// <summary>
                /// Gets a pointer to the function, deduced by the provided Args... template parameters. 
                /// For member functions the first type must be the target type. (possibly reference- or cv-qualified)
                /// </summary>
                template <typename... Args>
                static constexpr auto pointer{ member::template pointer<Args...> };

                /// <summary>
                /// Invokes the function with the given arguments. 
                /// If the function is an instance function, a reference
                /// to the instance is provided as first argument.
                /// </summary>
                template <typename... Args>
                static constexpr decltype(auto) invoke(Args&&... args)
                {
                    constexpr auto pointer = member::template pointer<Args...>;
                    if constexpr (std::is_member_function_pointer_v<decltype(pointer)>) {
                        return invoke_impl(pointer, std::forward<Args>(args)...);
                    } 
                    else {
                        return (*pointer)(std::forward<Args>(args)...);
                    }
                }
                
                /// <summary>
                /// A synonym for invoke(args...). 
                /// </summary>
                template <typename... Args>
                constexpr decltype(auto) operator()(Args&&... args) const
                {
                    return invoke(std::forward<Args>(args)...); 
                }

            };
        }

        using descriptor::field_descriptor;
        using descriptor::function_descriptor;

        namespace detail
        {
            template <typename T, size_t N>
            using make_descriptor = std::conditional_t<refl::trait::is_field_v<typename refl_impl::metadata::type_info__<T>::template member<N>>,
                field_descriptor<T, N>,
                std::conditional_t<refl::trait::is_function_v<typename refl_impl::metadata::type_info__<T>::template member<N>>,
                    function_descriptor<T, N>,
                    void
                >>;

			template <typename T, size_t... Idx>
			type_list<make_descriptor<T, Idx>...> enumerate_members(std::index_sequence<Idx...>);

		} // namespace detail
        
        /// <summary>
        /// A type_list of the member descriptors of the target type T. 
        /// </summary>
		template <typename T>
		using member_list = decltype(detail::enumerate_members<T>(std::make_index_sequence<refl_impl::metadata::type_info__<T>::member_count>{}));

        namespace descriptor
        {
            /// <summary>
            /// Represents a reflected type.
            /// </summary>
            template <typename T>
            class type_descriptor
            {
            private:

                static_assert(refl::trait::is_reflectable_v<T>, "This type does not support reflection!");

                typedef refl_impl::metadata::type_info__<T> type_info__;
                
            public:

                /// <summary>
                /// The list of member descriptors.
                /// </summary>
                static constexpr refl::member_list<T> members{  };

                /// <summary>
                /// The name of the reflected type.
                /// </summary>
                static constexpr const auto name{ type_info__::name };
                
                /// <summary>
                /// The attributes of the reflected type.
                /// </summary>
                static constexpr const auto attributes{ type_info__::attributes };

            };
        }
        
        using descriptor::type_descriptor;

        /// <summary>
        /// Returns true if the type T is reflectable.
        /// </summary>
        template <typename T>
        constexpr bool is_reflectable()
        {
            return trait::is_reflectable_v<T>;
        }
        
        /// <summary>
        /// Returns true if the type non-reference T is reflectable.
        /// </summary>
        template <typename T>
        constexpr bool is_reflectable(T&& t)
        {
            return trait::is_reflectable_v<std::remove_reference_t<T>>;
        }

        /// <summary>
        /// Returns the type descriptor for the type T. 
        /// </summary>
        template<typename T>
        constexpr type_descriptor<T> reflect()
        {
            return {};
        }

        /// <summary>
        /// Returns the type descriptor for the non-reference type T. 
        /// </summary>
        template<typename T>
        constexpr type_descriptor<std::remove_reference_t<T>> reflect(T&& t)
        {
            return {};
        }

        namespace trait
        {
            namespace detail
            {
                template <typename T>
                struct is_instance : public std::false_type {};

                template <template<typename...> typename T, typename... Args>
                struct is_instance<T<Args...>> : public std::true_type {};
            }

            /// <summary>
            /// True if T is a template specialization.
            /// </summary>
            template <typename T>
            struct is_instance : detail::is_instance<T>
            {
            };

            template <typename T>
            static constexpr bool is_instance_v{ is_instance<T>::value };

            namespace detail
            {
                template <template<typename...>  typename T, typename U>
                struct is_instance_of : public std::false_type {};

                template <template<typename...> typename T, template<typename...> typename U, typename... Args>
                struct is_instance_of<T, U<Args...>> : public std::is_same<T<Args...>, U<Args...>>
                {
                };
            }

            /// <summary>
            /// True if the type U is a template specialization of U.
            /// </summary>
            template <template<typename...>typename T, typename U>
            struct is_instance_of : detail::is_instance_of<T, U>
            {
            };

            template <template<typename...>typename T, typename U>
            static constexpr bool is_instance_of_v{ is_instance_of<T, U>::value };

            static_assert(is_instance_v<std::unique_ptr<int>>, "Error!");
            static_assert(is_instance_of_v<std::unique_ptr, std::unique_ptr<float>>, "Error!");

            namespace detail
            {   
                template <typename T, typename... Ts>
                struct contains_impl : std::disjunction<std::is_same<Ts, T>...>
                {
                };
                
                template <typename T, typename... Ts>
                struct contains_impl<T, type_list<Ts...>> : contains_impl<T, Ts...>
                {
                };
                
                template <typename T, typename... Ts>
                struct contains_impl<T, std::tuple<Ts...>> : contains_impl<T, Ts...>
                {
                };
                
                template <template<typename...> typename T, typename... Ts>
                struct contains_instance_impl : std::disjunction<trait::is_instance_of<T, Ts>...>
                {
                };
                
                template <template<typename...> typename T, typename... Ts>
                struct contains_instance_impl<T, type_list<Ts...>> : contains_instance_impl<T, Ts...>
                {
                };
                
                template <template<typename...> typename T, typename... Ts>
                struct contains_instance_impl<T, std::tuple<Ts...>> : contains_instance_impl<T, Ts...>
                {
                };
                
                template <typename T, typename... Ts>
                struct contains_base_impl : std::disjunction<std::is_base_of<Ts, T>...>
                {
                };

                template <typename T, typename... Ts>
                struct contains_base_impl<T, type_list<Ts...>> : contains_base_impl<T, Ts...>
                {
                };
                
                template <typename T, typename... Ts>
                struct contains_base_impl<T, std::tuple<Ts...>> : contains_base_impl<T, Ts...>
                {
                };
            }

            /// <summary>
            /// Checks whether T is contained in the list of types.
            /// </summary>
            template <typename T, typename... Ts>
            struct contains : detail::contains_impl<remove_qualifiers_t<T>, remove_qualifiers_t<Ts>...>
            {
            };

            template <typename T, typename... Ts>
            static constexpr bool contains_v = contains<T, Ts...>::value;

            /// <summary>
            /// Checks whether an instance of the tempalte T is contained in the list of types.
            /// </summary>
            template <template<typename...> typename T, typename... Ts>
            struct contains_instance : detail::contains_instance_impl<T, remove_qualifiers_t<Ts>...>
            {
            };
            
            /// <summary>
            /// Checks whether an instance of the tempalte T is contained in the list of types.
            /// </summary>
            template <template<typename...> typename T, typename... Ts>
            static constexpr bool contains_instance_v = contains_instance<T, Ts...>::value;

            /// <summary>
            /// Checks whether a type deriving from the type T is contained in the list of types.
            /// </summary>
            template <typename T, typename... Ts>
            struct contains_base : detail::contains_base_impl<remove_qualifiers_t<T>, remove_qualifiers_t<Ts>...>
            {
            };
            
            /// <summary>
            /// Checks whether a type deriving from the type T is contained in the list of types.
            /// </summary>
            template <typename T, typename... Ts>
            static constexpr bool contains_base_v = contains_base<T, Ts...>::value;

        } // namespace trait

        namespace util
        {
            namespace detail
            {
                template <template<typename...> typename T, ptrdiff_t N, typename... Ts>
                constexpr ptrdiff_t index_of_template()
                {
                    if constexpr (sizeof...(Ts) <= N)
                    {
                        return -1;
                    }
                    else if constexpr (trait::is_instance_of_v<T, trait::get_t<N, Ts...>>)
                    {
                        return N;
                    }
                    else 
                    {
                        return index_of_template<T, N + 1, Ts...>();
                    }
                }

                template <template<typename...> typename T, typename... Ts>
                constexpr ptrdiff_t index_of_template()
                {
                    if (!(... || trait::is_instance_of_v<T, Ts>)) return -1;
                    return index_of_template<T, 0, Ts...>();
                }

                template <typename T>
                struct index_of_test {};

                static_assert(!trait::contains_instance_v<index_of_test, std::tuple<int>>, "Error!");
                static_assert(index_of_template<index_of_test, int>() == -1, "Error!");
                static_assert(index_of_template<index_of_test, index_of_test<int>>() == 0, "Error!");
                static_assert(index_of_template<index_of_test, int, index_of_test<int>>() == 1, "Error!");
            }

            /// <summary>
            /// A synonym for std::get<T>(tuple).
            /// </summary>
            template <typename T, typename... Ts>
            constexpr const auto& get(const std::tuple<Ts...>& ts)
            {
                return std::get<T>(ts);
            }

            /// <summary>
            /// Returns the value of type U, where U is a template instance of T.
            /// </summary>
            template <template<typename...> typename T, typename... Ts>
            constexpr const auto& get_instance(const std::tuple<Ts...>& ts)
            {
                static_assert((... || trait::is_instance_of_v<T, Ts>), "The tuple does not contain a type that is a template instance of T!");
                constexpr size_t idx = static_cast<size_t>(detail::index_of_template<T, Ts...>());
                return std::get<idx>(ts);
            }

        } // namespace util

        namespace attr
        {
            /// <summary>
            /// Used to decorate a member that serves as a property. 
            /// Takes an optional friendly name.
            /// </summary>
            struct property : public usage::field, public usage::function
            {
                const std::optional<const char*> friendly_name{};

                constexpr property() = default;

                constexpr property(const char* friendly_name)
                    : friendly_name(friendly_name)
                {
                }
            };

            /// <summary>
            /// Used to specify how a type should be displayed in debugging contexts.
            /// </summary>
            template <typename F>
            struct debug : public usage::any
            {
                const F write;

                constexpr debug(F write)
                    : write(write)
                {
                }
            };

            /// <summary>
            /// Used to specify the base types of the target type.
            /// </summary>
            template <typename... Ts>
            struct base_types : usage::type
            {
                typedef type_list<Ts...> list_type;
                static constexpr list_type list{ };
            };

            /// <summary>
            /// Used to specify the base types of the target type.
            /// </summary>
            template <typename... Ts>
            static constexpr base_types<Ts...> bases{ };

        } // namespace attr

        namespace detail::macro_exports
        {
            using attr::property;
            using attr::debug;
            using attr::bases;
        }

        namespace trait
        {
            /// <summary>
            /// Checks whether T is marked as a property.
            /// </summary>
            template <typename T>
            struct is_property : std::bool_constant<
                (trait::is_field_v<T> || trait::is_function_v<T>) 
                    && trait::contains_v<attr::property, std::remove_cv_t<decltype(T::attributes)>>> 
            {
            };

            /// <summary>
            /// Checks whether T is marked as a property.
            /// </summary>
            template <typename T>
            static constexpr bool is_property_v{ is_property<T>::value };
        }

        namespace descriptor
        {
            /// <summary>
            /// Checks whether T is a field descriptor. 
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
            template <typename T>
            constexpr bool is_field(T&& t)
            {
                return trait::is_field_v<std::remove_reference_t<T>>;
            }
            
            /// <summary>
            /// Checks whether T is a function descriptor. 
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
            template <typename T>
            constexpr bool is_function(T&& t)
            {
                return trait::is_function_v<std::remove_reference_t<T>>;
            }

            /// <summary>
            /// Checks whether T has an attribute of type A. 
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
            template <typename A, typename T>
            constexpr bool has_attribute(T&& t)
            {
                return trait::contains_base_v<A, std::remove_cv_t<decltype(t.attributes)>>;
            }
            
            /// <summary>
            /// Checks whether T has an attribute of that is a template instance of A. 
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
            template <template<typename...> typename A, typename T>
            constexpr bool has_attribute(T&& t)
            {
                return trait::contains_instance_v<A, std::remove_cv_t<decltype(t.attributes)>>;
            }
            
            /// <summary>
            /// Checks whether T is a property descriptor. 
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
            template <typename T>
            constexpr bool is_property(T&& t)
            {
                return has_attribute<attr::property>(t);
            }
            
            /// <summary>
            /// Returns the value of the attribute A on T. 
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
            template <typename A, typename T>
            constexpr const A& get_attribute(T&& t)
            {
                return util::get<A>(t.attributes);
            }
            
            /// <summary>
            /// Returns the value of the attribute A on T. 
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
            template <template<typename...> typename A, typename T>
            constexpr const auto& get_attribute(T&& t)
            {
                return util::get_instance<A>(t.attributes);
            }

            /// <summary>
            /// Returns the debug name of T. (In the form of 'declaring_type::member_name').
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
            template <typename T>
			const char* get_debug_name(T&& t)
			{
                static const std::string name(std::string(t.declarator.name) + "::" + t.name.str());
				return name.c_str();
			}

            /// <summary>
            /// Returns the display name of T. 
            /// (Uses the friendly_name of the property attribute or falls back to the in-code name). 
            /// (Tip: Take advantage ADL-lookup whenever possible.)
            /// </summary>
			template <typename T>
			const char* get_display_name(T&& t)
			{
				if constexpr (trait::is_property_v<std::remove_reference_t<T>>) {
					auto&& friendly_name = util::get<attr::property>(t.attributes).friendly_name;
					return friendly_name ? *friendly_name : t.name;
				}
				return t.name;
			}

            /// <summary>
            /// Creates a function object that dispatches to the approprite invocation function of T.
            /// </summary>
			template <typename T>
			constexpr auto make_invoker(T&& t)
			{
				using descriptor_type = std::remove_reference_t<T>;
				return [](auto&&... args) -> decltype(auto) {
                    static_assert(is_field(descriptor_type{}) || is_function(descriptor_type{}), "Invalid field or function descriptor!");
                    return descriptor_type{}(std::forward<decltype(args)>(args)...);
                };
			}

        } // namespace descriptor

#ifndef REFL_DETAIL_FORCE_EBO
#ifdef _MSC_VER
#define REFL_DETAIL_FORCE_EBO __declspec(empty_bases)
#else
#define REFL_DETAIL_FORCE_EBO 
#endif
#endif

        namespace runtime
        {

            namespace detail
            {
                template <typename T>
                struct get_member_info;
                
                template <typename T, size_t N>
                struct get_member_info<refl::function_descriptor<T, N>>
                {
                    using type = typename refl_impl::metadata::type_info__<T>::template member<N>;
                };
                
                template <typename T, size_t N>
                struct get_member_info<refl::field_descriptor<T, N>>
                {
                    using type = typename refl_impl::metadata::type_info__<T>::template member<N>;
                };

                /// <summary>
                /// Implements a proxy for a reflected function.
                /// </summary>
                template <typename Derived, typename Func>
                struct REFL_DETAIL_FORCE_EBO function_proxy : public get_member_info<Func>::type::template remap<function_proxy<Derived, Func>>
                {
                    function_proxy()
                    {
                    }

                    template <typename... Args>
                    decltype(auto) call_impl(Args&& ... args)
                    {
                        return static_cast<Derived*>(this)->template call_impl<Func>(std::forward<Args>(args)...);
                    }
                    
                    template <typename... Args>
                    decltype(auto) call_impl(Args&& ... args) const
                    {
                        return static_cast<const Derived*>(this)->template call_impl<Func>(std::forward<Args>(args)...);
                    }

                };

                template <typename, typename>
                struct REFL_DETAIL_FORCE_EBO function_proxies;

                /// <summary>
                /// Implements a proxy for all reflected functions.
                /// </summary>
                template <typename Derived, typename... Members>
                struct REFL_DETAIL_FORCE_EBO function_proxies<Derived, type_list<Members...>> : public function_proxy<Derived, Members>...
                {
                };

				template <typename T>
				using functions = trait::filter<trait::is_function, member_list<std::remove_reference_t<T>>>;

            } // namespace detail
            
            /// <summary>
            /// A proxy object that has a static interface identical to the reflected functions of type T.
            /// Users should inherit from this class and specify a call_impl(Member member, Args&&... args) function.
            /// </summary>
            template <typename Derived, typename Target>
            struct REFL_DETAIL_FORCE_EBO proxy : public detail::function_proxies<proxy<Derived, Target>, detail::functions<Target>>
            {
                static_assert(
                    sizeof(detail::function_proxies<proxy<Derived, Target>, detail::functions<Target>>) == 1,
                    "Multiple inheritance EBO did not kick in! "
                    "You could try defining the REFL_DETAIL_FORCE_EBO macro appropriately to enable it on the required types. "
                    "Default for MSC is `__declspec(empty_bases)`.");

                constexpr proxy() = default;

            private:

                template <typename P, typename F>
                friend struct detail::function_proxy;

                // Called by one of the function_proxy bases.
                template <typename Member, typename... Args>
                decltype(auto) call_impl(Args&&... args)
                {
                    static_assert(std::is_base_of_v<proxy, Derived>);
                    static_cast<Derived*>(this)->call_impl(Member{}, std::forward<Args>(args)...);
                }

                // Called by one of the function_proxy bases.
                template <typename Member, typename... Args>
                decltype(auto) call_impl(Args&&... args) const
                {
                    static_assert(std::is_base_of_v<proxy, Derived>);
                    static_cast<const Derived*>(this)->call_impl(Member{}, std::forward<Args>(args)...);
                }

            };

            /// <summary>
            /// Writes the debug representation of value to the given std::ostream.
            /// </summary>
            template <typename T>
            void debug(std::ostream& os, const T& value, bool compact = false)
            {
				static_assert(trait::is_reflectable_v<T> || trait::is_container_v<T>, "Type is neither reflectable nor a container of reflectable types!");

				if constexpr (trait::is_reflectable_v<T>) {
				    typedef type_descriptor<T> type_descriptor;
					if constexpr (trait::contains_instance_v<attr::debug, decltype(type_descriptor::attributes)>) {
						auto debug = util::get_instance<attr::debug>(type_descriptor::attributes);
						debug.write(os, value);
					}
					else if constexpr (std::is_fundamental_v<T>) {
						std::ios_base::fmtflags old_flags{ os.flags() };
						os << std::boolalpha << value;
						os.flags(old_flags);
					}
					else if constexpr (std::is_pointer_v<T>) {
						std::ios_base::fmtflags old_flags{ os.flags() };
						os << "0x" << std::hex << value;
						os.flags(old_flags);
					}
					else {
						os << "{ ";
						if (!compact) os << "\n";
                        constexpr size_t count = count_if(type_descriptor::members, [](auto member) { return is_field(member) || is_property(member); });
						for_each(type_descriptor::members, [&](auto member, auto index) {
							if constexpr (is_field(member) || is_property(member)) {
								std::string name{ get_display_name(member) };

								if (!compact) os << "  ";
								os << name << " = ";

								if constexpr (trait::contains_instance_v<attr::debug, decltype(member.attributes)>) {
									auto&& dbg_attr = util::get_instance<attr::debug>(member.attributes);
                                    auto prop_value = member(value);
                                    if (!compact) os << "(" << reflect(prop_value).name << ")";
									dbg_attr.write(os, prop_value);
								}
								else {
                                    auto prop_value = member(value);
                                    if (!compact) os << "(" << reflect(prop_value).name << ")";
									debug(os, prop_value, true);
								}
                                if (index + 1 != count) {
								    os << ", ";
                                }
								if (!compact) os << "\n";
							}
						});

						if (compact) os << " }";
						else os << "}";
					}
				}
				else { // T supports begin() and end()
					os << "[";
					auto end = value.end();
					for (auto it = value.begin(); it != end; ++it)
					{
						debug(os, *it, true);
						if (std::next(it, 1) != end)
						{
							os << ", ";
						}
					}
					os << "]";
				}
            }

            template <typename T>
            std::string debug_str(const T& value, bool compact = false)
            {
                std::stringstream ss;
                debug(ss, value, compact);
                return ss.str();
            }

            /// <summary>
            /// Invokes the specified member. 
            /// When used with a member that is a field, the function gets or sets the value of the field.
            /// </summary>
            template <typename U, typename T, typename... Args>
            U invoke(T&& target, const char* name, Args&&... args)
            {
				using type = std::remove_reference_t<T>;
                static_assert(refl::trait::is_reflectable_v<type>, "Unsupported type!");
                typedef type_descriptor<type> type_descriptor;
                
                std::optional<U> result;

				bool found{ false };
				for_each(type_descriptor::members, [&](auto member) {
					if (found) return;

					constexpr auto invoker = make_invoker(member);
					using invoker_type = decltype(invoker);

					if constexpr (std::is_invocable_r_v<U, invoker_type, T, Args...>(member)) {
						if (std::strcmp(member.name, name) == 0) {
							result.emplace(invoker(target, std::forward<Args>(args)...));
							found = true;
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

#define REFL_DEFINE_PRIMITIVE(TypeName) \
    REFL_TYPE(TypeName) \
    REFL_END

    // Char types.
    REFL_DEFINE_PRIMITIVE(char);
    REFL_DEFINE_PRIMITIVE(wchar_t);
    REFL_DEFINE_PRIMITIVE(char16_t);
    REFL_DEFINE_PRIMITIVE(char32_t);
#ifdef __cpp_lib_char8_t
    REFL_DEFINE_PRIMITIVE(char8_t);
#endif 

    // Integral types.
    REFL_DEFINE_PRIMITIVE(bool);
    REFL_DEFINE_PRIMITIVE(signed char);
    REFL_DEFINE_PRIMITIVE(unsigned char);
    REFL_DEFINE_PRIMITIVE(signed short);
    REFL_DEFINE_PRIMITIVE(unsigned short);
    REFL_DEFINE_PRIMITIVE(signed int);
    REFL_DEFINE_PRIMITIVE(unsigned int);
    REFL_DEFINE_PRIMITIVE(signed long);
    REFL_DEFINE_PRIMITIVE(unsigned long);
    REFL_DEFINE_PRIMITIVE(signed long long);
    REFL_DEFINE_PRIMITIVE(unsigned long long);
    
    // Floating point types.
    REFL_DEFINE_PRIMITIVE(float);
    REFL_DEFINE_PRIMITIVE(double);
    REFL_DEFINE_PRIMITIVE(long double);

    // Other types.
    REFL_DEFINE_PRIMITIVE(decltype(nullptr));

    // Void type.
    REFL_TYPE(void)
    REFL_END

#define REFL_DEFINE_POINTER(Ptr) \
        template<typename T> \
        struct type_info__<T Ptr> { \
            typedef T Ptr type; \
            template <size_t N> \
            struct member {}; \
            static constexpr char name[] { #Ptr }; \
            static constexpr size_t member_count = 0; \
        }

    namespace refl_impl::metadata
    {
        REFL_DEFINE_POINTER(*);
        REFL_DEFINE_POINTER(&);
        REFL_DEFINE_POINTER(&&);
    }

#undef REFL_DEFINE_POINTER

    static_assert(refl::trait::is_reflectable_v<void>, "Static assertion failed!");
    static_assert(refl::trait::is_reflectable_v<int*>, "Static assertion failed!");
    static_assert(refl::trait::is_reflectable_v<int&>, "Static assertion failed!");
    static_assert(refl::trait::is_reflectable_v<int&&>, "Static assertion failed!");

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

namespace refl::detail
{
    template <typename T>
    auto write_impl(std::ostream& os, T&& t) -> decltype((os << t), void())
    {
        os << t;
    }

	template <typename T>
	void write_impl(std::ostream& os, const volatile T* ptr)
	{
		auto f(os.flags());
		os << "(" << reflect<T>().name << "*)" << std::hex << ptr;
		os.flags(f);
	}

	inline void write_impl(std::ostream& os, const char* ptr)
	{
		os << ptr;
	}

    template <typename Tuple, size_t... Idx>
    void write_impl(std::ostream& os, Tuple&& t, std::index_sequence<Idx...>)
    {
        ignore((os << std::get<Idx>(t))...);
    }

	inline void write_impl(std::ostream& os, const std::exception& e)
	{
		os << "Exception";
#ifdef REFL_RTTI_ENABLED
		os << " (" << typeid(e).name() << ")";
#endif
		os << ": `" << e.what() << "`";
	}

	inline void write_impl(std::ostream& os, const std::string& t)
	{
		os << std::quoted(t);
	}

	inline void write_impl(std::ostream& os, const std::wstring& t)
	{
#ifdef _MSC_VER
// Disable the "wcsrtombs is unsafe" warning in VS
#pragma warning(push)
#pragma warning(disable:4996)
#endif
		std::mbstate_t state = std::mbstate_t();
		const wchar_t* wsptr = t.c_str();
		std::size_t len = 1 + std::wcsrtombs(nullptr, &wsptr, 0, &state);

		std::string mbstr(len, '\0');
		std::wcsrtombs(mbstr.data(), &wsptr, mbstr.size(), &state);

		os << std::quoted(mbstr);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
	}

    template <typename... Ts>
    void write_impl(std::ostream& os, const std::tuple<Ts...>& t)
    {
        write_impl(os, t, std::make_index_sequence<sizeof...(Ts)>{});
    }

	template <typename K, typename V>
	void write_impl(std::ostream& os, const std::pair<K, V>& t);

	// Dispatches to the appropriate write_impl.
	constexpr auto write = [](std::ostream & os, auto && t) -> void
	{
		write_impl(os, t);
	};

    template <typename K, typename V>
    void write_impl(std::ostream& os, const std::pair<K, V>& t)
    {
        os << "(";
        write(os, t.first);
        os << ", ";
        write(os, t.second);
        os << ")";
    }
} // namespace refl::detail

// Custom reflection information for 
// some common built-in types (std::basic_string, std::tuple, std::pair).

#ifndef REFL_NO_STD_SUPPORT

REFL_TYPE(std::exception, debug{ refl::detail::write })
	REFL_FUNC(what, property{ })
REFL_END

REFL_TEMPLATE(
    (typename Elem, typename Traits, typename Alloc), 
    (std::basic_string<Elem, Traits, Alloc>), 
    debug{ refl::detail::write })
    REFL_FUNC(size, property{ })
    REFL_FUNC(data, property{ })
REFL_END

REFL_TEMPLATE(
    (typename... Ts),
    (std::tuple<Ts...>),
    debug{ refl::detail::write })
REFL_END

REFL_TEMPLATE(
    (typename K, typename V),
    (std::pair<K, V>),
    debug{ refl::detail::write })
REFL_END

#endif // REFL_NO_STD_SUPPORT

#endif // REFL_INCLUDE_HPP
