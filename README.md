# refl-cpp
A static reflection library for modern C++ 🔯🔥

## Synopsis
ReflCpp allows static reflection and inspection of types in C++ with full support for *templated types and functions*! The metadata required for this is specified through the use of macros (HOLD ON!). The macros require the user to only specify the type that is being reflected and only the names of the members that are of interest. ReflCpp has a small and well-organised API surface. 

Macro-based code-gen has been minimized as must as is possible to support all basic requirements. All higher-level operations are implemented as free types and function in one of the utility namespaces.

See example.cpp for more usage examples.

Example metadata declaration:
```cpp
struct Point {
  float x, y;
};

REFL_TYPE(Point)
  REFL_FIELD(x)
  REFL_FIELD(y)
REFL_END
```
That's it! OK. So now what? ReflCpp allows you to request some basic properties for a type using the refl::TypeInfo<T> type.
```cpp
  refl::TypeInfo<Point>::name // -> "Point"
  refl::TypeInfo<Point>::attributes // -> std::tuple<>
  refl::TypeInfo<Point>::Members // -> refl::Members<T> (*this is where most of the magic happens*)
```
## Interface of refl::Members&lt;T&gt;
```cpp
  refl::Members<Point>::All // -> refl::TypeList<Member-0, ..., Member-N>
  refl::Members<Point>::Fields // -> refl::TypeList<Field-0, ..., Field-N>
  refl::Members<Point>::InstanceFields // refl::TypeList<InstanceField-0, ..., InstanceField-N>
  refl::Members<Point>::StaticFields // -> refl::TypeList<StaticField-0, ..., StaticField-N>
  refl::Members<Point>::Functions // -> refl::TypeList<Function-0, ..., Function-N>
```

## Interface of a field metadata type
```cpp
    typedef refl::Members<Point>::Fields::Get<0> Field;
    Field::MemberType // -> member::Field (typedef)
    Field::name // -> const char[2] {"x"}
    Field::pointer // -> float Point::* (support: util::get_value, util::is_static)
```

## Interface of a function metadata type
```cpp
    typedef refl::Members<User>::Functions::Get<0> Function;
    Function::MemberType // -> member::Function (typedef)
    Function::name // -> const char[3] {"id"}
    Function::invoke // -> variadic function object (support: trait::can_invoke)
```

## TypeList 
```cpp
namespace refl
{
    template <typename... Ts>
    struct TypeList
    {
        template <size_t Index> using Get = /* type at index */; 
        static constexpr size_t size = /* number of types */;
    };

    namespace typelist
    {
        template <template<typename> typename Predicate, typename... Ts>
        using Filter = TypeList</* Types for which Predicate<T>::value is true */>;
        
        // Calls F with a direct-value constructred instance of every type in Ts. 
        // Stops when F returns false.
        template <typename F, typename... Ts>
        constexpr bool for_each(TypeList<Ts...>, F&& f);

        // Counts the number of times the predicate F returns true.
        template <typename F, typename... Ts>
        constexpr size_t count_if(TypeList<Ts...>, F&& f);

        // Applies function F to each type in the TypeList, aggregating
        // the results in a tuple.
        template <typename F, typename... Ts>
        constexpr auto map_to_tuple(TypeList<Ts...>, F&& f);

        // Applies function F to each type in the TypeList, aggregating
        // the results in an array.
        template <typename F, typename... Ts>
        constexpr auto map_to_array(TypeList<Ts...>, F&& f);
    }
}
```

## Attributes 
```cpp
namespace refl::attr
{
    namespace usage
    {
        /* 
            Inherit your custom attributes from the types
            below to constrait the possible targets for the attribute.
        */
        struct Any;
        struct Type;
        struct Member;
        struct Function;
        struct Field;
    }

    // Specifies that a given function (or function group) represents a property.
    struct Property : usage::Function {
        constexpr Property() noexcept;
        constexpr Property(const char* friendly_name) noexcept;
    }; 
    
    // Provides support for custom debug view of types. 
    // 'write' must have a signature of void(std::ostream&, <unspecified>)
    template <typename F>
    struct Debug : public usage::Type {    
        constexpr Debug(F write);
    };
}
```

