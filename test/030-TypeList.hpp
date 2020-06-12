#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

TEST_CASE( "type list" ) {

    SECTION( "basic properties" ) {
        REQUIRE( type_list<>::size == 0 );
        REQUIRE( type_list<int>::size == 1 );
        REQUIRE( type_list<int, float>::size == 2 );

        REQUIRE( type_tag<int>::size == 1 );
        REQUIRE( std::is_same_v<type_tag<int>, type_list<int>> );
        REQUIRE( std::is_same_v<type_tag<int>::type, int> );
    }

}