# Introduction to refl-cpp

## Overview

refl-cpp allows you to do compile-time introspection, create statically-resolved proxy objects, implement type- or member-wise operations most efficiently and probably more.

## Basic Usage
### [Type Descriptors](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1type__descriptor.html)
`type_descriptor<T>` is the type responsible for exposing a target type `T`'s information.

```cpp
type_descriptor<Point>::name; // -> const_string<5>{"Point"}
```

`name` is of type `const_string<N>` which is a refl-cpp-provided type which allows `constexpr` string equality, concat and slicing. 

The `type_descriptor<T>` type also exposes a `members` data member and a `member_types` type alias where the former is a default instance of the latter. 

```cpp
type_descriptor<Point>::member_types; // -> type_list<member-descriptor-x, member-descriptor-y, func-descriptor-magnitude>
```

Since refl-cpp is a compile-time library, all of the metadata is preserved in a compile-time aware way. Each `type`. `field` or `function_descriptor<T, N>` is a template specialization. That is different from most classic reflection libraries which make use of runtime polymorphism
and have a single non-template type responsible for all fields, functions, etc. 

To allows for operations on these member descriptors refl-cpp uses the `type_list<Ts...>` type which does nothing by itself and is empty, but provides a means for passing a list of types through the type system.

The `member_types` alias above is an alias to a `type_list` where the template arguments are the different "anonymous" classes describing the members of the target class `T`.

As every other `_descriptor` type, `type_descriptor` has an `attributes` data member. `attributes` is a `constexpr std::tuple<Ts...>`, where `Ts...` is exposed by the `attribute_types` type alias. The `attributes` object is a core refl-cpp feature which allows any type or member to be decorated with user-defined objects.

```cpp
type_descriptor<Point>::attributes; // -> std::tuple<Serializable, Debug>
```

These attributes can be used in a `constexpr` context.

### [Field Descriptors](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1field__descriptor.html)
Specializations of the `field_descriptor<T, N>` type represents member fields in refl-cpp. The `field_descriptor` type has a few key properties:

```cpp
using x_field_descriptor = trait::get_t<0, member_list<Point>>;

// inherited from member_descriptor_base<T, N>
x_field_descriptor::name; // -> const_string<1>{"X"}
x_field_descriptor::attributes; // -> std::tuple<...>{attrs...}
// field_descriptor-specific
x_field_descriptor::is_static; // -> false
x_field_descriptor::is_writable; // (is non-const?) -> true
x_field_descriptor::value_type; // -> double
x_field_descriptor::pointer; // -> pointer of type double Point::*
x_field_descriptor::get(); // -> double&

```

### [Function Descriptors](https://veselink1.github.io/refl-cpp/classrefl_1_1descriptor_1_1function__descriptor.html)
Specializations of the `function_descriptor<T, N>` type represents member functions in refl-cpp. The `function_descriptor` type has a few key properties:

```cpp
using magnitude_descriptor = trait::get_t<2, member_list<Point>>;

// inherited from member_descriptor_base<T, N>
magnitude_descriptor::name; // -> const_string<9>{"magnitude"}
magnitude_descriptor::attributes; // -> std::tuple<...>{attrs...}
// function_descriptor-specific
magnitude_descriptor::is_resolved; // -> true
magnitude_descriptor::pointer; // -> pointer of type double (Point::* const)(double, double)
magnitude_descriptor::resolve<Pointer>; // -> pointer of type Pointer on success, nullptr on fail.
magnitude_descriptor::invoke([Self&& self], Args&&... args); // -> the result of self.magnitude(args...)

```

`function_descriptors` can be tricky as they represent a "group" of functions with the same name. Overload resolution is done by the `resolve` or `invoke` functions of `function_descriptor`. Only when the function is not overloaded is `pointer` available (`nullptr` otherwise). A call to `resolve` is needed to get a pointer to a specific overload. A call to `resolve` is **not** needed to `invoke` the target function. The `(*this)` object must be passed as the first argument when a member function is invoked. When invoking a static function, simply provide the arguments as usual. 



---

The examples above expect the following definition of `Point`.
```cpp
class Point {
    double x;
    double y;
    double magnitude() const;
};

void print_point(std::ostream&, const Point&);

REFL_AUTO(
    type(Point, Serializable(), Debug(&print_point)),
    field(x),
    field(y),
    func(magnitude)
)
```
