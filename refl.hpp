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

#ifdef _MSC_VER
// Disable VS warning for "Not enough arguments for macro"
// (emitted when a REFL_ macro is not provided any attributes)
#pragma warning( disable : 4003 )
#endif

namespace refl
{
	template <typename T>
	struct Members;

	/// <summary>
	/// The core reflection info type. 
	/// </summary>
	template <typename T>
	struct TypeInfo
	{
		/// <summary>
		/// The type 'T' of this 'TypeInfo' instance.
		/// </summary>
		typedef T Type;

		/// <summary>
		/// Specializations of this type from 0 to TypeInfo::MemberCount provide additional reflection information.
		/// </summary>
		template <size_t N>
		struct Member {};

		/// <summary>
		/// Utility typedef for refl::Members.
		/// </summary>
		typedef refl::Members<T> Members;

		/// <summary>
		/// States whether the type 'T' has any reflection information available. 
		/// </summary>
		static constexpr bool is_valid = false;

		/// <summary>
		/// Contains any additional metadata associated with 'T'.
		/// </summary>
		static constexpr std::tuple<> attributes = {};

		/// <summary>
		/// The declaration name of the reflected type.
		/// </summary>
		static constexpr char name[] = "?";

		/// <summary>
		/// The number of reflectable members of 'T'.
		/// </summary>
		static constexpr size_t member_count = 0;
	};

	// CV qualifiers are not taken into account when reflecting on a type.
	template <typename T>
	struct TypeInfo<const T> : public TypeInfo<T> {};

	template <typename T>
	struct TypeInfo<volatile T> : public TypeInfo<T> {};

	template <typename T>
	struct TypeInfo<const volatile T> : public TypeInfo<T> {};

	namespace trait
	{
		/// <summary>
		/// Exposes a typedef `type` that is the element type of T.
		/// </summary>
		template <typename T>
		struct ElementType;

		template <typename T>
		struct ElementType<T&> { typedef T type; };

		template <typename T>
		struct ElementType<T&&> { typedef T type; };

		template <typename T>
		struct ElementType<T*> { typedef T type; };

		template <typename T, size_t N>
		struct ElementType<T[N]> { typedef T type; };

		template <typename T, typename Dx>
		struct ElementType<std::unique_ptr<T, Dx>> { typedef T type; };

		template <typename T>
		struct ElementType<std::shared_ptr<T>> { typedef T type; };

		template <typename T>
		struct ElementType<std::weak_ptr<T>> { typedef T type; };

		/// <summary>
		/// Exposes a const `value` that is equal to the statically-defined number of elements of T.
		/// </summary>
		template <typename T>
		struct ArraySize;

		template <typename T, size_t N>
		struct ArraySize<std::array<T, N>> { static constexpr size_t value = N; };

		template <typename T, size_t N>
		struct ArraySize<T[N]> { static constexpr size_t value = N; };

		template <typename T, size_t N, typename Dx>
		struct ArraySize<std::unique_ptr<T[N], Dx>> { static constexpr size_t value = N; };

		template <typename T, size_t N>
		struct ArraySize<std::shared_ptr<T[N]>> { static constexpr size_t value = N; };

		template <typename T, size_t N>
		struct ArraySize<std::weak_ptr<T[N]>> { static constexpr size_t value = N; };

		/// <summary>
		/// Exposes a const `value` that is true if T supports begin() and end() member functions.
		/// </summary>
		template <typename T>
		struct IsContainer
		{
		private:
			template <typename U>
			static auto test(int) -> decltype(std::declval<U>().begin(), std::declval<U>().end(), std::true_type{});

			template <typename U>
			static std::false_type test(...);
		public:
			static constexpr bool value = decltype(test<T>(0))::value;
		};

	} // namespace trait

	/// <summary>
	/// An enumeration of the possible member types.
	/// </summary>
	namespace member
	{
		/// <summary>
		/// Represents a field.
		/// </summary>
		struct Field {};

		/// <summary>
		/// Represents a function.
		/// </summary>
		struct Function {};
	}

	namespace util
	{
		/// <summary>
		/// True if T::pointer is not a member pointer.
		/// </summary>
		template <typename T>
		static constexpr bool is_static = !std::is_member_pointer_v<decltype(T::pointer)>;

		/// <summary>
		/// Returns the value of the reflected field `T`.
		/// </summary>
		template <typename T>
		decltype(auto) get_value() { return *T::pointer; }

		/// <summary>
		/// Returns the value of the reflected field `T`. 
		/// </summary>
		template <typename T, typename Self>
		decltype(auto) get_value(Self&& self) { return self.*(T::pointer); }
	}

	namespace detail
	{
		/// <summary>
		/// Exposes a typedef `type` that represents the N-th (0-based index) type in Ts...
		/// </summary>
		template <size_t N, typename... Ts>
		struct Get;

