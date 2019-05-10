#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <any>
#include <cassert>
#include <unordered_map>
#include "refl.hpp"

class User
{
public:

    User(uint64_t id, const std::string& first_name, const std::string& last_name)
        : id_(id)
        , first_name_(first_name)
        , last_name_(last_name)
    {
    }

    uint64_t id() const { return id_; }

    const std::string& first_name() const { return first_name_; }
    void first_name(std::string_view value) { first_name_ = value; }

    const std::string& last_name() const { return last_name_; }
    void last_name(std::string_view value) { last_name_ = value; }
    
private:

    uint64_t id_;
    std::string first_name_;
    std::string last_name_;

};

REFL_TYPE(User)
    REFL_FUNC(id, (refl::attr::Property{}))
    REFL_FUNC(first_name, (refl::attr::Property{ "firstName" }))
    REFL_FUNC(last_name, (refl::attr::Property{ "lastName" }))
REFL_END

struct Point
{
    int x;
    int y;
};

REFL_TYPE(Point)
    REFL_FIELD(x)
    REFL_FIELD(y)
REFL_END

template <typename T>
std::string serialize(const T& t);

template <typename TMap, typename TValue, typename TValueCreator>
TMap to_map(const TValue& t, TValueCreator&& v);

template <typename TMap, typename TValue>
TMap to_map(const TValue& t);

void showcase_runtime_invoke_point()
{
    Point pt{};
    // refl::runtime::invoke works with both fields and functions.
    // All invocations are checked at *compile-time*.
    refl::runtime::invoke<int>(pt, "x", 10); 
    assert(pt.x == 10);

    int new_x = refl::runtime::invoke<int>(pt, "x");
    assert(new_x == 10);

    // refl::runtime::invoke<std::string>(pt, "x", 10); <-- this will fail at compile-time
    // refl::runtime::invoke<int>(pt, "x", std::string{ "Hi!" }); <-- and so will this
}

void showcase_custom_converter_point()
{
    Point pt{};
    auto pt_map = to_map<std::unordered_map<std::string, int>>(pt);
    auto pt_map_exp = std::unordered_map<std::string, int>{
        {"x", 0},
        {"y", 0},
    };
    assert(pt_map == pt_map_exp);
}

void showcase_custom_converter_user()
{
    User admin(123, "Veselin", "Karaganev");

	auto create_value = [](auto&& x) 
	{
		std::stringstream ss;
		ss << x;
		return ss.str();
	};
    auto admin_map = to_map<std::unordered_map<std::string, std::string>>(admin, create_value);

    auto admin_map_exp = std::unordered_map<std::string, std::string>{
        { "id", "123" },
        { "firstName", "Veselin" },
        { "lastName", "Karaganev" },
    };
    assert(admin_map == admin_map_exp);
}

void showcase_custom_serialization_user()
{
    User admin(123, "Veselin", "Karaganev");
    assert(serialize(admin) == "id=123;firstName=Veselin;lastName=Karaganev;");
}

void showcase_debug()
{
    User admin(123, "Veselin", "Karaganev");
    refl::runtime::debug(std::cout, admin, false);
    // Prints { id = 123, firstName = "Veselin", lastName = "Karaganev" }
    std::cout << "\n";
}

void showcase_proxy()
{
    User admin(123, "Veselin", "Karaganev");

    auto admin_proxy = refl::runtime::make_ref_proxy(admin, [](auto member, auto& user, auto&& ... args) -> auto {
        std::cout << "Call to " << member.name << " on ";
        refl::runtime::debug(std::cout, user, true);
        std::cout << "\n";
        return member.invoke(user, std::forward<decltype(args)>(args)...);
    });
    auto i = admin_proxy.id();
    auto b = admin_proxy.first_name();
}

int main() try
{
    showcase_runtime_invoke_point();
    showcase_custom_converter_point();

    showcase_custom_converter_user();
    showcase_custom_serialization_user();

    showcase_debug();
    showcase_proxy();
    
    std::cout << "\n\nThe program completed successfully!\nPress any key to continue...\n";
    std::cin.get();
}
catch (const std::exception& e)
{
	std::cerr << '\n';
	refl::runtime::debug(std::cerr, e);
    std::cout << "\n\nThe program was terminated!\nPress any key to continue...\n";
    std::cin.get();
}


template <typename T>
std::string serialize(const T& t)
{
    std::stringstream ss;
    refl::runtime::for_each_property(t, [&](const char* name, auto && invoke, auto)
    {
        ss << name << "=" << invoke(t) << ";";
    });
    return ss.str();
}

template <typename TMap, typename TValue, typename TValueCreator>
TMap to_map(const TValue& t, TValueCreator&& v)
{
    TMap map{};
    refl::runtime::for_each_property(t, [&](auto name, auto && invoke, auto) {
        map.emplace(name, v(invoke(t)));
    });
    refl::runtime::for_each_field(t, [&](auto name, const auto & value, auto) {
        map.emplace(name, v(value));
    });
    return map;
}

template <typename TMap, typename TValue>
TMap to_map(const TValue& t)
{
    return to_map<TMap>(t, [](auto && x) -> decltype(auto) { return x; });
}