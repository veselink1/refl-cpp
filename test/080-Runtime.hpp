#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

struct Bar {
    int x;
    float* y;
    const char z;

    void f() {}
    void f(int) {}
    int g(int) { return 0; }
};

REFL_TYPE(Bar)
    REFL_FIELD(x)
    REFL_FIELD(y)
    REFL_FIELD(z)

    REFL_FUNC(f)
    REFL_FUNC(g)
REFL_END

struct BarBar {
    Bar bar{};
    void* vp{nullptr};
    int* ip = new int(0);
    std::string str = "Hi!";
    std::vector<Bar> bars{ {}, {} };
};

REFL_TYPE(BarBar)
    REFL_FIELD(bar)
    REFL_FIELD(vp)
    REFL_FIELD(ip)
    REFL_FIELD(str)
    REFL_FIELD(bars)
REFL_END

template <typename T>
struct dummy_proxy : refl::runtime::proxy<dummy_proxy<T>, T>
{
    constexpr dummy_proxy() {}

    template <typename Member>
    static constexpr int invoke_impl(...)
    {
        return 0;
    }
};

TEST_CASE( "runtime utils" ) {

    SECTION( "proxies" ) {
        dummy_proxy<Bar> pa;
        REQUIRE( pa.y() == 0 );
        REQUIRE( pa.f(1) == 0 );
        REQUIRE( util::make_const(pa).f(1) == 0 );

        REQUIRE( trait::is_proxy_v<dummy_proxy<Bar>> );
        REQUIRE( !trait::is_proxy_v<int> );

        dummy_proxy<ShadowingBase> sb;
        dummy_proxy<ShadowingDerived> sd;

        ShadowingDerived{}.foo();
        sd.foo();
        sd.baz();
    }

    SECTION( "debug" ) {
        REQUIRE( runtime::debug_str(Bar{}).find("x = ") != std::string::npos );
        REQUIRE( runtime::debug_str(Bar{}).find("y = ") != std::string::npos );
        REQUIRE( runtime::debug_str(Bar{}).find("y = ") != std::string::npos );
    }

    SECTION( "invoke" ) {
        REQUIRE( runtime::invoke<int>(Bar{}, "x", 1) == 1 );
    }

}