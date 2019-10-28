# [refl-cpp](https://github.com/veselink1/refl-cpp) (v0.6.4) ([Documentation](https://veselink1.github.io/refl-cpp/namespacerefl.html)) 
[![Gitter](https://badges.gitter.im/refl-cpp/community.svg)](https://gitter.im/refl-cpp/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

A compile-time reflection library for modern C++ with support for templates, attributes and proxies 🔥

## Synopsis
**[refl-cpp](https://github.com/veselink1/refl-cpp) allows static reflection and inspection of types in C++ with full support for *templated types and functions*!** The metadata required for this is specified through the use of macros (but wait!). The macros require the user to only specify the type that is being reflected and only the names of the members that are of interest. ReflCpp has a small and **well-organised API surface**. 

- Macro-based code-gen has been minimized as much as is possible to support all basic requirements. All higher-level operations are implemented as free types and function in one of the utility namespaces.

- Support for **autogeneration of reflection metadata** through a **non-intrusive interface** is ~~coming soon~~ *now available* in the form of [refl-ht](https://github.com/veselink1/refl-ht).

## Requirements
- Minimum language standard: C++17

## Usage
- refl-cpp is packaged as a header-only library! Simply `#include "refl.hpp"` just like any other header.

## License
- MIT License (for more details, see the [license file](https://github.com/veselink1/refl-cpp/blob/master/LICENSE))

## Related Articles
- Introductory post - [Compile-time reflection in C++ 17](https://medium.com/@vesko.karaganev/compile-time-reflection-in-c-17-55c14ee8106b)

- In-depth post - [refl-cpp — A deep dive](https://medium.com/@vesko.karaganev/refl-cpp-deep-dive-86b185f68678)

## Contributing
See [contributors.md](https://github.com/veselink1/refl-cpp/blob/master/contributors.md)

[refl-cpp](https://github.com/veselink1/refl-cpp) is publicly open for any contribution. Bugfixes and new features are welcome. Extra modules are too (as long as the single-header usability is preserved and the functionality is not overly specific to particular use case).

## Examples

### Example metadata declaration (with [refl-ht](https://github.com/veselink1/refl-ht) - a lightweight, portable preprocessor)
```cpp
struct Point {
  REFL(/* attributes */) float x;
  REFL() float y;
};
```

### Example metadata declaration (without [refl-ht](https://github.com/veselink1/refl-ht))
```cpp
struct Point {
  float x;
  float y;
};

/* New style */
REFL_AUTO(
  type(Point),
  field(x, /* attributes */),
  field(y)
)

/* Old style */
REFL_TYPE(Point)
  REFL_FIELD(x, /* attributes */)
  REFL_FIELD(y)
REFL_END
```

### Basic usage example
```cpp
  // Printing an object's properties: 
  Point pt{};
  for_each(refl::reflect(pt).members, [&](auto member) {
      std::cout << member.name << "=" << member(pt) << ";";
  }
  // Result: prints x=0;y=0;
  
  // Converting to an std::tuple: (Note the constexpr!)
  constexpr auto values = map_to_tuple(refl::reflect(pt).members, [&](auto member) {
      return member(pt);
  }
  // Result: values == std::tuple<int, int>{ 0, 0 };
```

## More examples
- See [examples/](https://github.com/veselink1/refl-cpp/tree/master/examples) for more usage examples.

## Metadata-generation macros 
```cpp
// Describes a type and all of its members
REFL_AUTO(
  type(Name, Attribute...),
  field(Name, Attribute...),
  func(Name, Attribute...)
)

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
*Releases follow the MAJOR.MINOR.PATCH versioning scheme*

### v0.6.4
  - Bugfix: usage of incorrect bound in `operator+(const_string<N>, const_string<M>)` (fixed by [Siapran](https://github.com/Siapran))

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