		template <size_t N, typename T, typename... Ts>
		struct Get<N, T, Ts...> : public Get<N - 1, Ts...>
		{
		};

		template <typename T, typename... Ts>
		struct Get<0, T, Ts...>
		{
			typedef T type;
		};

		static_assert(std::is_same_v<Get<0, int>::type, int>, "Error!");
		static_assert(std::is_same_v<Get<1, int, float>::type, float>, "Error!");
	}

	/// <summary>
	/// Represents a list of static types (implemented as variadic template parameters).
	/// </summary>
	template <typename... Ts>
	struct TypeList
	{
		/// <summary>
		/// Returns the type at position N.
		/// </summary>
		template <size_t N>
		using Get = typename detail::Get<N, Ts...>::type;

		/// <summary>
		/// Represents the number of types in this TypeList.
		/// </summary>
		static constexpr size_t size = sizeof...(Ts);
	};

	namespace trait
	{
		/// <summary>
		/// A synonym for std::is_invocable_v&lt;F, Args...&gt;.
		/// </summary>
		template <typename F, typename... Args>
		static constexpr bool can_invoke = std::is_invocable_v<F, Args...>;
	}

	namespace attr
	{
		namespace usage
		{
			/// <summary>
			/// Specifies that an attribute type inheriting from this type can be used 
			/// with all three of REFL_FUNC, REFL_FIELD, REFL_FUNC.
			/// </summary>
			struct Any {};

			/// <summary>
			/// Specifies that an attribute type inheriting from this type can only be 
			/// used with REFL_FUNC(...).
			/// </summary>
			struct Type : public Any {};

			/// <summary>
			/// Specifies that an attribute type inheriting from this type can only be 
			/// used with either REFL_FUNC(...) or REFL_FIELD(...).
			/// </summary>
			struct Member : public Any {};

			/// <summary>
			/// Specifies that an attribute type inheriting from this type can only be 
			/// used with REFL_FUNC(...).
			/// </summary>
			struct Function : public Member {};

			/// <summary>
			/// Specifies that an attribute type inheriting from this type can only be 
			/// used with REFL_FIELD(...).
			/// </summary>
			struct Field : public Member {};
		}

		namespace detail
		{
			// Used to support a variable number of arguments. 
			static constexpr struct {

				template <typename... Args>
				constexpr std::tuple<Args...> operator()(Args&& ... args) const noexcept
				{
					if constexpr (sizeof...(Args) == 0)
					{
						return std::tuple<>{};
					}
					else
					{
						return std::make_tuple(std::forward<Args>(args)...);
					}
				}

				template <typename... Args>
				constexpr std::tuple<Args...> operator()(const std::tuple<Args...>& args) const noexcept
				{
					return args;
				}

			} make_attrs_helper;

			template <typename... Args>
			constexpr std::tuple<Args...> lift_attrs(const std::tuple<Args...>& t) noexcept
			{
				return t;
			}

			constexpr std::tuple<> lift_attrs(const decltype(make_attrs_helper)&) noexcept
			{
				return std::tuple<>{};
			}

			constexpr bool validate_unique(TypeList<>) 
			{ 
				return true; 
			}

			/// <summary>
			/// Statically asserts that all arguments types in Ts... are unique.
			/// </summary>
			template <typename T, typename... Ts>
			constexpr bool validate_unique(TypeList<T, Ts...>)
			{
				constexpr bool cond = (... && (!std::is_same_v<T, Ts> && validate_unique(TypeList<Ts>{})));
				static_assert(cond, "Some of the attributes provided have duplicate types!");
				return cond;
			}

			template <typename Usage, typename... Ts>
			constexpr bool validate_usage(const std::tuple<Ts...>&)
			{
				validate_unique(TypeList<Ts...>{});
				return (... && (!std::is_base_of_v<usage::Any, Ts> || std::is_base_of_v<Usage, Ts>));
			}
		}
	} // namespace attr

	namespace detail
	{
		template <typename Self, typename Return, typename... Args>
		constexpr auto resolve(Return(*fn)(Args...))
		{
			return fn;
		}

