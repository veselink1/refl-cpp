#include "refl.hpp"
#include "extern/catch2/catch.hpp"

using namespace refl;

TEST_CASE( "utilities" ) {

    SECTION( "to_array" ) {
        REQUIRE( std::is_same_v<decltype(util::to_array<int>(std::make_tuple(0, 0))), std::array<int, 2>> );
        REQUIRE( std::is_same_v<decltype(util::to_array<int>(std::tuple<>{})), std::array<int, 0>> );
    }

    SECTION( "map_to_array" ) {
        constexpr std::array<int, 2> mta = util::map_to_array<int>(type_list<int, float>{}, [](auto) {
            return 5;
        });
        REQUIRE( mta[0] == 5 );
        REQUIRE( mta[1] == 5 );

        constexpr std::array<int, 2> mta2 = util::map_to_array<int>(type_list<int, float>{}, [](auto, size_t i) {
            return static_cast<int>(i);
        });
        REQUIRE( mta2[0] == 0 );
        REQUIRE( mta2[1] == 1 );
    }

    SECTION( "to_tuple" ) {
        REQUIRE( std::is_same_v<decltype(util::to_tuple(std::array<int, 2>{})), std::tuple<int, int>> );
        REQUIRE( std::is_same_v<decltype(util::to_tuple(std::array<int, 0>{})), std::tuple<>> );
    }

    SECTION( "map_to_tuple" ) {
        constexpr std::tuple<int, float> mtt = util::map_to_tuple(type_list<int, float>{}, [](auto x) {
            return decltype(x)(5);
        });
        REQUIRE( std::get<0>(mtt) == 5 );
        REQUIRE( std::get<1>(mtt) == 5.f );

        constexpr std::tuple<size_t, size_t> mtt2 = util::map_to_tuple(type_list<int, float>{}, [](auto, size_t i) {
            return i;
        });
        REQUIRE( std::get<0>(mtt2) == 0 );
        REQUIRE( std::get<1>(mtt2) == 1 );
    }

    SECTION( "for_each" ) {
        util::for_each(type_list<int, int>{}, [&](auto) {
            // Do nothing.
        });

        std::array<int, 2> fe{};
        util::for_each(type_list<int, int>{}, [&](auto, size_t i) {
            fe[i] = i;
        });

        assert(fe[0] == 0);
        assert(fe[1] == 1);
    }

    SECTION( "accumulate" ) {
        constexpr int acc = util::accumulate(type_list<int, int>{}, std::plus<int>(), 5);
        REQUIRE( acc == 5 );
    }

    SECTION( "count_if" ) {
        constexpr int cnt = util::count_if(type_list<int, float>{}, [](auto x) {
            return std::is_integral_v<decltype(x)>;
        });
        REQUIRE( cnt == 1 );
    }

    SECTION( "filter" ) {
        constexpr type_list<int> flt = util::filter(type_list<int, float>{}, [](auto x) {
            return std::is_integral_v<decltype(x)>;
        });
        REQUIRE( flt.size == 1 );
    }

    SECTION( "contains" ) {
        constexpr bool con = util::contains(type_list<int, float>{}, [](auto x) {
            return std::is_same_v<decltype(x), float>;
        });
        REQUIRE( con );
    }

    SECTION( "apply" ) {
        constexpr int apl = util::apply(type_list<int, float>{}, [](int, float) {
            return 0;
        });
        REQUIRE( apl == 0 );
    }

    SECTION( "box/unbox" ) {
        constexpr auto boxed = reflect_types(type_list<int, float>{});
        constexpr auto boxed2 = reflect_types(unreflect_types(boxed));

        REQUIRE( std::is_same_v<decltype(boxed), decltype(boxed2)> );
    }

}