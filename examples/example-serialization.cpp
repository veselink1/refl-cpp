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

struct serializable : refl::attr::usage::field, refl::attr::usage::function
{
};

template <typename T>
void serialize(std::ostream& os, T&& value)
{
    for_each(refl::reflect(value).members, [&](auto member)
    {
        if constexpr (is_readable(member) && refl::descriptor::has_attribute<serializable>(member))
        {
            os << get_display_name(member) << "=" << member(value) << ";";
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

REFL_TYPE(Point, debug(debug_point), bases<>)
    REFL_FIELD(x, serializable())
    REFL_FIELD(y, serializable())
    REFL_FUNC(magnitude)
REFL_END

int main()
{
    std::cout << "Custom serialization: ";
    serialize(std::cout, Point{ 1, 1 });

    std::cout << "\nBuilt-in debug (supports arbitary containers): ";
    std::vector<Point> pts{ {0,1}, {1,0} };
	refl::runtime::debug(std::cout, pts);
    
    std::cout << std::endl;
}