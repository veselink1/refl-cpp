# Introduction to refl-cpp
## Basics
refl-cpp relies on the user to properly specify type metadata through the use of the `REFL_AUTO` macro.

```cpp
struct A {
    int foo;
    void bar();
    void bar(int);
};

REFL_AUTO(
    type(A),
    field(foo),
    func(bar)
)
```

This macro generated the necessary metadata needed for compile-time reflection to work. The metadata is encoded in the type system via generated type specializations which is why there is currently no other way that using a macro. See [example-macro.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-macro.cpp) for what the output of the macro looks like. (NOTE: It is a lot, but remember that compilers only validate and produce code for templates once they are used, until then the metadata is just token soup that gets optimized out from the resulting binary)

- The metadata should be available before it is first requested, and should ideally be put right after the definition of the target type (forward declarations won't work).

## Two macro styles
The two types are functionally-equivalent, in fact, Type 1 is directly transformed into Type 2 macros via preprocessor magic. Type 1 is more succint, but generates worse stack traces.
### Type 1
```cpp
// Example
REFL_AUTO(
    type(Point)
    field(x)
    field(y)
)

// Describes a type and all of its members
REFL_AUTO(
  type(Name, Attribute...),
  field(Name, Attribute...),
  func(Name, Attribute...)
)
```
### Type 2
```
// Example
REFL_TYPE(Point)
    REFL_FIELD(x)
    REFL_FIELD(y)
REFL_END

// Starts the declaration of Type's metadata.
// Must be followed by one of: REFL_END, REFL_FIELD, REFL_FUNC.
// Example: REFL_TYPE(Dog, bases<Animal>)
REFL_TYPE(Type, Attributes...)

// Starts the declaration of Type's metadata.
// Must be followed by one of: REFL_END, REFL_FIELD, REFL_FUNC.
// Example: REFL_TEMPLATE((typename K, typename V), (std::pair<K, V>))
REFL_TEMPLATE((typename... Ts), (Type<Ts...>)), Attributes...)

// End the declaration of Type's metadata.
// Does not have an argument list
REFL_END

// Describes a field.
// Example: REFL_FIELD(first_name, property{ "Value" })
REFL_FIELD(Field, Attributes...)

// Describes a function.
// Example: REFL_FUNC(first_name, property{ "Size" })
REFL_FUNC(Function, Attribute...)

// NOTE: None of the macros above need a terminating semi-colon (;)
```

## Type metadata
refl-cpp exposes access to the metadata through the [`type_descriptor<T>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1type__descriptor.html) type. All of the metadata is stored in static fields on the corresponding specialization of that type, but for convenience, objects of the metadata types are typically used in many places, and can be obtained through calling the trivial constructor or through the [`reflect<T>`](https://veselink1.github.io/refl-cpp/namespacerefl.html#ae95fbc2d63a7db5ce4d8a4dcca3d637e) family of functions.

```cpp
// continued from previous example
using refl::reflect;
using refl::descriptors::type_descriptor;

constexpr type_descriptor<A> type{};
constexpr auto type = reflect<A>(); // equivalent
```

[`type_descriptor<T>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1type__descriptor.html) provides access to the target's name, members and attributes.

```cpp
type.name; // -> const_string<5>{"Point"}
foo.members; // -> type_list<>{}
foo.attributes; // -> std::tuple<>{}
```

`name` is of type `const_string<N>` which is a refl-cpp-provided type which allows `constexpr` string equality, concat and slicing.

In a similar way to type metadata, member metadata is also represented through template specializations. The `type_list<Ts...>` is an empty trivial template type, which provides a means for passing that list of types through the type system.

Custom attributes are stored in a constexpr `std::tuple` which is exposed through the metadata descriptor.

* Since the type `A` has no members and no attributes defined, `members` and `attribute` are of type `type_list<>`, an empty list of types, and `std::tuple<>`, an empty tuple, respectively.

## Field metadata
Let's use a the following simple Point type definition to demonstrate how field reflection works.

```cpp
struct Point {
    int x;
    int y;
};

REFL_AUTO(
    type(Point),
    field(x),
    field(y)
)
```

Fields are represented through specializations of the [`field_descriptor<T, N>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1field__descriptor.html). `T` is the target type, and `N` is the index of the reflected member, regardless of the type of that member (field or function). [`field_descriptor<T, N>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1field__descriptor.html) is never used directly.

```cpp
constexpr auto type = refl::reflect<Point>();

std::cout << "type " << type.c_str() << ":";
// for_each discovered by Koenig lookup (for_each and decltype(type.members) are in the same namespace)
for_each(type.members, [](auto member) { // template lambda invoked with field_descriptor<Point, 0..1>{}
    std::cout << '\t' << member.name << '\n';
});
/* Output:
   type Point:
       x
       y
*/
```

There are multiple ways to get a field's descriptor. The easiest one is by using the name of the member together with the [`find_one`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1util.html#a019b3322cffd29fd129b6378ef499668) helper.
```cpp
using refl::reflect;
using refl::util::find_one;
constexpr auto type = reflect<Point>();

constexpr auto field = find_one(type.members, [](auto m) { return m.name == "x"; }); // -> field_descriptor<Point, 0>{...}
```

Field descriptors provide access to the field's name, type, const-ness, whether the field is static, a (member) pointer to the field and convenience [`get()`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1field__descriptor.html#a5a7ca636e0dea5431786a2dac4f149c4) and [`operator()`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1field__descriptor.html#af003db500839f3ca03b9408bcf76b009) methods which return references to that field.

```cpp
// continued from previous example
field.name; // -> const_string<1>{"x"}
field.attributes; // -> std::tuple<>{}
field.is_static; // -> false
field.is_writable; // -> true (non-const)
field.value_type; // -> int
field.pointer; // -> pointer of type int Point::*

Point pt{5, -2};
field.get(pt); // -> int& (5)
field(pt); // -> int& (5)
```

As with [`type_descriptor<T>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1type__descriptor.html), all of the metadata is exposed through constexpr static fields and functions, but an object is used to access those for convenience purposes and because objects can be passed to functions as values as well.

* Since the field `Point::x` in this example has no custom attributes associated with it, the `field.attributes` in the example above will be generated as `static constexpr std::tuple<>{}`.

## Function metadata
We will be using the following type definition for the below examples.

```cpp
class Circle {
    double r;
public:
    Circle(double r) : r(r) {}
    double getRadius() const;
    double getDiameter() const;
    double getArea() const;
};

REFL_AUTO(
    type(Circle),
    func(getRadius),
    func(getDiameter),
    func(getArea)
)
```

Like fields, functions are represented through specializations of a "descriptor" type, namely, [`function_descriptor<T, N>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html). `T` is the target type, and `N` is the index of the reflected member, regardless of the type of that member (field or function). [`function_descriptor<T, N>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html) is never used directly.

There are multiple ways to get a function's descriptor. The easiest one is by using the name of the member together with the [`find_one`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1util.html#a019b3322cffd29fd129b6378ef499668) helper.

```cpp
using refl::reflect;
using refl::util::find_one;
constexpr auto type = reflect<Circle>();

constexpr auto func = find_one(type.members, [](auto m) { return m.name == "getRadius"; }); // -> function_descriptor<Circle, 0>{...}
```

Function descriptors expose a number of properties to the user.

```cpp
// continued from previous example
func.name; // -> const_string<6>{"getRadius"}
func.attributes; // -> std::tuple<>{}
func.is_resolved; // -> true
func.pointer; // -> pointer of type double (Circle::* const)()

using radius_t = double (Circle::* const)();
func.template resolve<radius_t>; // -> pointer of type radius_t on success, nullptr_t on fail.

Circle c(2.0);
func.invoke(c); // -> the result of c.getRadius()
```

Function descriptors can be tricky as they represent a "group" of functions with the same name. Overload resolution is done by the [`resolve`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html#a7f8b63e35466c3c2887f601272d9f0a0) or [`invoke`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html#a5f6c4091c03a8fb9d5f6459c686ea655) functions of [`function_descriptor<T, N>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html). Only when the function is not overloaded is [`pointer`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html#a6e18ad19be31eb26acfe1e84fd320c36) available (`nullptr` otherwise). A call to [`resolve`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html#a7f8b63e35466c3c2887f601272d9f0a0) is needed to get a pointer to a specific overload. A call to [`resolve`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html#a7f8b63e35466c3c2887f601272d9f0a0) is **not** needed to [`invoke`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html#a5f6c4091c03a8fb9d5f6459c686ea655) the target function. The `(*this)` object must be passed as the first argument when a member function is invoked. When invoking a static function, simply provide the arguments as usual.

## Custom Attributes
refl-cpp allows the association of compile-time values with reflectable items. Those are referred to as attributes. There are 3 built-in attributes, which can all be found in the [`refl::attr`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1attr.html) namespace.

### Properties
[`property`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1property.html) (usage: function) - used to specify that a function call corresponds to an object property

```cpp
RELF_AUTO(
    type(Circle),
    func(getArea, property("area"))
)
```

Built-in support for properties includes:
- [`refl::descriptor::get_property`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1descriptor.html#a486e58c3515f4da0b687a195f0db1734) - returns the `property` attribute
- [`refl::descriptor::is_property`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1descriptor.html#acc8c814d7bd04ba0cf4386e49e469a3b) - checks whether the function is marked with the `property` attribute
- [`refl::descriptor::get_display_name`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1descriptor.html#aa7c9753a84fecf4d9c62ce5b5063fb47) - returns the [`friendly_name`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1property.html#a8c45f77ef5159115250f2294bd37d296) set on the property, if present, otherwise the name of the member itself

### Base Types
[`base_types`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1base__types.html) (usage: type) - used to specify the base types of the target. The `bases<Ts...>` template variable can be used in place of `base_types<Ts...>{}`
```cpp
REFL_AUTO(
    type(Circle, bases<Shape>),
    /* ... */
)
```

Built-in support for base types includes:
- [`refl::descriptor::get_bases`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1descriptor.html#a5ca7ae3c51dbe88ffa332c310eac9f11) - returns a [`type_list`](https://veselink1.github.io/refl-cpp/structrefl_1_1util_1_1type__list.html) of the type descriptors of the base classes (Important: Fails when there is no [`base_types`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1base__types.html) attribute)
- [`refl::descriptor::has_bases`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1descriptor.html#a079a7d252e99cd446ec275b218c461d1) - checks whether the target type has a [`base_types`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1base__types.html) attribute

### Debug Formatter
[`debug<F>`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1debug.html) (usage: any) - used to specify a function to be used when constructing the debug representation of an object by [`refl::runtime::debug`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1runtime.html#a1edadcdb602a1e96beedbdeeca801701)

All attributes specify what targets they can be used with. That is done by inheriting from one or more of the marker types found in [`refl::attr::usage`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1attr_1_1usage.html). These include [`field`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1usage_1_1field.html), [`function`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1usage_1_1field.html), [`type`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1usage_1_1type.html), [`member`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1usage_1_1member.html) (`field` or `function`), [`any`](https://veselink1.github.io/refl-cpp/structrefl_1_1attr_1_1usage_1_1any.html) (`member` or `type`).

### Custom Attributes

Custom attributes can be created by inheriting from one of the usage strategies:
```cpp
struct Serializable : refl::attr::usage::member
{
};
```

And then used by passing in objects of those types as trailing arguments to the member macros.

```cpp
REFL_AUTO(
    type(Circle),
    func(getArea, property("area"), Serializable())
)
```

The presence of custom attributes can be detected using [`refl::descriptor::has_attribute<T>`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1descriptor.html#af47d9eca02998b834a0d827aa2be2252).
```cpp
using refl::reflect;
using refl::descriptor::has_attribute;

for_each(reflect<Circle>().members, [](auto member) {
    if constexpr (has_attribute<Serializable>(member)) {
        std::cout << get_display_name(member) << " is serializable\n";
    }
});
```

Values can be obtained using [`refl::descriptor::get_attribute<T>`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1descriptor.html#a2e9e9b85233f7f13fe09cb4fd6bbc6f6).

**NOTE: Most of the descriptor-related functions in `refl::descriptor` which take a descriptor parameter can be used without being imported into the current namespace thanks to ADL-lookup (example: `get_display_name` is not explictly imported above)**

## Proxies

The powerful proxy functionality provided by [`refl::runtime::proxy<Derived, Target>`](https://veselink1.github.io/refl-cpp/structrefl_1_1runtime_1_1proxy.html) in refl-cpp allows the user to transform existing types.

```cpp
template <typename T>
struct Echo : refl::runtime::proxy<value_proxy<T>, T>
{
    template <typename Member, typename Self, typename... Args>
    static constexpr decltype(auto) invoke_impl(Self&& self, Args&&... args)
    {
        std::cout << "Calling " << get_display_name(Member{}) << "\n";
        return Member{}(self, std::forward<Args>(args)...);
    }
};

Echo<Circle> c;
double d = c.getRadius(); // -> calls invoke_impl with Member=function_descriptor<Circle, ?>, Self=Circle&, Args=<>
// prints "Calling Circle::getRadius" to stdout
```

This is a very powerful and extremely low-overhead, but also complicated feature. Delegating calls to `invoke_impl` is done at compile-time with no runtime penalty. Arguments are passed using perfect forwarding.

See the examples below for how to build a generic builder pattern and POD wrapper types using proxies.

## Runtime Utilities
Utilities incurring runtime penalty are contained in the [`refl::runtime`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1runtime.html) namespace. That make it clear when some overhead can be expected.

### Invoking a member by name at runtime
[`refl::runtime::invoke`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1runtime.html#a0831da0114fa506579224ce219d0ab50) can invoke a member (function or field) on the provided object by taking the name of the member as a `const char*`. [`invoke`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1runtime.html#a0831da0114fa506579224ce219d0ab50) compiles into very efficient code, but is not as fast a directly invoking a member due to the needed string comparison.  This can be useful when generating bindings for external tools and languages. [`invoke`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1runtime.html#a0831da0114fa506579224ce219d0ab50) filters members by argument and returns types *before* doing the string comparison, which often reduces the number of comparisons required substantially.

```cpp
using refl::runtime::invoke;

Circle c;
double rad = invoke<double>(c, "getRadius"); // calls c.getRadius(), returns double
```

## Printing debug output
refl-cpp can automatically generate a debug representation for your types based on the type metadata it is provided.

```cpp
REFL_AUTO(
    type(Circle),
    func(getRadius, property("radius")),
    func(getArea, property("area"))
)

using refl::runtime::debug;

Circle c(2.0);

debug(std::cout, c);
/* Output: {
  radius = (double)2,
  area = (double)19.7392
} */
debug(std::cout, c, /* compact */ true);
/* Output: { radius = 2, area = 19.7392 } */
```

While [`debug`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1runtime.html#a1edadcdb602a1e96beedbdeeca801701) outputs to a `std::ostream`, a `std::string` result can also be obtained by [`debug_str`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1runtime.html#aa31714a8d8acc6824a58850336fa43ae).

## Utility library
All utility functions are contained in the [`refl::util`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1util.html) namespace. Some of the most useful utility functions include:
- [`for_each`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1util.html#a19919596cdd45c858d891c91a7826b22) - Applies function F to each type in the type_list. F can optionally take an index of type size_t.
- [`map_to_tuple(type_list<Ts...>, F&& f)`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1util.html#af69ca2ba2b0353b7d3433f6a77b62902) - Applies function F to each type in the type_list, aggregating the results in a tuple. F can optionally take an index of type size_t.
- [`get_instance<T>(std::tuple<Ts...>& ts)`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1util.html#a0aec187f4a301c7f995ac65e17fa029a) - Returns the value of type U, where U is a template instance of T.

**NOTE: Most of the utility functions in `refl::util` which take a `type_list<...>` parameter can be used without being imported into the current namespace thanks to ADL-lookup**

Example:
```cpp
for_each(refl::reflect<Circle>(), [](auto m) {});
```

## Type-level operations
refl-cpp provides a range of type-transforming operations in the [`refl::trait`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1trait.html) namespace. Some of the most commonly used type traits are:
- [`get<N, type_list<Ts...>>`](https://veselink1.github.io/refl-cpp/structrefl_1_1trait_1_1get.html)
- [`is_container<T>`](https://veselink1.github.io/refl-cpp/structrefl_1_1trait_1_1is__container.html)
- [`is_reflectable<T>`](https://veselink1.github.io/refl-cpp/structrefl_1_1trait_1_1is__reflectable.html)
- [`is_proxy<T>`](https://veselink1.github.io/refl-cpp/structrefl_1_1trait_1_1is__proxy.html)
- [`as_type_list<T<Ts...>>`](https://veselink1.github.io/refl-cpp/structrefl_1_1trait_1_1as__type__list.html)
- [`contains<T, type_list<Ts...>>`](https://veselink1.github.io/refl-cpp/structrefl_1_1trait_1_1contains.html)
- [`map<Mapper, type_list<Ts...>>`](https://veselink1.github.io/refl-cpp/structrefl_1_1trait_1_1map_3_01_mapper_00_01type__list_3_01_ts_8_8_8_01_4_01_4.html)
- [`filter<Predicate, type_list<Ts...>>`](https://veselink1.github.io/refl-cpp/structrefl_1_1trait_1_1filter_3_01_predicate_00_01type__list_3_01_ts_8_8_8_01_4_01_4.html)

`[trait]_t` and `[trait]_v` typedefs and constexpr variables are provided where appropriate.
