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
        if constexpr (is_property(member) && refl::descriptor::has_attribute<serializable>(member))
        {
            os << get_display_name(member) << "=" << member(value) << ";";
        }
    });
}

template <typename T>
struct value_proxy : refl::runtime::proxy<value_proxy<T>, T>
{
    T target;

    value_proxy(const T& t)
        : target(t)
    {   
    }

    template <typename Member, typename... Args>
    decltype(auto) call_impl(Member member, Args&&... args)
    {
        std::cout << "\n\tCalling '" << get_debug_name(member) << "' on " << refl::runtime::debug_str(target, true);
        auto result = member(target, std::forward<Args>(args)...);
        std::cout << " => result is " << result << "\n";
        return result;
    }
};

struct Point
{
    typedef float value_type; 

    value_type x;
    value_type y;

    value_type magnitude() const
    {
        return std::sqrt(x * x + y * y);
    }
};

void debug_point(std::ostream& os, const Point& pt)
{
    os << "(" << pt.x << ", " << pt.y << ")";
}

REFL_TYPE(Point, debug(debug_point), bases<>)
    REFL_FIELD(x, property("X"), serializable())
    REFL_FIELD(y, property("Y"), serializable())
    REFL_FUNC(magnitude)
REFL_END

int main()
{
    std::cout << "Custom serialization: ";
    serialize(std::cout, Point{ 1, 1 });

    std::cout << "\nBuilt-in debug (supports arbitary containers): ";
    std::vector<Point> pts{ {0,1}, {1,0} };
	refl::runtime::debug(std::cout, pts);

    std::cout << "\nProxy (acts as a mediator for function calls): ";
    
    value_proxy<Point> pt_proxy{ Point{ 5, 10 } };
    pt_proxy.magnitude();
}