#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

struct FooBaseBase {};

REFL_AUTO(type(FooBaseBase))

struct FooBase {
    void m();
};

REFL_TYPE(FooBase, bases<FooBaseBase>)
    REFL_FUNC(m)
REFL_END

struct Foo {
    int x;
    float* y;
    const char z;

    void f() {}
    void f(int) {}
    int g(int) { return 0; }
};

REFL_TYPE(Foo, bases<FooBase>)
    REFL_FIELD(x)
    REFL_FIELD(y)
    REFL_FIELD(z)

    REFL_FUNC(f)
    REFL_FUNC(g)
REFL_END

struct ShadowingBase {
    int baz = 0;
    int bar = 0;
    int foo() { return 0; }
};

REFL_AUTO(type(ShadowingBase), func(foo), field(bar), field(baz))

struct ShadowingDerived : public ShadowingBase {
    int bar = 1;
    int foo() { return 1; }
};

REFL_AUTO(type(ShadowingDerived, bases<ShadowingBase>), func(foo), field(bar))

TEST_CASE( "descriptors" ) {

    SECTION( "type_descriptor" ) {
        REQUIRE( std::is_same_v<decltype(type_descriptor<int>::attributes), const std::tuple<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int*>::attributes), const std::tuple<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int* const>::attributes), const std::tuple<>> );

        REQUIRE( std::is_same_v<decltype(type_descriptor<int>::members), const type_list<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int*>::members), const type_list<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int* const>::members), const type_list<>> );

        REQUIRE( type_descriptor<Foo>::name == "Foo" );
    }

    SECTION( "type_descriptor inheritance" ) {
        REQUIRE( type_descriptor<FooBaseBase>::declared_base_types::size == 0 );
        REQUIRE( type_descriptor<FooBaseBase>::base_types::size == 0 );

        REQUIRE( type_descriptor<FooBase>::declared_base_types::size == 1 );
        REQUIRE( type_descriptor<FooBase>::base_types::size == 1 );

        REQUIRE( type_descriptor<Foo>::declared_base_types::size == 1 );
        REQUIRE( type_descriptor<Foo>::base_types::size == 2 );

        REQUIRE( type_descriptor<Foo>::member_types::size == type_descriptor<Foo>::declared_member_types::size + type_descriptor<FooBase>::member_types::size );

        REQUIRE( type_descriptor<ShadowingBase>::member_types::size == 3 );
        REQUIRE( type_descriptor<ShadowingDerived>::declared_member_types::size == 2 );
        REQUIRE( type_descriptor<ShadowingDerived>::member_types::size == 5 );

        ShadowingDerived instance;
        for_each(type_descriptor<ShadowingDerived>::members, [&](auto member) {
            if constexpr (std::is_same_v<typename decltype(member)::declaring_type, ShadowingDerived>) {
                REQUIRE( member(instance) == 1 );
            } else {
                REQUIRE( member(instance) == 0 );
            }
        });
    }

    SECTION( "field_descriptor" ) {
        using namespace std::string_literals;
        REQUIRE( field_descriptor<Foo, 0>::name == "x" );
        REQUIRE( field_descriptor<Foo, 0>::pointer == &Foo::x );
        REQUIRE( !field_descriptor<Foo, 0>::is_static );

        constexpr auto y_member = trait::get_t<1, member_list<Foo>>{};
        REQUIRE( is_field(y_member) );
        REQUIRE( !is_function(y_member) );
        REQUIRE( get_display_name(y_member) == "y"s );
        REQUIRE( get_debug_name(y_member) == "Foo::y"s );
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