## Traits 
```cpp
namespace refl::trait
{
    template <typename T> 
    using IsField = /* std::bool_constant */;

    template <typename T> 
    using IsFunction = /* std::bool_constant */;
    
    template <typename T> 
    using IsStatic = /* std::bool_constant */;

    template <typename T> 
    using IsInstance = /* std::bool_constant */;

    // Transfers all type parameters from U to T.
    template <template<typename...> typename T, typename U>
    using TransferTypes = /* unspecified */;

    // Creates a TypeList by transferring U's type arguments.
    template <typename U>
    using MakeTypeList = TransferTypes<TypeList, U>;

    // Creates an std::tuple by transferring U's type arguments.
    template <typename U>
    using MakeTuple = TransferTypes<std::tuple, U>;
    
    // True if T is a template specialization.
    template <typename T>
    constexpr bool is_template = /* unspecified */;

    // True if the type U is a template specialization of U. 
    // Useful for detecting templated attributes.
    template <template<typename...>typename T, typename U>
    constexpr bool is_instance_of = /* unspecified */;

    // Gets the element type of T.
    template <typename T>
    struct ElementType 
    {
        typedef /* element type of T */ type;
    };

    /* Examples:
        ElementType<int&>::type -> int
        ElementType<int*>::type -> int
        ElementType<int[3]>::type -> int
        ElementType<std::unique_ptr<int>>::type -> int
    */

    template <typename T>
    struct ArraySize
    {
        static constexpr size_t value = /* size of T */;
    };

    /* Examples:
        ArraySize<int[3]>::value -> 3
        ArraySize<std::unique_ptr<int[3]>>::value -> 3
        ArraySize<std::array<int, 3>>::value -> 3
    */

    template <typename T>
    struct IsContainer
    {
        static constexpr bool value = /* T supports .begin() and .end()? */;
    };
}
```

## Runtime 
```cpp
namespace refl::runtime
{
    // Creates a proxy object that has a static interface identical to the reflected functions of type T.
    // Allows the user to set a `trap` function to act as an mediator.
    template <typename T, typename Trap>
    struct Proxy
    {
        Proxy(const T& value, const Trap& trap);
        Proxy(T&& value, Trap&& trap);
    };
    
    // Creates a proxy.
    template <typename T, typename Trap>
    auto make_proxy(T&& value, Trap&& trap);
    
    // Creates a proxy that holds a reference to `T`.
    template <typename T, typename Trap>
    auto make_ref_proxy(T&& value, Trap&& trap);

    // Writes the debug representation of value to the given std::ostream.
    template <typename T>
    void debug(std::ostream& os, const T& value, bool compact = false);
    
    // Executes a function F for each field of value. 
    // F takes an member name, the value of the field, and an index. (in this order).
    template <typename T, typename F>
    void for_each_field(T&& value, F&& f, bool include_statics = false);
    
    // Executes a function F for each function of value. 
    // F takes an member name, an invoker object, and an index. (in this order).
    template <typename T, typename F>
    void for_each_function(T&& value, F&& f);

    // Executes a function F for each property of value. 
    // F takes a *property* name, an invoker object, and an index. (in this order).
    template <typename T, typename F>
    void for_each_property(T&& value, F&& f);

    // Invokes the specified member. 
    // When used with a member that is a field, the function gets or sets the value of the field.
    template <typename U, typename T, typename... Args>
    U invoke(T&& target, const char* name, Args&&... args);
}
```

## Utils 
```cpp
namespace refl::util
{
    // Can serve as a placeholder for any type in unevaluated contexts.
    struct Placeholder
    {
        template <typename T>
        operator T() const;
    };

    // Ignores all arguments.
    constexpr void ignore(...) noexcept {}

    // A combination of all of its supplied type arguments.
    template <typename... Ts>
    struct Combine : public Ts... { /* ... */ };

    // Combines all arguments into one object. 
    // Can be used to create overloaded function objects.
    template <typename... Ts>
    constexpr Combine<Ts...> combine(Ts&& ... ts);

    // Creates an array from the provided arguments.
    template <typename... Ts>
    std::array<std::common_type_t<std::decay_t<Ts>...>, sizeof...(Ts)> make_array(Ts&& ... ts);
    
    // Creates an array of type 'T' from the provided tuple.
    template <typename T, typename... Ts>
    std::array<T, sizeof...(Ts)> to_array(const std::tuple<Ts...>& tuple);

    /// Creates a tuple from the provided array.
    template <typename T, size_t N>
    auto to_tuple(const std::array<T, N>& array);

    /// Returns true if the supplied std::tuple contains a type T.
    template <typename T, typename... Ts>
    constexpr bool contains(const std::tuple<Ts...>& ts);

    // Returns true if the supplied std::tuple contains a type U that is a template instance of T.
    template <template<typename...> typename T, typename... Ts>
    constexpr bool contains(const std::tuple<Ts...>& ts);

    // Returns true if the supplied std::tuple contains a type that derives from T.
    template <typename T, typename... Ts>
    constexpr bool contains_base(const std::tuple<Ts...>& ts);

    // A synonym for std::get<T>(tuple).
    template <typename T, typename... Ts>
    constexpr auto get(const std::tuple<Ts...>& ts);
    
    // Returns the value of type U, where U is a template instance of T.
    template <template<typename...> typename T, typename... Ts>
    constexpr auto get(const std::tuple<Ts...>& ts);
}
```