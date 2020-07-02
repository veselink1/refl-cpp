/**
 * ***README***
 * In this very simple example we define a custom attribute
 * called serializable and our own serialization routine which
 * detects the presence of this attribute and selectively serializes
 * the members of the provided value.
 */
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <any>
#include <cassert>
#include <unordered_map>
#include <cmath>
#include "refl.hpp"

// define an empty maker attribute to be used on fields and functions only
struct serializable : refl::attr::usage::field, refl::attr::usage::function
{
};

template <typename T>
void serialize(std::ostream& os, T&& value)
{
    // iterate over the members of T
    for_each(refl::reflect(value).members, [&](auto member)
    {
        // is_readable checks if the member is a non-const field
        // or a 0-arg const-qualified function marked with property attribute
        if constexpr (is_readable(member) && refl::descriptor::has_attribute<serializable>(member))
        {
            // get_display_name prefers the friendly_name of the property over the function name
            os << get_display_name(member) << "=";
            // member(value) returns a reference to the field or calls the property accessor
            os << member(value) << ";";
        }
    });
}

struct Point
{
    float x;
    float y;

    float magnitude() const
    {
        return std::sqrt(x * x + y * y);
    }
};

void debug_point(std::ostream& os, const Point& pt)
{
    os << "(" << pt.x << ", " << pt.y << ")";
}

// the built-in debug<F> attribute can take a function object to be
// used for printing the debug output for the type/field/property
// bases<Ts...> is a template value attribute which can be used to
// specify the base types of a class.
REFL_TYPE(Point, debug(debug_point), bases<>)
    REFL_FIELD(x, serializable()) // here we use serializable only as a maker
    REFL_FIELD(y, serializable())
    REFL_FUNC(magnitude)
REFL_END

int main()
{
    std::cout << "Custom serialization: ";
    serialize(std::cout, Point{ 1, 1 });

    std::cout << "\nBuilt-in debug (supports arbitrary containers): ";
    std::vector<Point> pts{ {0,1}, {1,0} };

    // debug will print the debug representation of the value to the stream;
    // for types with begin() and end(), refl-cpp will automatically
    // interate over the target value and print the values as well.
	refl::runtime::debug(std::cout, pts);

    std::cout << std::endl;
}