#include <string>
#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

TEST_CASE( "const string" ) {

    const_string<5> hello {"Hello"};

    SECTION( "basic properties" ) {
        REQUIRE( const_string<0>::npos == std::string::npos );
        REQUIRE( hello.size == 5 );
        REQUIRE( hello.c_str() == std::string("Hello") );
        REQUIRE( hello.str() == "Hello" );
    }

    SECTION( "comparison operators" ) {
        REQUIRE( hello == const_string<5>{"Hello"} );
        REQUIRE( hello != const_string<5>{"elloH"} );
        REQUIRE( hello != const_string<4>{"Hell"} );
    }

    SECTION( "creating const_strings from sized literals" ) {
        REQUIRE( make_const_string() == "" );
        REQUIRE( make_const_string("Hello") == hello );
    }

    constexpr const char* HELLO = "Hello";

    SECTION( "creating const_strings from char*" ) {
        constexpr auto str = REFL_MAKE_CONST_STRING(HELLO);
        REQUIRE( str.size == 5 );
        REQUIRE( str == "Hello" );
        REQUIRE( str != "Hell" );
    }

    SECTION( "creating substrings" ) {
        REQUIRE( make_const_string().template substr<0>() == "" );
        REQUIRE( make_const_string().template substr<0, 0>() == "" );
        REQUIRE( make_const_string().template substr<0, 50>() == "" );
        REQUIRE( make_const_string("Hello").template substr<1>() == "ello" );
        REQUIRE( make_const_string("Hello").template substr<1, 3>() == "ell" );
    }

    SECTION( "searching" ) {
        REQUIRE( make_const_string("Hello").find('e') == 1 );
        REQUIRE( make_const_string("Hello").find('l') == 2 );
        REQUIRE( make_const_string("Hello").find('w') == static_cast<size_t>(-1) );
        REQUIRE( make_const_string("Hello").find('l', 3) == 3 );

        REQUIRE( make_const_string("Hello").rfind('e') == 1 );
        REQUIRE( make_const_string("Hello").rfind('l') == 3 );
        REQUIRE( make_const_string("Hello").rfind('w') == static_cast<size_t>(-1) );
        REQUIRE( make_const_string("Hello").rfind('l', 3) == 3 );
    }

}