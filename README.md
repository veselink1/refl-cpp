<center>
<img src="https://raw.githubusercontent.com/veselink1/refl-cpp/master/branding/logo.svg?sanitize=true" alt="refl-cpp Logo" style="max-width: 100%;" height="140">

[![Contributors](https://img.shields.io/github/contributors/veselink1/refl-cpp.svg)](https://github.com/veselink1/refl-cpp/graphs/contributors)
[![Activity](https://img.shields.io/github/commit-activity/m/veselink1/refl-cpp.svg)](https://github.com/veselink1/refl-cpp/pulse)
[![CircleCI](https://circleci.com/gh/veselink1/refl-cpp.svg?style=shield)](https://circleci.com/gh/veselink1/refl-cpp)
[![Gitter](https://badges.gitter.im/refl-cpp/community.svg)](https://gitter.im/refl-cpp/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Patrons](https://img.shields.io/liberapay/patrons/veselink1.svg?logo=liberapay)](https://liberapay.com/veselink1/donate)
[![AUR version](https://img.shields.io/aur/version/refl-cpp?logo=arch-linux)](https://aur.archlinux.org/packages/refl-cpp)

</center>

## refl-cpp v0.12.0 [[Documentation](https://veselink1.github.io/refl-cpp/md__introduction.html)]

A compile-time reflection library for modern C++ with support for overloads, templates, attributes and proxies

## Introduction
**[refl-cpp](https://github.com/veselink1/refl-cpp) allows static reflection and inspection of types in C++17! On top of that, refl-cpp is extensible (build your own abstraction or runtime reflection system) and supports all of the following and more:**
- **overloaded and template functions** - invoke and get pointers to overloaded functions without specifying type arguments in metadata
- **template types** - reflect and work with template classes like regular classes
- **constexpr attributes**- attach custom objects to reflectable members and types (create your own markers, names for external bindings or arbitrary data)
- **proxy types** - programmatic type generation (implement unknown interface, create a generic *builder*, *value type wrapper*, etc.)

## Requirements
- Minimum language standard: C++17

## Usage
- refl-cpp is packaged as a header-only library! Simply `#include "refl.hpp"` just like any other header.

## License
- MIT License (for more details, see the [license file](https://github.com/veselink1/refl-cpp/blob/master/LICENSE))

## Contributing
See [contributors.md](https://github.com/veselink1/refl-cpp/blob/master/contributors.md)

[refl-cpp](https://github.com/veselink1/refl-cpp) is publicly open for any contribution. Bugfixes and new features are welcome. Extra modules are too (as long as the single-header usability is preserved and the functionality is not overly specific to particular use case).

## Donations
Donations are wholeheartedly accepted and will help support this library long term! If you are using refl-cpp in your project and it's making your work easier, please consider making a donation!

[![Donate](https://liberapay.com/assets/widgets/donate.svg)](https://liberapay.com/veselink1/donate)

## Getting started
See the [Introduction](https://veselink1.github.io/refl-cpp/md__introduction.html) for a quick overview of refl-cpp.

## Examples

- **Implement a simple serialization system** - [example-serialization.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-serialization.cpp)
    Shows how to implement a very efficient and generic serialization procedure from scratch

- **Implement a generic builder class factory** - [example-builders.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-builders.cpp)
    Shows how to utilize refl-cpp proxy classes to define a generic `builder<T>` class, which implements the builder pattern

- **Simple SQL database abstraction** - [example-dao.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-dao.cpp)
    Shows how to implement a basic ORM system which can generate SQL statements at compile-time from model classes using custom properties

- **Iterate base classes with `bases<>`** - [example-inheritance.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-inheritance.cpp)
    Shows to use the built-in `bases<>` attribute to iterate over a type's base classes

- **Access reflection information at runtime** - [example-custom-rtti.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-custom-rtti.cpp)
    Shows how to implement a basic runtime abstraction over refl-cpp which provides access to the reflection metadata at runtime via custom metadata objects

- **Type factories with proxies** - [example-proxy.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-proxy.cpp)
    Shows to implement a basic type factory that exposes target type fields as functions

- **XML-based GUI with strongly-types properties** - [example-binding.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-binding.cpp)
    Shows how to implement a system for reading XML resources describing a UI for an imaginary GUI system with refl-cpp

### Basic usage example
A basic example showcasing how refl-cpp can be use to convert an arbitrary type to a tuple at compile-time
```cpp
struct Point {
  float x;
  float y;
};

/* Metadata */
REFL_AUTO(
  type(Point),
  field(x, /* attributes */),
  field(y)
)

Point pt{};

// Converting to an std::tuple: (Note the constexpr!)
constexpr auto values = map_to_tuple(refl::reflect(pt).members, [&](auto member) {
  // refl::descriptor::is_readable (found by Koenig lookup)
  if constexpr (is_readable(member)) {
    return member(pt); // invoke the member
  }
})
// Result: values == std::tuple<int, int>{ 0, 0 };
```

## How it works
Warning: Content might be outdated
- Introductory post - [Compile-time reflection in C++ 17](https://veselink1.github.io/blog/cpp/metaprogramming/2019/06/08/compile-time-reflection-cpp-17.html)

- In-depth post - [refl-cpp — A deep dive](https://veselink1.github.io/blog/cpp/metaprogramming/2019/07/13/refl-cpp-deep-dive.html)

## Metadata-generation macros
```cpp
// Describes a type and all of its members
REFL_AUTO(
  type(Name, Attribute...),
  field(Name, Attribute...),
  func(Name, Attribute...)
)
```

## Old style macros
```cpp
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

## Changelog
*Releases follow the `MAJOR.MINOR.PATCH` versioning scheme*

`Major` denotes a very big change in the structure of the library.

`Minor` denotes a smaller change, which usually includes new features and might also change or even remove certain features.

`Patch` denotes a version which fixed a bug or does not include major new features.

---
### v0.12.0
  - Added `std::complex` support metadata. Disable with `REFL_NO_STD_COMPLEX`.
  - Added `refl::descriptor::get_reader`/`get_writer` utilities for getter/setter properties.
  - Added `refl::trait::index_of` for finding index of type in a `type_list`.
  - Added `refl::descriptor::get_display_name_const` as `const_string` alternative to `get_display_name`.
  - Fixed #33 (by [james-conrad](https://github.com/james-conrad))

### v0.11.0
  - Much improved support for inheritance ([#27](https://github.com/veselink1/refl-cpp/issues/27), [#28](https://github.com/veselink1/refl-cpp/issues/28))
    - `refl::type_descriptor::declared_bases` lists base types declared via the `bases<...>` attribute
    - `refl::type_descriptor::bases` lists base types declared via the `bases<...>` attribute as well as inherited members
    - `refl::type_descriptor::members` now includes declared and inherited members (when base types are specified via `bases<...>` attribute)
    - `refl::type_descriptor::declared_members` preserves the old behavior
    - `refl::attr::bases<...>` types are now validated (via `std::is_base_of`)
  - Added `refl::descriptor::get_simple_name`, which strips namespace and template declarations from the type name (`std::vector<int>` -> `vector`)
  - Added free function equivalents of members of `_descriptor` types for symmetry purposes (and might allow for more optimal implementation in the future)
  - Added `refl::const_string::find/rfind` for searching chars in const_strings
  - Added `refl::make_const_string(char)` overload
  - Added `refl::type_list<T>` specialization which exposes `typedef T type` and a synonym `refl::type_tag<T>`
  - `refl::trait::concat` now support an arbitrary number of `type_list`s as type arguments
  - Added `refl::trait::as_tuple` (similar to `as_type_list` but for `std::tuple`)
  - Added `refl::trait::reverse`
  - Added `refl::util::reflect_types`/`refl::util::unreflect_types` to quickly create a list of `type_descriptor`s from a list of types and then back
  - Introduced support for different types of `std::basic_ostream` in `attr::debug` and `util::debug` (up to the user to take advantage of)
  - Built-in support for `std::string_view`
  - More type assertions in `descriptor::` utilities
  - Simplification of some `trait::` types (should lead to better compile-times)
  - Made unit tests more comprehensive
  - Fixed incorrect `refl::util::identity` implementation for rvalues
  - Fixed static function invocation with `operator()/invoke`
  - Fixed `refl::util::debug` for `std::tuple` [#26](https://github.com/veselink1/refl-cpp/issues/26)
  - Deprecated `refl::descriptor::get_bases` in favor of `refl::descriptor::get_base_types`

### v0.10.0
  - Introduced automatic property name normalization in `get_display_name` for properties with no `friendly_name`. Example: `get_display_name(get_foo) -> "foo"`
  - Added built-in reflection support for `std::string_view` [#23](https://github.com/veselink1/refl-cpp/issues/23)
  - Fixed: operations on `type_list` do not properly take into account reference qualifiers
  - Fixed `const_string` constructor in older versions of clang
  - Fixed `util::filter` results list order (used to be reversed)
  - Fixed implementation of `reflect`, `is_reflectable`, `debug`
  - Improved `runtime::debug` print formatting
  - Removed filtering by `const_string` utils [#21](https://github.com/veselink1/refl-cpp/issues/21). Suggested workaround: use predicate variants
  - Removed refl-ht support
  - Code cleanup [#24](https://github.com/veselink1/refl-cpp/issues/24), [#20](https://github.com/veselink1/refl-cpp/issues/20)

### v0.9.1
  - Bugfix for `refl::descriptor::get_bases` for clang [#19](https://github.com/veselink1/refl-cpp/issues/19)

### v0.9.0
  - Introduced `refl::descriptor::get_bases` (as well as `refl::descriptor::has_bases`) to inspect a type's declared base types (see [example-inheritance.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-inheritance.cpp))
  - Bugfix for is_instance_of for template types with different arity [#18](https://github.com/veselink1/refl-cpp/issues/18)
  - Renamed `refl::descriptor::get_property_info` to `refl::descriptor::get_property` to match the attribute name and its trait-based counterparts.
  - Discontinued [refl-ht](https://github.com/veselink1/refl-ht) - the preprocessor for refl-cpp, due to unfixable bugs and lack of time/interest.

### v0.8.2
  - CV-qualifiers are now properly removed from attribute objects (previously, the values in the backing `std::tuple<...>` were sometimes cv-qualified).

### v0.8.1
  - Fixed a bug that prevented the compilation of `refl::descriptor::get_display_name()` in some compilers (thanks to [ticelo](https://github.com/ticelo))
  - Removed improper usage of `[[maybe_unused]]` that caused warnings under clang.

### v0.8.0
  - Added the ability to create `refl::const_string<N>` instances from `const char*` which allows using c-strings (`const char*`) values from attributes together with `const_string<N>` and in `constexpr` context possible. That feature is exposed by the `REFL_MAKE_CONST_STRING(CString)` macro. Important: `CString` must be a [Core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression#Core_constant_expressions). The macro is necessary to avoid the duplication of the provided expression and currently expands to ~ `detail::copy_from_unsized<detail::strlen(CString)>(CString)`. (another thanks to [Siapran](https://github.com/Siapran) for contributing the implementation for the `constexpr length()` function.)

### v0.7.0
  - Added `constexpr const_string::substr<Pos, [Count]>()`
  - Added `make_const_string()` as a shortcut for the creation of empty const strings.
  - Added `trait::{first, last, tail, init, append, prepend, reverse, concat}` and `TraitName_t` equivalents to ease the use of `type_list<...>`.
  - Breaking change: `trait::map` and `trait::filter` now no longer accept variadic template parameters. Wrap variadic parameters in a type_list. (Allowing both `type_list<Ts...>` and `Ts...` is problematic due to cryptic errors when e.g. `const type_list<Ts...>` is provided.)
  - Breaking change: `util::accumulate` now mimics the behaviour of `std::accumulate` more closely. It relies on the return value of the functor instead of mutating the accumulator argument. The new implementation now allows for the types of `initial_value` and the return type to differ (thanks to [Siapran](https://github.com/Siapran)).
  - Removed: `util::get<N>(const type_list<Ts...>&)`. Use `trait::get_t<N, type_list<Ts...>>` instead. This is part of a move towards minimizing the duplication between the `trait::` and `util::` namespaces.
  - Added two new examples: [example-binding.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-binding.cpp) and [example-dao.cpp](https://github.com/veselink1/refl-cpp/blob/master/examples/example-dao.cpp) to present some of the new things introduced in this release. (example-dao.cpp based on work by [Siapran](https://github.com/Siapran))

### v0.6.5
  - Fixed multiple unused X warnings (fixed by [Siapran](https://github.com/Siapran)) #12

### v0.6.4
  - Bugfix: usage of incorrect bound in `operator+(const_string<N>, const_string<M>)` (fixed by [Siapran](https://github.com/Siapran)) #11

### v0.6.3
  - Bugfix: clang errors during compilation of reflected overloaded member functions (since 0.6.0)

### v0.6.2
  - Bugfix: Compiler error in clang: refl::attr::property's implicitly-declared default constructor is deleted due to N3797 §8.5/7 [#9](https://github.com/veselink1/refl-cpp/issues/9)

### v0.6.1
  - Bugfix: const_string::operator std::string() was incorrectly marked as constexpr which caused compilation failure in non-C++20-compliant compilers

### v0.6.0
  - Changed: property() is now only usable on function members / use contract based checks for functions and fields (is_readable/writable)
  - Removed: read_only, write_only, read_write access modifiers for property() attributes (rw-detection now based on contract).

### v0.5.2
  - Added: `type_descriptor<T>::type` as an alias for `T`.
  - Added: `function_descriptor<T, N>::pointer` which points to the target function, if the pointer can be resolved and to nullptr otherwise.
  - Added: `function_descriptor<T, N>::is_resolved` for checking whether the pointer to the function has been resolved successfully.
  - Added: `function_descriptor<T, N>::resolve<Pointer>` for explicitly resolving the function pointer as a value of the specified type.
  - Added: `util::get<size_t N>(type_list<Ts...>&&)` which zero-initializes and returns the type at the N-th position.
  - Added: `util::get<size_t N>(Tuple&&)` as an alias for `std::get<N>`.

### v0.5.1
  - Bugfix: The REFL_AUTO macro was not working properly in MSVC and led to Internal compiler errors
  - Changed: As a result of the related bugfix, the REFL_AUTO macro has been stabilized and its use is now recommended

### v0.5.0
  - Removed deprecated macros `$refl(...)`, `REFL_UNSTABLE(...)`, `REFL_DEPRECATED(...)`
  - Removed deprecated `refl::attr::access_type::read, write` constants. Replaced by read_only, write_only for consistency.
  - Removed deprecated `refl::attr::read_only, read_write, write_only` constants. Use the enum-qualified values. The constants are still available unqualified in macro context.
  - Removed refl::descriptor::make_invoker (which was not deprecated but is now obsolete.) Simply replace make_invoker(x) with x.
  - The defaulted constructor for `refl::util::const_string<N>` was replaced by a user-declared default constructor which now always zero-initializes the string buffer.


### v0.4.2
  - Added: `trait::is_type[_v]`, `descriptor::is_type(const T&)` for checking whether a type is a type_descriptor<T> instance.

### v0.4.1
  - `trait::as_type_list` now supports reference type in place of `T<Ts...>` (e.g. `std::tuple<Ts...>&` -> `type_list<Ts...>`).
  - `refl::attr::access_type::{read, write}` now deprecated, replaced by `{read_only, write_only}` (Note: implicitly-usable `read_only`, `write_only` constants in attribute (in `REFL_...` macros) context remain unchanged)
  - `refl::descriptor::field_descriptor::operator()` now supports acting as a setter. `refl::descriptor::make_invoker` is now no longer needed and marked as deprecated.

### v0.4.0
  - Renamed: `trait::filter` to `trait::filter_t`, replaced by `struct trait::filter { typedef ... type; }` to follow preset convention
  - Renamed: `trait::map` to `trait::map_t`, replaced by `struct trait::map { typedef ... type; }` to follow preset convention
  - Breaking change: `function_descriptor<...>::pointer` now no longer exists. (Reason: Replacing with forwarding function allows for more flexibility when using  `refl::runtime::proxy<>`. Migrate by taking a pointer to `function_descriptor<...>::invoke()`)
  - Breaking change: trait::contains/base/instance now no longer work implicitly with `std::tuple<...>` or support variadic arguments. Use `type_list<>` or `trait::as_type_list_t` to migrate working code. (Reason: Specializations found to be causative to many recently discovered bugs and are error-prone.)
  - Bugfix: faulty implementation of `trait::skip<N>`
  - Bugfix: faulty implementation of `util::to_tuple`
  - Bugfix: `trait::contains_base` was working the other way around (checking for presense of base of a specified type, instead of checking for a derived type with a specified base)

### v0.3.5
  - Bugfix: `type_info__<T*>` has name defined as `char[N]` instead of `const_string<N>`

### v0.3.4
  - added new experimental syntax for metadata declaration that makes use of variadic macro expansion (can be disabled with `REFL_NO_VARIADICS`)
```cpp
REFL_AUTO
(
    type(User), // expands to REFL_TYPE(User)
    field(id, property(read_only)), // expands to REFL_FIELD(id, property(read_only))
    field(email),
    field(first_name, property("firstName")),
    field(last_name, property("lastName"))
    func(save_to_db) // expands to REFL_FUNC(save_to_db)
) // REFL_END appended automatically
```

### v0.3.3
  - `refl::util::const_string<N>` can now be directly compared for equality with `char[M]`
  - `refl::util::const_string<N>` can now be concatenated with `char[M]`

### v0.3.2
  - `refl::runtime::debug` now captures values of member invocations by universal references (previous implementations required a copy of the value)
  - `refl::runtime::debug` now does not require the members of composites to be reflectable. (members that cannot be printed get replaced with `<?>`).
  - `refl::trait::is_reflectable` now discards cv-qualifiers -> all cv-qualified types are now also reflectable.

### v0.3.1
  - `refl::descriptor::is_writable` now correctly supports field descriptors
  - most of `refl.hpp` is now not included when `REFL_PREPROCESSOR` is defined (optimization for refl-ht)

### v0.3.0
  - added new `REFL(...)` macro for annotation of reflectable members (replaces `$refl(...)`)
  - `$refl(...)` macro usage now deprecated

### v0.2.1
  - added support for [refl-ht](https://github.com/veselink1/refl-ht) (the refl-cpp header tool)

### v0.2.0
  - added `refl::util::contains(type_list<...>, [predicate]), refl::util::contains(type_list<...>, const_string<N>)`
  - removed `refl::attr::is_readable/is_writable` (use `refl::descriptor::is_readable/is_writable` instead; reason: lack of support for fields that lack a property attribute)

### v0.1.2
  - `refl::runtime::proxy<Derived, Target>` can now delegate field 'invocations'. (Methods with names matching those of the reflected fields are created.)
  - `refl::attr::property` now can take an optional `refl::attr::access_type` (values: `read_only`, `write_only`, `read_write`) to specify whether the property is considered readable or writable (or both).
  - added `refl::descriptor::is_readable/is_writable`
  - added `refl::attr::is_readable/is_writable`
