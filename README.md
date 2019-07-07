# [refl-cpp](https://github.com/veselink1/refl-cpp) (v0.3.2-dev) ([Documentation](https://veselink1.github.io/refl-cpp/namespacerefl.html))
A compile-time reflection library for modern C++ with support for templates, attributes and proxies 🔯🔥

## Synopsis
**[refl-cpp](https://github.com/veselink1/refl-cpp) allows static reflection and inspection of types in C++ with full support for *templated types and functions*!** The metadata required for this is specified through the use of macros (but wait!). The macros require the user to only specify the type that is being reflected and only the names of the members that are of interest. ReflCpp has a small and **well-organised API surface**. 

- Macro-based code-gen has been minimized as must as is possible to support all basic requirements. All higher-level operations are implemented as free types and function in one of the utility namespaces.

- Support for **autogeneration of reflection metadata** through a **non-intrusive interface** is ~~coming soon~~ *now available* in the form of [refl-ht](https://github.com/veselink1/refl-ht).

## Requirements
- Minimum language standard: C++17

## License
- MIT License (for more details, see the [license file](https://github.com/veselink1/refl-cpp/blob/master/LICENSE))

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
