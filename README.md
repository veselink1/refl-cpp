# refl-cpp v0.12.1
## [Documentation](https://veselink1.github.io/refl-cpp/md__r_e_a_d_m_e.html)

<center>
<img src="https://raw.githubusercontent.com/veselink1/refl-cpp/master/branding/logo.svg?sanitize=true" alt="refl-cpp Logo" style="max-width: 100%;" height="140">

[![Contributors](https://img.shields.io/github/contributors/veselink1/refl-cpp.svg)](https://github.com/veselink1/refl-cpp/graphs/contributors)
[![Activity](https://img.shields.io/github/commit-activity/m/veselink1/refl-cpp.svg)](https://github.com/veselink1/refl-cpp/pulse)
[![CircleCI](https://circleci.com/gh/veselink1/refl-cpp.svg?style=shield)](https://circleci.com/gh/veselink1/refl-cpp)
[![Gitter](https://badges.gitter.im/refl-cpp/community.svg)](https://gitter.im/refl-cpp/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Patrons](https://img.shields.io/liberapay/patrons/veselink1.svg?logo=liberapay)](https://liberapay.com/veselink1/donate)
[![AUR version](https://img.shields.io/aur/version/refl-cpp?logo=arch-linux)](https://aur.archlinux.org/packages/refl-cpp)

</center>

refl-cpp encodes type metadata in the type system to allow compile-time reflection via constexpr and template metaprogramming in C++17

## Introduction

refl-cpp aims to provide a generic static reflection system that can be extended to suit your needs. **Runtime reflection** can also be implemented on top of the existing system for when needed (see the examples).

Some nice things refl-cpp supports out-of-the-box:
- **custom attributes** - encodes type-level and member-level attributes as a constexpr std::tuple associated with that item
- **proxy types** - builds configurable generic types with identical member functions that can be hooked into to wrap or extend functionality
- **overloaded functions** - wraps member functions in generic variadic templates
- **template functions** - template function for which the template parameters can be inferred from the arguments are also supported
- **template types** - uses template specialization to encode metadata for types; template types are perfectly-well supported

## Code Example (see on [Compiler Explorer](https://godbolt.org/z/1G35xhqo8))
A basic example showcasing how refl-cpp can be use to convert an arbitrary type to a tuple at compile-time
```cpp
// Custom attribute to mark fields for validation
struct NonNegative : refl::attr::usage::member {};

struct Point {
    double x, y;
};

REFL_AUTO(
    type(Point),
    field(x, NonNegative()),
    field(y, NonNegative())
)

struct Circle {
    Point origin;
    double radius;
};

REFL_AUTO(
    type(Circle),
    field(origin),
    field(radius, NonNegative())
)

template <typename T>
constexpr bool checkNonNegative(const T& obj) {
    // Get the type descriptor for T
    constexpr auto type = refl::reflect<T>();
    // Get the compile-time refl::type_list<...> of member descriptors
    constexpr auto members = get_members(type);
    // Filter out the non-readable members (not field or getters marked with the property() attribute)
    constexpr auto readableMembers = filter(members, [](auto member) { return is_readable(member); });

    auto invalidMemberCount = count_if(readableMembers, [&](auto member) {
        // Function-call syntax is a uniform way to get the value of a member (whether a field or a getter)
        auto&& value = member(obj);
        // Check if the NonNegative attribute is present
        if constexpr (refl::descriptor::has_attribute<NonNegative>(member)) {
            // And if so, make the necessary checks
            return value < 0;
        }
        // Recursively check the value of the member
        else if (!checkNonNegative(value)) {
            return true;
        }
        return false;
    });

    return invalidMemberCount == 0;
}

// These all run at compile-time
constexpr Circle c1{ {0., 5.}, 100. };
static_assert(checkNonNegative(c1));

constexpr Circle c2{ {0., 5.}, -100. };
static_assert(!checkNonNegative(c2));

constexpr Circle c3{ {0., -5.}, 100. };
static_assert(!checkNonNegative(c3));
```

## Requirements
- Minimum language standard: C++17

## Usage
- refl-cpp can be included as a CMake dependecy (3.14+ required) or as a [single-header library](https://github.com/veselink1/refl-cpp/blob/master/include/refl.hpp).

## For developers
Run CMake with `-Drefl-cpp_DEVELOPER_MODE=ON` flag. You may also want to setup a custom preset for a more convenient developer experience ([see this comment](https://github.com/veselink1/refl-cpp/pull/44#issuecomment-811878328) on #44).

## Contributing
- Issue tracker - [refl-cpp/issues](https://github.com/veselink1/refl-cpp/issues)
- Source code - [refl-cpp](https://github.com/veselink1/refl-cpp)
- Contributors - [refl-cpp/contributors.md](https://github.com/veselink1/refl-cpp/blob/master/contributors.md)

## License
- MIT License (for more details, see the [license file](https://github.com/veselink1/refl-cpp/blob/master/LICENSE))
