#include <vector>
#include <string>
#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

struct non_reflectable {};

struct A {
    int x;
    void f() {}
};

REFL_AUTO(
    type(A),
    field(x),
    func(f)
)

TEST_CASE( "type traits" ) {

    SECTION( "remove_qualifiers" ) {
        REQUIRE( std::is_same_v<trait::remove_qualifiers_t<const int&>, int> );
        REQUIRE( std::is_same_v<typename trait::remove_qualifiers<const int&>::type, int> );
    }

    SECTION( "is_reflectable" ) {
        REQUIRE( trait::is_reflectable<int>::value );
        REQUIRE( trait::is_reflectable_v<int> );
        REQUIRE( !trait::is_reflectable_v<non_reflectable> );
    }

    SECTION( "is_container" ) {
        REQUIRE( trait::is_container_v<std::vector<int>> );
        REQUIRE( trait::is_container_v<std::string> );
        REQUIRE( !trait::is_container_v<int> );
        REQUIRE( !trait::is_container_v<non_reflectable> );
    }

    SECTION( "type_list basic operations" ) {
        REQUIRE( std::is_same_v<trait::get_t<0, type_list<int, float>>, int> );
        REQUIRE( std::is_same_v<trait::get_t<1, type_list<int, float>>, float> );

        REQUIRE( std::is_same_v<trait::first_t<type_list<int, float>>, int> );
        REQUIRE( std::is_same_v<trait::first_t<type_list<int, float>>, int> );
        REQUIRE( std::is_same_v<trait::last_t<type_list<int, float>>, float> );

        REQUIRE( std::is_same_v<trait::tail_t<type_list<int, float, double>>, type_list<float, double>> );
        REQUIRE( std::is_same_v<trait::init_t<type_list<int, float, double>>, type_list<int, float>> );

        REQUIRE( std::is_same_v<trait::append_t<int, type_list<float>>, type_list<float, int>> );
        REQUIRE( std::is_same_v<trait::prepend_t<int, type_list<float>>, type_list<int, float>> );
    }

    SECTION( "type_list reverse" ) {
        REQUIRE( std::is_same_v<trait::reverse_t<type_list<>>, type_list<>> );
        REQUIRE( std::is_same_v<trait::reverse_t<type_list<int>>, type_list<int>> );
        REQUIRE( std::is_same_v<trait::reverse_t<type_list<int, float, double>>, type_list<double, float, int>> );
    }

    SECTION( "type_list concat" ) {
        REQUIRE( std::is_same_v<trait::concat_t<type_list<>, type_list<>>, type_list<>> );
        REQUIRE( std::is_same_v<trait::concat_t<type_list<int, float>, type_list<>>, type_list<int, float>> );
        REQUIRE( std::is_same_v<trait::concat_t<type_list<>, type_list<int, float>>, type_list<int, float>> );
        REQUIRE( std::is_same_v<trait::concat_t<type_list<int, float>, type_list<float, int>>, type_list<int, float, float, int>> );
        REQUIRE( std::is_same_v<trait::concat_t<type_list<int, float>, type_list<>, type_list<int, float>>, type_list<int, float, int, float>> );
    }

    SECTION( "type_list get" ) {
        REQUIRE( std::is_same_v<trait::concat_t<type_list<>, type_list<>>, type_list<>> );
        REQUIRE( std::is_same_v<trait::concat_t<type_list<int, float>, type_list<>>, type_list<int, float>> );
        REQUIRE( std::is_same_v<trait::concat_t<type_list<>, type_list<int, float>>, type_list<int, float>> );
        REQUIRE( std::is_same_v<trait::concat_t<type_list<int, float>, type_list<float, int>>, type_list<int, float, float, int>> );
    }

    SECTION( "type_list skip" ) {
        REQUIRE( std::is_same_v<trait::skip_t<1, type_list<int, float>>, type_list<float>> );
        REQUIRE( std::is_same_v<trait::skip_t<2, type_list<int, float>>, type_list<>> );
    }

    SECTION( "type_list filter" ) {
        REQUIRE( std::is_same_v<trait::filter_t<std::is_integral, type_list<>>, type_list<>> );
        REQUIRE( std::is_same_v<trait::filter_t<std::is_integral, type_list<int>>, type_list<int>> );
        REQUIRE( std::is_same_v<trait::filter_t<std::is_integral, type_list<float>>, type_list<>> );
        REQUIRE( std::is_same_v<trait::filter_t<std::is_integral, type_list<int, float>>, type_list<int>> );
    }

    SECTION( "type_list map" ) {
        REQUIRE( std::is_same_v<trait::map_t<std::remove_reference, type_list<>>, type_list<>> );
        REQUIRE( std::is_same_v<trait::map_t<std::remove_reference, type_list<int&>>, type_list<int>> );
        REQUIRE( std::is_same_v<trait::map_t<std::remove_reference, type_list<int&, float&>>, type_list<int, float>> );
        REQUIRE( !std::is_same_v<trait::map_t<std::remove_reference, type_list<float&>>, type_list<int>> );
    }

    SECTION( "descriptor type checks" ) {
        REQUIRE( !trait::is_member_v<int> );
        REQUIRE( trait::is_member_v<trait::get_t<0, member_list<A>>> );

        REQUIRE( !trait::is_field_v<int> );
        REQUIRE( trait::is_field_v<trait::get_t<0, member_list<A>>> );

        REQUIRE( !trait::is_function_v<int> );
        REQUIRE( trait::is_function_v<trait::get_t<1, member_list<A>>> );
    }

    SECTION( "is_instance" ) {
        REQUIRE( trait::is_instance_v<std::tuple<>> );
        REQUIRE( !trait::is_instance_v<int> );

        REQUIRE( trait::is_instance_of_v<std::tuple, std::tuple<>> );
        REQUIRE( !trait::is_instance_of_v<std::tuple, int> );
        REQUIRE( !trait::is_instance_of_v<std::vector, std::tuple<>> );
    }

    SECTION( "contains" ) {
        REQUIRE( trait::contains_v<int, type_list<int, float>>);
        REQUIRE( !trait::contains_v<int, type_list<float, double>>);

        REQUIRE( trait::index_of_v<int, type_list<int, float>> == 0 );
        REQUIRE( trait::index_of_v<int, type_list<float, double>> == -1 );
    }

    SECTION( "contains_instance" ) {
        REQUIRE( trait::contains_instance_v<std::tuple, type_list<int, std::tuple<float>>> );
        REQUIRE( !trait::contains_instance_v<std::tuple, type_list<int, float>> );
        REQUIRE( !trait::contains_instance_v<attr::debug, trait::get_t<0, member_list<A>>::attribute_types> );

        REQUIRE( trait::index_of_instance_v<std::tuple, type_list<int, std::tuple<float>>> == 1 );
        REQUIRE( trait::index_of_instance_v<std::tuple, type_list<int, float>> == -1 );
        REQUIRE( trait::index_of_instance_v<attr::debug, trait::get_t<0, member_list<A>>::attribute_types> == -1 );
    }

    SECTION( "contains_base" ) {
        struct B : A {};

        REQUIRE( trait::contains_base_v<A, type_list<int, A>> );
        REQUIRE( trait::contains_base_v<A, type_list<int, B>> );
        REQUIRE( !trait::contains_base_v<A, type_list<int, float>> );

        REQUIRE( trait::index_of_base_v<A, type_list<int, A>> == 1 );
        REQUIRE( trait::index_of_base_v<A, type_list<int, B>> == 1 );
        REQUIRE( trait::index_of_base_v<A, type_list<int, float>> == -1 );
    }

}