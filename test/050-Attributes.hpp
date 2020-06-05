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
            using getter_x_t = trait::get_t<1, member_list<Derived>>;
            REQUIRE( is_property(getter_x_t{}) );
            REQUIRE( *get_property(getter_x_t{}).friendly_name == std::string("x") );

            using getter_y_t = trait::get_t<3, member_list<Derived>>;
            REQUIRE( is_property(getter_y_t{}) );
            REQUIRE( get_property(getter_y_t{}).friendly_name == std::nullopt );
        }

    }

}