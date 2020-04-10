#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

struct Foo {
    int x;
    float* y;
    const char z;

    void f() {}
    void f(int) {}
    int g(int) { return 0; }
};

REFL_TYPE(Foo)
    REFL_FIELD(x)
    REFL_FIELD(y)
    REFL_FIELD(z)

    REFL_FUNC(f)
    REFL_FUNC(g)
REFL_END

TEST_CASE( "descriptors" ) {

    SECTION( "type_descriptor" ) {
        REQUIRE( std::is_same_v<decltype(type_descriptor<int>::attributes), const std::tuple<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int*>::attributes), const std::tuple<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int* const>::attributes), const std::tuple<>> );

        REQUIRE( std::is_same_v<decltype(type_descriptor<int>::members), const type_list<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int*>::members), const type_list<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int* const>::members), const type_list<>> );
    }

    SECTION( "field_descriptor" ) {
        REQUIRE( field_descriptor<Foo, 0>::name == "x" );
        REQUIRE( field_descriptor<Foo, 0>::pointer == &Foo::x );
        REQUIRE( !field_descriptor<Foo, 0>::is_static );

        constexpr auto y_member = trait::get_t<1, member_list<Foo>>{};
        REQUIRE( is_field(y_member) );
        REQUIRE( !is_function(y_member) );
        REQUIRE( strcmp(get_display_name(y_member), "y") == 0 );
        REQUIRE( strcmp(get_debug_name(y_member), "Foo::y") == 0 );
        REQUIRE( y_member(Foo{}) == 0 );
        REQUIRE( std::is_same_v<trait::get_t<3, member_list<Foo>>::return_type<Foo, int>, void> );
        REQUIRE( std::is_invocable_v<trait::get_t<0, member_list<Foo>>, Foo, int> );
    }

    SECTION( "function_descriptor" ) {
        REQUIRE( function_descriptor<Foo, 3>::name == "f" );
        REQUIRE( std::is_invocable_v<function_descriptor<Foo, 3>, Foo> );
        REQUIRE( std::is_invocable_v<function_descriptor<Foo, 3>, Foo, int> );
        REQUIRE( !std::is_invocable_v<function_descriptor<Foo, 3>, Foo, std::string> );

        using f_td = trait::get_t<3, member_list<Foo>>;
        static_assert(!f_td::is_resolved);
        [[maybe_unused]] auto f_ptr = f_td::resolve<void(Foo::*)()>;

        using g_td = trait::get_t<4, member_list<Foo>>;
        static_assert(g_td::is_resolved);
        [[maybe_unused]] auto g_ptr = g_td::pointer;
    }

}