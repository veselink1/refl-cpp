/**
 * ***README***
 * Proxies are a really powerful feature of refl-cpp.
 * They allow the user to create types which imitate the
 * interface of another type while also allowing the
 * interception of the calls to these members.
 * And the best part it that all of that is done at
 * compile-time with zero runtime overhead.
 *
 * Here we see a proxy type called value_proxy which
 * imitates the interface of a provided type T, but
 * also automatically implements the const T& memberName() const
 * and void memberName(const T&) accessors for the fields of the target.
 */
#include <iostream>
#include <cassert>
#include "refl.hpp"

// One can create a proxy by subclassing from refl::runtime::proxy<Derived, Target>
// The first template parameter is the subclassing type, the second is the target type.
template <typename T>
struct value_proxy : refl::runtime::proxy<value_proxy<T>, T>
{
    // refl::runtime::proxy allows the user to provide their own
    // storage method for the data.
    T target;

    // Provide constructors for value_proxy.
    constexpr value_proxy(const T& target)
        : target(target)
    {
    }

    constexpr value_proxy(T&& target)
        : target(std::move(target))
    {
    }

    template <typename... Args>
    constexpr value_proxy(Args&&... args)
        : target(std::forward<Args>(args)...)
    {
    }

    // static invoke_impl<Member>(Self&&, Args&&...) is the only required
    // method for proxy to work. It gets called whenever a member of value_proxy
    // is called. The result of the call is whatever is returned from invoke_impl.
    template <typename Member, typename Self, typename... Args>
    static constexpr decltype(auto) invoke_impl(Self&& self, Args&&... args)
    {
        // Create an instance of Member to support utility functions.
        constexpr Member member{};
        // Print information about which member is being called.
        std::cout << "Calling " << get_debug_name(member) << " with ";
        // Print all arguemnts provided to that method.
        refl::runtime::debug_all(std::cout, args...);
        std::cout << '\n';

        if constexpr (is_field(member)) {
            static_assert(sizeof...(Args) <= 1, "Invalid number of arguments provided for property!");
            // One argument means that this is a setter-style method.
            if constexpr (sizeof...(Args) == 1) {
                // Check if the value is writable.
                static_assert(is_writable(member));
                // Assign the value. Use identity to instruct that there is only a single argument.
                member(self.target, std::forward<Args>(args)...);
            }
            // Zero arguments means a get method.
            else {
                // Check if the value is readable.
                static_assert(is_readable(member));
                // Return a const reference to the value.
                return refl::util::make_const(member(self.target));
            }
        }
        else {
            // If the member is not a property, simply delegate the call
            // to its original implementation.
            return member(self.target, std::forward<Args>(args)...);
        }
    }
};

namespace model
{
    struct User
    {
        const long id;
        std::string email;
    };
}

REFL_AUTO(type(model::User), field(id), field(email))

// User will now have the interface { auto id(args...); auto email(args...); }
// The extact types of arguments and the return type of the members will be deduced
// from the value_proxy's invoke_impl method.
using User = value_proxy<model::User>;

int main()
{
    User user{{ 10 }};

    assert(user.id() == 10);
    // user.id(10) // fails with a static assertion (is_writable(member))

    assert(user.email().empty());
    user.email("john@example.com");
    assert(user.email() == "john@example.com");

    refl::runtime::debug(std::cout, &user.target);
    std::cout << std::endl;

    static_assert(std::is_same_v<decltype(user.email()), const std::string&>);
    static_assert(std::is_same_v<decltype(user.email("")), void>);
}
