#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

struct NotReflected {};

namespace ns
{
    struct InNamespace
    {
        int x;
    };

    template<typename T>
    struct TemplateInNamespace
    {
        T x;
    };
}

REFL_AUTO(type(ns::InNamespace), field(x));

REFL_AUTO(template((typename T), (ns::TemplateInNamespace<T>)), field(x));

struct FooBaseBase {};

REFL_AUTO(type(FooBaseBase))

struct FooBase : FooBaseBase
{
    void m();
};

REFL_TYPE(FooBase, bases<FooBaseBase>)
    REFL_FUNC(m)
REFL_END

struct Foo : FooBase
{
    int x;
    float* y;
    const char z;

    void f() {}
    void f(int) {}
    int g(int) { return 0; }

    int m() & { return 0; }
    int m() const& { return 1; }
    int m() && { return 2; }

    static int m(int) { return 3; }
};

REFL_TYPE(Foo, bases<FooBase>)
    REFL_FIELD(x)
    REFL_FIELD(y)
    REFL_FIELD(z)

    REFL_FUNC(f)
    REFL_FUNC(g)
    REFL_FUNC(m)
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
    using namespace std::string_literals;

    SECTION( "type_descriptor" ) {
        REQUIRE( std::is_same_v<decltype(type_descriptor<int>::attributes), const std::tuple<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int*>::attributes), const std::tuple<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int* const>::attributes), const std::tuple<>> );

        REQUIRE( std::is_same_v<decltype(type_descriptor<int>::members), const type_list<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int*>::members), const type_list<>> );
        REQUIRE( std::is_same_v<decltype(type_descriptor<int* const>::members), const type_list<>> );

        REQUIRE( type_descriptor<Foo&>::name == "Foo&" );
        REQUIRE( type_descriptor<Foo&&>::name == "Foo&&" );
        REQUIRE( type_descriptor<Foo*>::name == "Foo*" );
        REQUIRE( type_descriptor<NotReflected*>::name == "(unknown)*" );
        REQUIRE( type_descriptor<NotReflected&>::name == "(unknown)&" );
        REQUIRE( type_descriptor<NotReflected&&>::name == "(unknown)&&" );
        REQUIRE( type_descriptor<ns::InNamespace>::name == "ns::InNamespace" );
        REQUIRE( type_descriptor<ns::TemplateInNamespace<int>>::name == "ns::TemplateInNamespace<T>" );

        REQUIRE( type_descriptor<Foo>::name == "Foo" );
        REQUIRE( type_descriptor<Foo>::members.size > 0 );
        REQUIRE( type_descriptor<const Foo>::members.size > 0 );
        REQUIRE( type_descriptor<volatile Foo>::members.size > 0 );
        REQUIRE( type_descriptor<const volatile Foo>::members.size > 0 );

        REQUIRE( type_descriptor<Foo&>::members.size == 0 );
        REQUIRE( type_descriptor<const Foo&>::members.size == 0 );
        REQUIRE( type_descriptor<Foo&&>::members.size == 0 );
        REQUIRE( type_descriptor<const Foo&&>::members.size == 0 );

        REQUIRE( get_name(type_descriptor<signed int>{}) == "signed int" );
        REQUIRE( get_name(type_descriptor<const Foo&&>{}) == "Foo&&" );
        REQUIRE( get_name(type_descriptor<ns::InNamespace>{}) == "ns::InNamespace" );
        REQUIRE( get_name(type_descriptor<ns::TemplateInNamespace<int>>{}) == "ns::TemplateInNamespace<T>" );

        REQUIRE( get_attributes(type_descriptor<signed int>{}) == std::tuple<>{} );
        REQUIRE( util::contains_instance<attr::base_types>(get_attributes(type_descriptor<Foo>{})) );

        REQUIRE( get_simple_name(type_descriptor<signed int>{}) == "signed int" );
        REQUIRE( get_simple_name(type_descriptor<const Foo&&>{}) == "Foo&&" );
        REQUIRE( get_simple_name(type_descriptor<ns::InNamespace>{}) == "InNamespace" );
        REQUIRE( get_simple_name(type_descriptor<ns::TemplateInNamespace<int>>{}) == "TemplateInNamespace" );

        REQUIRE( get_debug_name(trait::get_t<0, member_list<ns::InNamespace>>{}) == "ns::InNamespace::x"s );
        REQUIRE( get_debug_name(trait::get_t<0, member_list<ns::TemplateInNamespace<int>>>{}) == "ns::TemplateInNamespace<T>::x"s );

        REQUIRE( get_debug_name_const(trait::get_t<0, member_list<ns::InNamespace>>{}) == "ns::InNamespace::x" );
        REQUIRE( get_debug_name_const(trait::get_t<0, member_list<ns::TemplateInNamespace<int>>>{}) == "ns::TemplateInNamespace<T>::x" );
    }

    SECTION( "type_descriptor inheritance" ) {
        REQUIRE( get_declared_base_types(type_descriptor<FooBaseBase>{}).size == 0 );
        REQUIRE( type_descriptor<FooBaseBase>::declared_base_types::size == 0 );
        REQUIRE( get_base_types(type_descriptor<FooBaseBase>{}).size == 0 );
        REQUIRE( type_descriptor<FooBaseBase>::base_types::size == 0 );

        REQUIRE( get_declared_base_types(type_descriptor<FooBase>{}).size == 1 );
        REQUIRE( type_descriptor<FooBase>::declared_base_types::size == 1 );
        REQUIRE( get_base_types(type_descriptor<FooBase>{}).size == 1 );
        REQUIRE( type_descriptor<FooBase>::base_types::size == 1 );

        REQUIRE( get_declared_base_types(type_descriptor<Foo>{}).size == 1 );
        REQUIRE( type_descriptor<Foo>::declared_base_types::size == 1 );
        REQUIRE( get_base_types(type_descriptor<Foo>{}).size == 2 );
        REQUIRE( type_descriptor<Foo>::base_types::size == 2 );

        REQUIRE( get_members(type_descriptor<Foo>{}).size == get_declared_members(type_descriptor<Foo>{}).size + get_declared_members(type_descriptor<FooBase>{}).size );
        REQUIRE( type_descriptor<Foo>::member_types::size == type_descriptor<Foo>::declared_member_types::size + type_descriptor<FooBase>::member_types::size );

        REQUIRE( get_members(type_descriptor<ShadowingBase>{}).size == 3 );
        REQUIRE( type_descriptor<ShadowingBase>::member_types::size == 3 );
        REQUIRE( get_declared_members(type_descriptor<ShadowingDerived>{}).size == 2 );
        REQUIRE( type_descriptor<ShadowingDerived>::declared_member_types::size == 2 );
        REQUIRE( get_members(type_descriptor<ShadowingDerived>{}).size == 5 );
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
        REQUIRE( field_descriptor<Foo, 0>::name == "x" );
        REQUIRE( field_descriptor<Foo, 0>::pointer == &Foo::x );
        REQUIRE( !field_descriptor<Foo, 0>::is_static );

        constexpr auto y_member = trait::get_t<1, member_list<Foo>>{};
        REQUIRE( is_field(y_member) );
        REQUIRE( !is_function(y_member) );
        REQUIRE( get_display_name(y_member) == "y"s );
        REQUIRE( get_display_name_const(y_member) == "y" );
        REQUIRE( get_debug_name(y_member) == "Foo::y"s );
        REQUIRE( get_debug_name_const(y_member) == "Foo::y" );
        REQUIRE( std::is_same_v<decltype(get_reader(y_member)), std::remove_cv_t<decltype(y_member)>> );
        REQUIRE( std::is_same_v<decltype(get_writer(y_member)), std::remove_cv_t<decltype(y_member)>> );
        REQUIRE( invoke(y_member, Foo{}) == 0 );
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
        REQUIRE( !is_resolved(f_td{}) );
        REQUIRE( !f_td::is_resolved );

#if defined(__clang__) || defined(_MSC_VER)
        // gcc has bug which prevents SFIANE for kicking in for resolve (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=71487)
        REQUIRE( !descriptor::can_resolve<int(Foo::*)(Foo)>(f_td{}) );
        REQUIRE( !f_td::can_resolve<int(Foo::*)(Foo)>() );
        REQUIRE( descriptor::resolve<void(Foo::*)()>(f_td{}) != nullptr );
        REQUIRE( f_td::resolve<void(Foo::*)()>() != nullptr );
        REQUIRE( descriptor::can_resolve<void(Foo::*)()>(f_td{}) );
        REQUIRE( f_td::can_resolve<void(Foo::*)()>() );
#endif

        using g_td = trait::get_t<4, member_list<Foo>>;
        REQUIRE( g_td::is_resolved );
        REQUIRE( is_resolved(g_td{}) );
        REQUIRE( get_pointer(g_td{}) != nullptr );
        REQUIRE( g_td::pointer != nullptr );

        using m_td = trait::get_t<5, member_list<Foo>>;
        REQUIRE( !m_td::is_resolved );
        REQUIRE( !is_resolved(m_td{}) );
        REQUIRE( get_pointer(m_td{}) == nullptr );
        REQUIRE( m_td::pointer == nullptr );

        Foo foo{};
        REQUIRE( m_td{}((Foo&)foo) == 0 );
        REQUIRE( m_td{}((const Foo&)foo) == 1 );
        REQUIRE( m_td{}((Foo&&)Foo{}) == 2 );
        REQUIRE( m_td{}(0) == 3 );
    }

}