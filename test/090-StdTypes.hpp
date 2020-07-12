#include "refl.hpp"
#include "extern/catch2/catch.hpp"

#include <stdexcept>
#include <string>
#include <memory>
#include <vector>
#include <complex>

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
        REQUIRE(runtime::debug_str(std::string("xyz")) == "\"xyz\"");
    }

    SECTION( "std::wstring" ) {
        REQUIRE(runtime::debug_str<wchar_t>(std::wstring(L"xyz")) == L"\"xyz\"");
    }

    SECTION( "std::string_view" ) {
        REQUIRE(is_reflectable<std::string_view>());
        REQUIRE(is_reflectable<std::wstring_view>());
        constexpr auto members = reflect<std::string_view>().members;
        REQUIRE(members.size >= 2); // size + data
        REQUIRE(runtime::debug_str(std::string_view("xyz")) == "\"xyz\"");
    }

    SECTION( "std::wstring_view" ) {
        REQUIRE(runtime::debug_str<wchar_t>(std::wstring_view(L"xyz")) == L"\"xyz\"");
    }

    SECTION( "std::tuple" ) {
        REQUIRE(is_reflectable<std::tuple<>>());
        REQUIRE(runtime::debug_str(std::tuple<>()) == "()");
        REQUIRE(runtime::debug_str(std::tuple<int>(5)) == "(5)");
        REQUIRE(runtime::debug_str(std::tuple<int, std::string>(5, "xyz")) == "(5, \"xyz\")");
    }

    SECTION( "std::pair" ) {
        REQUIRE(is_reflectable<std::pair<int, float>>());
        REQUIRE(runtime::debug_str(std::pair<int, std::string>(5, "xyz")) == "(5, \"xyz\")");
    }

    SECTION( "std::unique_ptr" ) {
        REQUIRE(is_reflectable<std::unique_ptr<int>>());
    }

    SECTION( "std::shared_ptr" ) {
        REQUIRE(is_reflectable<std::shared_ptr<int>>());
    }

    SECTION( "std::complex" ) {
        REQUIRE(is_reflectable<std::complex<double>>());
        REQUIRE(runtime::debug_str(std::complex<double>(5.0, 1.0)) == "5+1i");
    }

}