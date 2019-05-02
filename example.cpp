// ReflCpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <any>
#include <cassert>
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
	void first_name(const std::string& value) { first_name_ = value; }

	const std::string& last_name() const { return last_name_; }
	void last_name(const std::string& value) { last_name_ = value; }
	
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
std::string serialize(const T& t)
{
	std::stringstream ss;
	refl::runtime::for_each_property(t, [&](const char* name, auto&& invoke, auto)
	{
		ss << name << "=" << invoke(t) << ";";
	});
	return ss.str();
}

int main() try
{
	using namespace std::string_literals;
	using namespace refl;
	using namespace refl::runtime;

	Point pt{};
	invoke<int>(pt, "x", 10);
	int new_x = invoke<int>(pt, "x");
	assert(new_x == 10);

	User admin(123, "Veselin", "Karaganev");
	// Example usage of refl::runtime::debug. 
	debug(std::cout, admin, false);
	std::cout << "\n";

	// Example usage of the reflection capabilities.
	assert(serialize(admin) == "id=123;firstName=Veselin;lastName=Karaganev;");

	// Example usage of refl::runtime::Proxy. 
	auto admin_proxy = make_ref_proxy(admin, [](auto member, auto& user, auto&&... args) -> auto {
		std::cout << "Call to " << member.name << " on ";
		refl::runtime::debug(std::cout, user, true);
		std::cout << "\n";
		return member.invoke(user, std::forward<decltype(args)>(args)...);
	});
	auto i = admin_proxy.id();
	auto b = admin_proxy.first_name();

	assert(TypeInfo<User>::name == "User"s);

	// Example usage of refl::runtime::invoke. 
	auto id = invoke<uint64_t>(admin, "id");
	auto id_any = invoke<std::any>(admin, "id");
	assert(id == std::any_cast<uint64_t>(id_any));

	auto name = invoke<std::string>(admin, "first_name") + " " + invoke<std::string>(admin, "last_name");
	assert(name == admin.first_name() + " " + admin.last_name());
	
	std::cout << "\n\nThe program completed successfully!\nPress any key to continue...\n";
	std::cin.get();
}
catch (const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	std::cout << "\n\nThe program was terminated!\nPress any key to continue...\n";
	std::cin.get();
}