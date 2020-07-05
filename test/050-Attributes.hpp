#include <sstream>
#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

struct Base {};
REFL_AUTO(type(Base));

struct Derived : public Base {
    int x;

    int get_x() const { return x; }
    void set_x(int value) { x = value; }
    int y() const { return 0; }
};

constexpr auto custom_print = [](auto& os, auto value) { os << value << "custom";  };

REFL_AUTO(
    type(Derived, bases<Base>),
    field(x, debug{ custom_print }),
    func(get_x, property{ "x" }),
    func(set_x, property{ "x" }),
    func(y, property{ })
)

struct NormalizedProps {
    int getfoo() const { return 0; }
    int get_foo() const { return 0; }
    int getFoo() const { return 0; }
    int get_Foo() const { return 0; }
    int Getfoo() const { return 0; }
    int Get_foo() const { return 0; }
    int GetFoo() const { return 0; }
    int Get_Foo() const { return 0; }
};

REFL_AUTO(
    type(NormalizedProps),
    func(getfoo, property()),
    func(get_foo, property()),
    func(getFoo, property()),
    func(get_Foo, property()),
    func(Getfoo, property()),
    func(Get_foo, property()),
    func(GetFoo, property()),
    func(Get_Foo, property())
)

TEST_CASE( "attributes" ) {

    SECTION( "usage tags" ) {
        REQUIRE( std::is_base_of_v<attr::usage::field, attr::usage::member> );
        REQUIRE( std::is_base_of_v<attr::usage::function, attr::usage::member> );
        REQUIRE( std::is_base_of_v<attr::usage::member, attr::usage::any> );
        REQUIRE( std::is_base_of_v<attr::usage::type, attr::usage::any> );
    }

    SECTION( "built-in attributes" ) {

        SECTION( "bases<>" ) {
            using bases_t = decltype(get_bases(reflect<Derived>()));
            REQUIRE( bases_t::size == 1 );
            REQUIRE( std::is_same_v<trait::first_t<bases_t>::type, Base> );
        }

        SECTION( "debug<F>" ) {
            using field_t = trait::first_t<member_list<Derived>>;
            constexpr auto debug = descriptor::get_attribute<attr::debug>(field_t{});

            std::stringstream ss;
            debug.write(ss, 0);
            REQUIRE( ss.str() == "0custom" );
        }

        SECTION( "property" ) {
            using namespace std::string_literals;

            using getter_x_t = trait::get_t<1, member_list<Derived>>;
            REQUIRE( is_property(getter_x_t{}) );
            REQUIRE( *get_property(getter_x_t{}).friendly_name == std::string("x") );

            using getter_y_t = trait::get_t<3, member_list<Derived>>;
            REQUIRE( is_property(getter_y_t{}) );
            REQUIRE( get_property(getter_y_t{}).friendly_name == std::nullopt );

            REQUIRE( get_display_name(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "getfoo"; })) == "getfoo"s );
            REQUIRE( get_display_name(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "Getfoo"; })) == "Getfoo"s );
            REQUIRE( get_display_name(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "Get_foo"; })) == "Get_foo"s );
            REQUIRE( get_display_name(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "Get_Foo"; })) == "Get_Foo"s );
            REQUIRE( get_display_name(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "get_Foo"; })) == "get_Foo"s );

            REQUIRE( get_display_name(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "get_foo"; })) == "foo"s );
            REQUIRE( get_display_name(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "getFoo"; })) == "foo"s );
            REQUIRE( get_display_name(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "GetFoo"; })) == "Foo"s );

            REQUIRE( get_display_name_const(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "get_foo"; })) == "foo" );
            REQUIRE( get_display_name_const(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "getFoo"; })) == "foo" );
            REQUIRE( get_display_name_const(find_one(member_list<NormalizedProps>(), [](auto m) { return m.name == "GetFoo"; })) == "Foo" );

            constexpr auto get_x = find_one(member_list<Derived>(), [](auto m) { return m.name == "get_x"; });
            constexpr auto set_x = find_one(member_list<Derived>(), [](auto m) { return m.name == "set_x"; });

            REQUIRE( std::is_same_v<decltype(get_reader(get_x)), std::remove_cv_t<decltype(get_x)>> );
            REQUIRE( std::is_same_v<decltype(get_writer(get_x)), std::remove_cv_t<decltype(set_x)>> );

            REQUIRE( std::is_same_v<decltype(get_reader(set_x)), std::remove_cv_t<decltype(get_x)>> );
            REQUIRE( std::is_same_v<decltype(get_writer(set_x)), std::remove_cv_t<decltype(set_x)>> );
        }

    }

}