		template <typename Self, typename Return, typename... Args>
		constexpr auto resolve(Return(Self::*fn)(Args...))
		{
			return fn;
		}
	}

#define REFL_STRINGIFY(...) #__VA_ARGS__
#define REFL_EXPAND(...) __VA_ARGS__
#define REFL_EXPAND_FIRST(A, ...) A
#define REFL_EXPAND_SECOND(A, B, ...) B 

#define REFL_DETAIL_ATTRIBUTES(DeclType, ...) \
	public: \
		static constexpr auto attributes = ::refl::attr::detail::lift_attrs(::refl::attr::detail::make_attrs_helper __VA_ARGS__); \
	private: /* GCC chokes without valid_usage being a constexpr variable. */ \
		static constexpr bool valid_usage = ::refl::attr::detail::validate_usage<::refl::attr::usage:: DeclType>(attributes); \
		static_assert(valid_usage, "Some of the supplied attributes cannot be used on a " #DeclType " declaration!"); \
	public: \

#define REFL_DETAIL_TYPE_BODY(TypeName, ...) \
		friend struct ::refl::Members<TypeName>; \
		typedef ::refl::Members<TypeName> Members; \
		typedef TypeName Type; \
		static constexpr bool is_valid = true; \
		REFL_DETAIL_ATTRIBUTES(Type, REFL_EXPAND_FIRST(__VA_ARGS__)) \
	public: \
		static constexpr char name[] = REFL_STRINGIFY(TypeName); \
	private: \
		static constexpr size_t member_index_offset = __COUNTER__ + 1; \
		template <size_t N, typename = void> \
		struct Member {}; 

/// <summary>
/// Creates reflection information for a specified type. Takes an optional (*brace-enclosed*) attribute list. 
/// </summary>
#define REFL_TYPE(TypeName, ...) \
	namespace refl { template<> struct TypeInfo<TypeName> { \
		REFL_DETAIL_TYPE_BODY(REFL_EXPAND(TypeName), __VA_ARGS__)

#define REFL_TEMPLATE_TYPE(TemplateDeclaration, TypeName, ...) \
	namespace refl { template <REFL_EXPAND TemplateDeclaration> struct TypeInfo<TypeName> { \
		REFL_DETAIL_TYPE_BODY(REFL_EXPAND(TypeName), __VA_ARGS__)

#define REFL_END \
		static constexpr size_t member_count = __COUNTER__ - member_index_offset; \
	}; }


#define REFL_DETAIL_MEMBER_HEADER template<typename Unused__> struct Member<__COUNTER__ - member_index_offset, Unused__>

#define REFL_DETAIL_MEMBER_COMMON(MemberTy, MemberName, Attrs) \
		typedef ::refl::TypeInfo<Type> DeclaringType; \
		typedef ::refl::member::MemberTy MemberType; \
		static constexpr char name[] = REFL_STRINGIFY(MemberName); \
		REFL_DETAIL_ATTRIBUTES(MemberTy, (Attrs)) 


/// <summary>
/// Creates reflection information for a public field. Takes an optional (*brace-enclosed*) attribute list. 
/// </summary>
#define REFL_FIELD(FieldName, ...) \
	REFL_DETAIL_MEMBER_HEADER { \
		REFL_DETAIL_MEMBER_COMMON(Field, FieldName, REFL_EXPAND_FIRST(__VA_ARGS__)) \
		typedef decltype(Type::FieldName) ValueType; \
	public: \
		static constexpr auto pointer = &Type::FieldName; \
	};

/// <summary>
/// Creates reflection information for a public function. Takes an optional *brace-enclosed* attribute list. 
/// </summary>
#define REFL_FUNC(FunctionName, ...) \
	REFL_DETAIL_MEMBER_HEADER { \
		REFL_DETAIL_MEMBER_COMMON(Function, FunctionName, REFL_EXPAND_FIRST(__VA_ARGS__)) \
	private: \
		template<typename Self, typename... Args> static constexpr auto invoke_impl(Self&& self, Args&&... args) -> decltype(auto) { return self.FunctionName(::std::forward<Args>(args)...); } \
		template<typename Self, typename... Args> static constexpr auto invoke_impl(Args&&... args) -> decltype(auto) { return Self::FunctionName(::std::forward<Args>(args)...); } \
		template<typename... Args> static constexpr auto invoke_impl(Args&&... args) -> decltype(auto) { return invoke_impl<Type>(::std::forward<Args>(args)...); } \
	public: \
		static constexpr struct { template<typename... Args> constexpr auto operator()(Args&&... args) const -> decltype(auto) { return invoke_impl(::std::forward<decltype(args)>(args)...); } } invoke {}; \
		/* 
			There can be a total of 12 differently qualified member functions with the same name. 
			Providing Remaps for non-const and const-only strikes a balance between compilation time and usability.
			And even though there are many other Remap implementation possibilities (like virtual, field variants),
			adding them is considered to be non-efficient for the compiler.
		*/ \
		template <typename Proxy> struct Remap { \
			template <typename... Args> decltype(auto) FunctionName(Args&&... args) { \
				static_assert(::std::is_base_of_v<Remap<Proxy>, Proxy>, "Incorrect usage or Remap<Proxy>! Proxy must derive from Remap<Proxy>."); \
				return (reinterpret_cast<Proxy*>(this))->trap(::std::forward<Args>(args)...); \
			} \
		}; \
	};

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

	namespace refl
	{

#define REFL_DEFINE_POINTER(Ptr) \
		template<typename T> \
		struct TypeInfo<T Ptr> { \
			typedef T Ptr Type; \
			template <size_t N> \
			struct Member {}; \
			typedef ::refl::Members<T*> Members; \
			static constexpr bool is_valid = true; \
			static constexpr ::std::tuple<> attributes = {}; \
			static constexpr char name[] = #Ptr; \
			static constexpr size_t member_count = 0; \
		}

		REFL_DEFINE_POINTER(*);
		REFL_DEFINE_POINTER(&);
		REFL_DEFINE_POINTER(&&);

#undef REFL_DEFINE_POINTER

		static_assert(TypeInfo<void>::is_valid, "Static assertion failed!");
		static_assert(TypeInfo<int*>::is_valid, "Static assertion failed!");
		static_assert(TypeInfo<int&>::is_valid, "Static assertion failed!");
		static_assert(TypeInfo<int&&>::is_valid, "Static assertion failed!");

		namespace util
		{
			/// <summary>
			/// Can serve as a placeholder for any type in unevaluated contexts.
			/// </summary>
			struct Placeholder
			{
				template <typename T>
				operator T() const;
			};

			/// <summary>
			/// Ignores all arguments.
			/// </summary>
			constexpr void ignore(...) noexcept {}
		}

		namespace typelist
		{
			namespace detail
			{
				template <typename, typename>
				struct Cons;

				template <typename T, typename... Args>
				struct Cons<T, TypeList<Args...>>
				{
					using Type = TypeList<T, Args...>;
				};

				template <template<typename> typename, typename...>
				struct ApplyImpl;

				template <template<typename> typename Predicate>
				struct ApplyImpl<Predicate>
				{
					using Type = TypeList<>;
				};

				template <template<typename> typename Predicate, typename Head, typename ...Tail>
				struct ApplyImpl<Predicate, Head, Tail...>
				{
					using Type = std::conditional_t<Predicate<Head>::value,
						typename Cons<Head, typename ApplyImpl<Predicate, Tail...>::Type>::Type,
						typename ApplyImpl<Predicate, Tail...>::Type
					>;
				};

				template <template<typename> typename Predicate, typename... Ts>
				struct ApplyImpl<Predicate, TypeList<Ts...>> : public ApplyImpl<Predicate, Ts...>
				{
				};

				/// <summary>
				/// Filters a TypeList according to a Predicate (a type-trait-like template type).
				/// </summary>
				template <template<typename> typename Predicate>
				struct Filter
				{
					template <typename... Ts>
					using Apply = typename detail::ApplyImpl<Predicate, Ts...>::Type;
				};
			}

			/// <summary>
			/// Filters a TypeList according to a predicate template.
			/// </summary>
			template <template<typename> typename Predicate, typename Ts>
			using Filter = typename detail::Filter<Predicate>::template Apply<Ts>;

			namespace detail
			{
				template <typename F, typename T>
				decltype(auto) dispatch_visitor(F&& f, T&& value, size_t idx)
				{
					if constexpr (trait::can_invoke<F, util::Placeholder, size_t>) {
						return f(value, idx);
					}
					else if constexpr (trait::can_invoke<F, util::Placeholder>) {
						return f(value);
					}
					else {
						static_assert(trait::can_invoke<F, util::Placeholder>, "Invalid function!");
					}
				}
			}

			/// <summary>
			/// Applies function F to each type in the TypeList.
			/// Stops when F returns false. F can optionally take an index of type size_t.
			/// </summary>
			template <typename F, typename... Ts>
			constexpr bool for_each(TypeList<Ts...>, F&& f)
			{
				size_t idx = 0;
				return (... || !detail::dispatch_visitor(f, Ts{}, idx++));
			}

			/// <summary>
			/// Counts the number of times the predicate F returns true.
			/// </summary>
			template <typename F, typename... Ts>
			constexpr size_t count_if(TypeList<Ts...>, F&& f)
			{
				size_t idx = 0;
				util::ignore((f(Ts{}) ? idx++ : 0)...);
				return idx;
			}

			/// <summary>
			/// Applies function F to each type in the TypeList, aggregating
			/// the results in a tuple.
			/// </summary>
			template <typename F, typename... Ts>
			constexpr auto map_to_tuple(TypeList<Ts...>, F&& f)
			{
				return std::make_tuple(f(Ts{})...);
			}

			/// <summary>
			/// Applies function F to each type in the TypeList, aggregating
			/// the results in an array.
			/// </summary>
			template <typename F, typename... Ts>
			constexpr auto map_to_array(TypeList<Ts...>, F&& f)
			{
				return make_array(f(Ts{})...);
			}
		} // namespace typelist

		namespace trait
		{
			/// <summary>
			/// A trait for detecting whether the type 'T' is a field info type.
			/// </summary>
			template <typename T>
			using IsField = std::is_base_of<typename T::MemberType, member::Field>;

			/// <summary>
			/// A trait for detecting whether the type 'T' is a function info type.
			/// </summary>
			template <typename T>
			using IsFunction = std::is_base_of<typename T::MemberType, member::Function>;

			/// <summary>
			/// A trait for detecting whether the type 'T' is a static field info type.
			/// </summary>
			template <typename T>
			using IsStatic = std::bool_constant<util::is_static<T>>;

			/// <summary>
			/// A trait for detecting whether the type 'T' is an instance field info type.
			/// </summary>
			template <typename T>
			using IsInstance = std::bool_constant<!util::is_static<T>>;
		} // namespace trait

		template <typename T>
		struct Members
		{
			// The type T that is being reflected.
			typedef TypeInfo<T> DeclaringType;

		private:

			template <size_t N>
			using Member = typename DeclaringType::template Member<N>;

			template <size_t... Idx>
			static auto enumerate_members(std::index_sequence<Idx...>) -> TypeList<Member<Idx>...>;

		public:

			typedef decltype(enumerate_members(std::make_index_sequence<DeclaringType::member_count>{})) All;

			// A TypeList of all reflected fields.
			typedef typelist::Filter<trait::IsField, All> Fields;
			// A TypeList of all reflected instance fields.
			typedef typelist::Filter<trait::IsInstance, Fields> InstanceFields;
			// A TypeList of all reflected static fields.
			typedef typelist::Filter<trait::IsStatic, Fields> StaticFields;

			// A TypeList of all reflected functions.
			typedef typelist::Filter<trait::IsFunction, All> Functions;
		};

		namespace trait
		{
			namespace detail
			{
				template <typename T>
				struct IsTemplate : public std::false_type {};

				template <template<typename...> typename T, typename... Args>
				struct IsTemplate<T<Args...>> : public std::true_type {};
			}

			/// <summary>
			/// True if T is a template specialization.
			/// </summary>
			template <typename T>
			static constexpr bool is_template = detail::IsTemplate<T>::value;

			namespace detail
			{
				template <template<typename...> typename, typename>
				struct TransferTypes;

				template <template<typename...> typename T, template<typename...> typename U, typename... Args>
				struct TransferTypes<T, U<Args...>>
				{
					typedef T<Args...> Type;
				};
			}

			/// <summary>
			/// Transfers all type parameters from U to T.
			/// </summary>
			template <template<typename...> typename T, typename U>
			using TransferTypes = typename detail::TransferTypes<T, U>::Type;

			/// <summary>
			/// Creates a TypeList by transferring U's type arguments.
			/// </summary>
			template <typename U>
			using MakeTypeList = TransferTypes<TypeList, U>;

			/// <summary>
			/// Creates an std::tuple by transferring U's type arguments.
			/// </summary>
			template <typename U>
			using MakeTuple = TransferTypes<std::tuple, U>;

			namespace detail
			{
				template <template<typename...>  typename T, typename U>
				struct IsInstanceOf : public std::false_type {};

				template <template<typename...> typename T, template<typename...> typename U, typename... Args>
				struct IsInstanceOf<T, U<Args...>> : public std::is_same<T<Args...>, U<Args...>>
				{
				};
			}

			/// <summary>
			/// True if the type U is a template specialization of U.
			/// </summary>
			template <template<typename...>typename T, typename U>
			static constexpr bool is_instance_of = detail::IsInstanceOf<T, U>::value;

			static_assert(is_template<std::unique_ptr<int>>, "Error!");
			static_assert(is_instance_of<std::unique_ptr, std::unique_ptr<float>>, "Error!");

		} // namespace trait

		namespace util
		{
			/// <summary>
			/// A combination of all of its supplied type arguments.
			/// </summary>
			template <typename... Ts>
			struct Combine : public Ts...
			{
				constexpr Combine() = default;

				constexpr Combine(Ts&& ... ts) : Ts(std::forward<Ts>(ts))...
				{
				}
			};

			/// <summary>
			/// Combines all arguments into one object. 
			/// Can be used to create overloaded function objects.
			/// </summary>
			template <typename... Ts>
			constexpr Combine<Ts...> combine(Ts&& ... ts)
			{
				return Combine<Ts...>(std::forward<Ts>(ts)...);
			}

			/// <summary>
			/// Creates an array from the provided arguments.
			/// </summary>
			template <typename... Ts>
			std::array<std::common_type_t<std::decay_t<Ts>...>, sizeof...(Ts)> make_array(Ts&& ... ts)
			{
				return { std::forward<Ts>(ts)... };
			}

			/// <summary>
			/// Creates an array of type 'T' from the provided tuple.
			/// </summary>
			template <typename T, typename... Ts>
			std::array<T, sizeof...(Ts)> to_array(const std::tuple<Ts...>& tuple)
			{
				return std::apply([](auto&& ... args) -> std::array<T, sizeof...(Ts)> { return { std::forward<decltype(args)>(args)... }; }, tuple);
			}

			/// <summary>
			/// Creates an empty array of type 'T'.
			/// </summary>
			template <typename T>
			std::array<T, 0> to_array(const std::tuple<>& tuple)
			{
				return {};
			}

			/// <summary>
			/// Creates a tuple from the provided array.
			/// </summary>
			template <typename T, size_t N>
			auto to_tuple(const std::array<T, N>& array)
			{
				return std::apply(std::make_tuple, array);
			}

			/// <summary>
			/// Returns true if the supplied std::tuple contains a type T.
			/// </summary>
			template <typename T, typename... Ts>
			constexpr bool contains(const std::tuple<Ts...>& ts)
			{
				return (... || std::is_same_v<Ts, T>);
			}

			/// <summary>
			/// Returns true if the supplied std::tuple contains a type U that is a template instance of T.
			/// </summary>
			template <template<typename...> typename T, typename... Ts>
			constexpr bool contains(const std::tuple<Ts...>& ts)
			{
				return (... || trait::is_instance_of<T, Ts>);
			}

			/// <summary>
			/// Returns true if the supplied std::tuple contains a type that derives from T.
			/// </summary>
			template <typename T, typename... Ts>
			constexpr bool contains_base(const std::tuple<Ts...>& ts)
			{
				return (... || std::is_base_of_v<Ts, T>);
			}

			namespace detail
			{
				template <template<typename...> typename T, typename... Ts>
				constexpr ptrdiff_t index_of_template()
				{
					if (!(... || trait::is_instance_of<T, Ts>)) return -1;

					bool found = false;
					ptrdiff_t idx = -1;
					// Intellisense raises an assignment of constant in bool context warning.
					ignore((... || ((!found ? ++idx : 0), (trait::is_instance_of<T, Ts> ? ((found = true)) : ((found = false))))));
					return idx;
				}

				template <typename T>
				struct IndexOfTest {};

				static_assert(!contains<IndexOfTest>(std::tuple<int>()), "Error!");
				static_assert(index_of_template<IndexOfTest, int>() == -1, "Error!");
				static_assert(index_of_template<IndexOfTest, IndexOfTest<int>>() == 0, "Error!");
				static_assert(index_of_template<IndexOfTest, int, IndexOfTest<int>>() == 1, "Error!");
			}

			/// <summary>
			/// A synonym for std::get<T>(tuple).
			/// </summary>
			template <typename T, typename... Ts>
			constexpr auto get(const std::tuple<Ts...>& ts)
			{
				return std::get<T>(ts);
			}

			/// <summary>
			/// Returns the value of type U, where U is a template instance of T.
			/// </summary>
			template <template<typename...> typename T, typename... Ts>
			constexpr auto get(const std::tuple<Ts...>& ts)
			{
				static_assert((... || trait::is_instance_of<T, Ts>), "The tuple does not contain a type that is a template instance of T!");
				constexpr size_t idx = detail::index_of_template<T, Ts...>();
				return std::get<idx>(ts);
			}

		} // namespace util

		namespace attr
		{
			/// <summary>
			/// Used to decorate a function that serves as a property. 
			/// Takes an optional friendly name.
			/// </summary>
			struct Property : public usage::Function
			{
				const std::optional<const char*> friendly_name{};

				constexpr Property() = default;

				constexpr Property(const char* friendly_name)
					: friendly_name(friendly_name)
				{
				}
			};

			/// <summary>
			/// Used to specify how a type should be displayed in debugging contexts.
			/// </summary>
			template <typename F>
			struct Debug : public usage::Type
			{
				const F write;

				constexpr Debug(F write)
					: write(write)
				{
					static_assert(std::is_same_v<void, decltype(write(std::declval<std::ostream&>(), util::Placeholder{}))> , "'write' must have a signature of void(std::ostream&, <unspecified>)");
				}
			};

		} // namespace attr

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
				/// <summary>
				/// Implements a proxy for a reflected function.
				/// </summary>
				template <typename Proxy, typename Func>
				struct REFL_DETAIL_FORCE_EBO ProxyFunc : public Func::template Remap<ProxyFunc<Proxy, Func>>
				{
					ProxyFunc()
					{
					}

					template <typename... Args>
					decltype(auto) trap(Args&& ... args)
					{
						return reinterpret_cast<Proxy*>(reinterpret_cast<char*>(this))->template trap<Func>(std::forward<Args>(args)...);
					}

				};

				template <typename, typename>
				struct REFL_DETAIL_FORCE_EBO ProxyFuncs;

				/// <summary>
				/// Implements a proxy for all reflected functions.
				/// </summary>
				template <typename Proxy, typename... Members>
				struct REFL_DETAIL_FORCE_EBO ProxyFuncs<Proxy, TypeList<Members...>> : public ProxyFunc<Proxy, Members>...
				{
				};
			} // namespace detail
			
			/// <summary>
			/// Creates a proxy object that has a static interface identical to the reflected functions of type T.
			/// Allows the user to set a `trap` function to act as an mediator.
			/// </summary>
			template <typename T, typename Trap>
			struct REFL_DETAIL_FORCE_EBO Proxy : public detail::ProxyFuncs<Proxy<T, Trap>, typename Members<std::remove_reference_t<T>>::Functions>
			{
				static_assert(
					sizeof(detail::ProxyFuncs<Proxy<T, Trap>, typename Members<T>::Functions>) == 1, 
					"Multiple inheritance EBO did not kick in! "
					"You could try defining the REFL_DETAIL_FORCE_EBO macro appropriately to enable it on the required types. "
					"Default for MSC is `__declspec(empty_bases)`.");

				Proxy(const T& value, const Trap& trap)
					: value_(value)
					, trap_(trap)
				{
				}

				template <typename = std::enable_if_t<!std::is_reference_v<T>>>
				Proxy(T&& value, Trap&& trap)
					: value_(std::move(value))
					, trap_(std::move(trap))
				{
				}

			private:

				template <typename P, typename F>
				friend struct detail::ProxyFunc;

				// Called by one of the ProxyFunc bases.
				template <typename Member, typename... Args>
				decltype(auto) trap(Args&&... args)
				{
					return trap_(Member{}, value_, std::forward<Args>(args)...);
				}

				// Called by one of the ProxyFunc bases.
				template <typename Member, typename... Args>
				decltype(auto) trap(Args&&... args) const
				{
					return trap_(Member{}, value_, std::forward<Args>(args)...);
				}

				T value_;
				Trap trap_;

			};

			/// <summary>
			/// Creates a proxy.
			/// </summary>
			template <typename T, typename Trap>
			auto make_proxy(T&& value, Trap&& trap)
			{
				return Proxy<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<Trap>>>(
					std::forward<T>(value), std::forward<Trap>(trap));
			}

			/// <summary>
			/// Creates a proxy that holds a reference to `T`.
			/// </summary>
			template <typename T, typename Trap>
			auto make_ref_proxy(T&& value, Trap&& trap)
			{
				return Proxy<std::remove_reference_t<T>&, std::remove_cv_t<std::remove_reference_t<Trap>>>(
					std::forward<T>(value), std::forward<Trap>(trap));
			}

			/// <summary>
			/// Writes the debug representation of value to the given std::ostream.
			/// </summary>
			template <typename T>
			void debug(std::ostream& os, const T& value, bool compact = false)
			{
				typedef TypeInfo<T> TypeInfo;
				static_assert(TypeInfo::is_valid || trait::IsContainer<T>::value, "Unsupported type!");

				if constexpr (TypeInfo::is_valid) {
					if constexpr (util::contains<attr::Debug>(TypeInfo::attributes)) {
						auto debug = util::get<attr::Debug>(TypeInfo::attributes);
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
						size_t func_count = TypeInfo::Members::Functions::size;
						typelist::for_each(typename TypeInfo::Members::Functions{}, [&](auto func, auto idx) -> bool {
							typedef decltype(func) func_t;
							if constexpr (util::contains<attr::Property>(func_t::attributes)) {
								auto&& attr = std::get<attr::Property>(func_t::attributes);
								std::string name = attr.friendly_name ? *attr.friendly_name : func_t::name;

								if (!compact) os << "  ";
								os << name << " = ";
								debug(os, func_t::invoke(value), true);
								if (idx != func_count - 1) os << ", ";
								if (!compact) os << "\n";
							}
							return true;
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
			/// Executes a function F for each field of value. 
			/// F takes an member name, the value of the field, and an index. (in this order).
			/// </summary>
			template <typename T, typename F>
			void for_each_field(T&& value, F&& f, bool include_statics = false) {
				typedef TypeInfo<std::remove_reference_t<T>> TypeInfo;
				typelist::for_each(typename TypeInfo::Members::InstanceFields{}, [&](auto field, size_t idx) -> bool {
					f(field.name, util::get_value<decltype(field)>(value), idx);
					return true;
				});
				if (include_statics) {
					typelist::for_each(typename TypeInfo::Members::StaticFields{}, [&](auto field, size_t idx) -> bool {
						f(field.name, util::get_value<decltype(field)>(), idx);
						return true;
					});
				}
			}

			/// <summary>
			/// Executes a function F for each function of value. 
			/// F takes an member name, an invoker object, and an index. (in this order).
			/// </summary>
			template <typename T, typename F>
			void for_each_function(T&& value, F&& f) {
				typedef TypeInfo<std::remove_reference_t<T>> TypeInfo;
				typelist::for_each(typename TypeInfo::Members::Functions{}, [&](auto func, size_t idx) -> bool {
					f(func.name, func.invoke, idx);
					return true;
				});
			}

			/// <summary>
			/// Executes a function F for each property of value. 
			/// F takes an member name, an invoker object, and an index. (in this order).
			/// </summary>
			template <typename T, typename F>
			void for_each_property(T&& value, F&& f) {
				typedef TypeInfo<std::remove_reference_t<T>> TypeInfo;
				size_t idx = 0;
				typelist::for_each(typename TypeInfo::Members::Functions{}, [&](auto func) -> bool {
					if constexpr (util::contains<attr::Property>(func.attributes)) {
						auto p = util::get<attr::Property>(func.attributes);
						auto name = p.friendly_name ? *p.friendly_name : func.name;
						f(name, func.invoke, idx);
						idx++;
					}
					return true;
				});
			}

			namespace detail
			{
				template <typename T, typename U>
				static constexpr bool is_compatible = std::is_assignable_v<std::add_lvalue_reference_t<U>, T> && std::is_constructible_v<U, T>;
			}

			/// <summary>
			/// Invokes the specified member. 
			/// When used with a member that is a field, the function gets or sets the value of the field.
			/// </summary>
			template <typename U, typename T, typename... Args>
			U invoke(T&& target, const char* name, Args&&... args)
			{
				static_assert(refl::TypeInfo<T>::is_valid, "Unsupported type!");
				typedef refl::TypeInfo<std::remove_reference_t<T>> TypeInfo;
				
				std::optional<U> result;

				bool found = refl::typelist::for_each(typename TypeInfo::Members::Functions{}, [&](auto func) -> bool {
					constexpr bool can_invoke = refl::trait::can_invoke<decltype(func.invoke), T, Args...>;
					if constexpr (can_invoke) {
						typedef decltype(func.invoke(target, std::forward<Args>(args)...)) ReturnType;
						if constexpr (detail::is_compatible<ReturnType, U>) {
							if (std::strcmp(func.name, name) == 0) {
								result.emplace(func.invoke(target, std::forward<Args>(args)...));
								return false;
							}
						}
					}
					return true;
				});

				// If we failed to find a match in the reflected function definitions
				// we try to "invoke" the reflected fields of the type by either getting or setting the value.
				if (!found && (sizeof...(Args) == 0 || sizeof...(Args) == 1)) {
					found = refl::typelist::for_each(typename TypeInfo::Members::Fields{}, [&](auto field) -> bool {
						typedef decltype(field) field_t;
						if constexpr (detail::is_compatible<typename field_t::ValueType, U>) {
							if (std::strcmp(field_t::name, name) == 0) {
								// No args means this is considered a getter invocation.
								if constexpr (sizeof...(Args) == 0) {
									result.emplace(util::get_value<field_t>(target));
								}
								// Exactly one arg means this is considered a setter invocation.
								else { // sizeof...(Args) == 1
									result.emplace(util::get_value<field_t>(target) = std::forward<Args>(args)...);
								}
								return false;
							}
						}
						return true;
					});
				}

				if (found) {
					return std::move(*result);
				}
				else {
					throw std::runtime_error(std::string("The member ")
						+ TypeInfo::name + "::" + name
						+ " is not compatible with the provided parameters or return type, is not reflected or does not exist!");
				}
			}
		} // namespace runtime

} // namespace refl

namespace refl::detail
{
	template <typename T>
	auto write_impl(std::ostream& os, T&& t) -> decltype((os << t), void())
	{
		os << t;
	}

	// Dispatches to the appropriate write_impl.
	static constexpr auto write = [](std::ostream& os, auto&& t) -> void { write_impl(os, t); };

	template <typename Tuple, size_t... Idx>
	void write_impl(std::ostream& os, Tuple&& t, std::index_sequence<Idx...>)
	{
		refl::util::ignore((os << std::get<Idx>(t))...);
	}

	template <typename... Ts>
	void write_impl(std::ostream& os, const std::tuple<Ts...>& t)
	{
		write_impl(os, t, std::make_index_sequence<sizeof...(Ts)>{});
	}

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

REFL_TEMPLATE_TYPE(
	(typename Elem, typename Traits, typename Alloc), 
	REFL_EXPAND(std::basic_string<Elem, Traits, Alloc>), 
	(attr::Debug{ refl::detail::write }))

	REFL_FUNC(size, (attr::Property{ }))
	REFL_FUNC(data, (attr::Property{ }))
REFL_END

REFL_TEMPLATE_TYPE(
	(typename... Ts),
	REFL_EXPAND(std::tuple<Ts...>),
	(attr::Debug{ refl::detail::write }))
REFL_END

REFL_TEMPLATE_TYPE(
	(typename K, typename V),
	REFL_EXPAND(std::pair<K, V>),
	(attr::Debug{ refl::detail::write }))
REFL_END

#endif // REFL_NO_STD_SUPPORT

#endif // REFL_INCLUDE_HPP
