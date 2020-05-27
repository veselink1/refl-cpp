#include "refl.hpp"
#include "extern/catch2/catch.hpp"

#include <stdexcept>
#include <string>
#include <memory>
#include <vector>

using namespace refl;

TEST_CASE( "built-in support for standard types" ) {

    SECTION( "std::exception" ) {
        REQUIRE(is_reflectable<std::exception>());
        constexpr auto members = reflect<std::exception>().members;
        REQUIRE(members.size >= 1); // what
    }

    SECTION( "std::string" ) {
        REQUIRE(is_reflectable<std::string>());
        REQUIRE(is_reflectable<std::wstring>());
        constexpr auto members = reflect<std::string>().members;
        REQUIRE(members.size >= 2); // size + data
    }

    SECTION( "std::string_view" ) {
        REQUIRE(is_reflectable<std::string_view>());
        REQUIRE(is_reflectable<std::wstring_view>());
        constexpr auto members = reflect<std::string_view>().members;
        REQUIRE(members.size >= 2); // size + data
    }

    SECTION( "std::tuple" ) {
        REQUIRE(is_reflectable<std::tuple<>>());
    }

    SECTION( "std::pair" ) {
        REQUIRE(is_reflectable<std::pair<int, float>>());
    }

    SECTION( "std::unique_ptr" ) {
        REQUIRE(is_reflectable<std::unique_ptr<int>>());
    }

    SECTION( "std::shared_ptr" ) {
        REQUIRE(is_reflectable<std::shared_ptr<int>>());
    }

}