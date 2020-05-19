# Introduction to refl-cpp

## Introduction

refl-cpp is a new compile-time reflection library targeting C++17 and newer. It works by encoding type metadata in the type system, which allows the user to access that information at compile-time. refl-cpp supports compile-time enumeration of fields and functions, constexpr custom attributes (objects bound to types/fields/functions), template types, proxy objects which resolve function calls at compilation time and much more.

This documents gives tips on how best to utilize refl-cpp for your use case. Please, also refer to the [documentation](https://veselink1.github.io/refl-cpp/namespacerefl.html) and [examples](https://github.com/veselink1/refl-cpp/tree/master/examples).

## Basics
refl-cpp relies on the user to properly specify type metadata through the use of macros.

```cpp
struct A {};

REFL_AUTO(type(A))
```

* The metadata should be available before it is first requested, and should ideally be put right after the definition of the target type (forward declarations won't work).

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
    Circle(double r) : r() {}
    double radius() const;
    double diameter() const;
    double area() const;
};

REFL_AUTO(
    type(Circle),
    func(radius),
    func(diameter),
    func(area)
)
```

Like fields, functions are represented through specializations of a "descriptor" type, namely, [`function_descriptor<T, N>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html). `T` is the target type, and `N` is the index of the reflected member, regardless of the type of that member (field or function). [`function_descriptor<T, N>`](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html) is never used directly.

There are multiple ways to get a function's descriptor. The easiest one is by using the name of the member together with the [`find_one`](https://veselink1.github.io/refl-cpp/namespacerefl_1_1util.html#a019b3322cffd29fd129b6378ef499668) helper.

```cpp
using refl::reflect;
using refl::util::find_one;
constexpr auto type = reflect<Circle>();

constexpr auto func = find_one(type.members, [](auto m) { return m.name == "radius"; }); // -> function_descriptor<Circle, 0>{...}
```

Function descriptors expose a number of properties to the user.

```cpp
// continued from previous example
func.name; // -> const_string<6>{"radius"}
func.attributes; // -> std::tuple<>{}
func.is_resolved; // -> true
func.pointer; // -> pointer of type double (Circle::* const)()

using radius_t = double (Circle::* const)();
func.template resolve<radius_t>; // -> pointer of type radius_t on success, nullptr_t on fail.

Circle c(2.0);
func.invoke(c); // -> the result of c.radius()
```

Function descriptors can be tricky as they represent a "group" of functions with the same name. Overload resolution is done by the `resolve` or `invoke` functions of `function_descriptor`. Only when the function is not overloaded is `pointer` available (`nullptr` otherwise). A call to `resolve` is needed to get a pointer to a specific overload. A call to `resolve` is **not** needed to `invoke` the target function. The `(*this)` object must be passed as the first argument when a member function is invoked. When invoking a static function, simply provide the arguments as usual.

## Custom Attributes
TOOD

## Proxies
TODO

## Examples Guide

- Implementing a simple serialization system - [example-serialization.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-serialization.cpp)
    Shows how to implement a very efficient and generic serialization procedure from scratch

- Implementing a generic builder class factory - [example-builders.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-builders.cpp)
    Shows how to utilize refl-cpp proxy classes to define a generic `builder<T>` class, which implements the builder pattern

- Simple SQL database abstraction - [example-dao.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-dao.cpp)
    Shows how to implement a basic ORM system which can generate SQL statements at compile-time from model classes using custom properties

- Iterating base classes with `bases<>` - [example-inheritance.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-inheritance.cpp)
    Shows to use the built-in `bases<>` attribute to iterate over a type's base classes

- Accessing reflection information at runtime - [example-custom-rtti.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-custom-rtti.cpp)
    Shows how to implement a basic runtime abstraction over refl-cpp which provides access to the reflection metadata at runtime via custom metadata objects

- Type factories with proxies - [example-proxy.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-proxy.cpp)
    Shows to implement a basic type factory that exposes target type fields as functions

- XML-based GUI with strongly-types properties - [example-binding.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-binding.cpp)
    Shows how to implement a system for reading XML resources describing a UI for an imaginary GUI system with refl-cpp