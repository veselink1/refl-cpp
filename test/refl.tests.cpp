#include "../refl.hpp"
#include <cassert>
#include <vector>

struct A {
    int x;
    float* y;
    const char z;

    void f() {}
    void f(int) {}
    int g(int) { return 0; }
};

REFL_TYPE(A)
    REFL_FIELD(x)
    REFL_FIELD(y, property(read_only, "Y"))
    REFL_FIELD(z)
    
    REFL_FUNC(f)
    REFL_FUNC(g)
REFL_END

template <typename T>
struct Box {
    using type = std::true_type;
    T value;
};

REFL_AUTO(
    template((typename T), (Box<T>)),
    field(value)
)

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

void tests()
{
    using namespace refl;

    /* const_string */
    static_assert(const_string<0>() == "");
    static_assert(const_string<1>("A") == "A");
    static_assert("AB" == const_string<2>("AB"));
    static_assert(const_string<2>("AB") == const_string<1>("A") + const_string<1>("B"));

    /* type_list */
    static_assert(type_list<>::size == 0);
    static_assert(type_list<int>::size == 1);

    /* trait::* */
    static_assert(std::is_same_v<trait::remove_qualifiers_t<const int&>, int>);
    static_assert(trait::is_reflectable_v<int>);
    
    struct custom {};
    static_assert(!trait::is_reflectable_v<custom>);

    static_assert(trait::is_container_v<std::vector<int>>);
    static_assert(!trait::is_container_v<custom>);

    static_assert(std::is_same_v<trait::get_t<0, type_list<int, float>>, int>);
    static_assert(std::is_same_v<trait::get_t<1, type_list<int, float>>, float>);
    
    static_assert(std::is_same_v<trait::skip_t<1, type_list<int, float>>, type_list<float>>);
    static_assert(std::is_same_v<trait::skip_t<2, type_list<int, float>>, type_list<>>);
    
    static_assert(std::is_same_v<trait::filter_t<std::is_integral, type_list<int, float>>, type_list<int>>);
    static_assert(std::is_same_v<trait::map_t<std::remove_reference, type_list<int&, float&>>, type_list<int, float>>);

    /* attr::usage::* */
    static_assert(std::is_base_of_v<attr::usage::field, attr::usage::member>);
    static_assert(std::is_base_of_v<attr::usage::function, attr::usage::member>);
    static_assert(std::is_base_of_v<attr::usage::member, attr::usage::any>);
    static_assert(std::is_base_of_v<attr::usage::type, attr::usage::any>);

    static_assert(!trait::is_member_v<int>);
    static_assert(trait::is_member_v<trait::get_t<0, member_list<A>>>);
    
    static_assert(!trait::is_field_v<int>);
    static_assert(trait::is_field_v<trait::get_t<0, member_list<A>>>);
    
    static_assert(!trait::is_function_v<int>);
    static_assert(trait::is_function_v<trait::get_t<3, member_list<A>>>);

    /* util::* */
    {
        static_assert(std::is_same_v<decltype(util::to_array<int>(std::make_tuple(0, 0))), std::array<int, 2>>);
        static_assert(std::is_same_v<decltype(util::to_array<int>(std::tuple<>{})), std::array<int, 0>>);

        static_assert(std::is_same_v<decltype(util::to_tuple(std::array<int, 2>{})), std::tuple<int, int>>);
        static_assert(std::is_same_v<decltype(util::to_tuple(std::array<int, 0>{})), std::tuple<>>);
        
        constexpr std::tuple<int, float> mtt = util::map_to_tuple(type_list<int, float>{}, [](auto x) { 
            return decltype(x)(5);
        });
        static_assert(std::get<0>(mtt) == 5);
        static_assert(std::get<1>(mtt) == 5.f);
        
        constexpr std::tuple<size_t, size_t> mtt2 = util::map_to_tuple(type_list<int, float>{}, [](auto, size_t i) { 
            return i;
        });
        static_assert(std::get<0>(mtt2) == 0);
        static_assert(std::get<1>(mtt2) == 1);

        constexpr std::array<int, 2> mta = util::map_to_array<int>(type_list<int, float>{}, [](auto) { 
            return 5; 
        });
        static_assert(mta[0] == 5);
        static_assert(mta[1] == 5);
        
        constexpr std::array<int, 2> mta2 = util::map_to_array<int>(type_list<int, float>{}, [](auto, size_t i) { 
            return static_cast<int>(i); 
        });
        static_assert(mta2[0] == 0);
        static_assert(mta2[1] == 1);

        util::for_each(type_list<int, int>{}, [&](auto) {
            // Do nothing.
        });

        std::array<int, 2> fe{};
        util::for_each(type_list<int, int>{}, [&](auto x, size_t i) {
            fe[i] = i;
        });

        assert(fe[0] == 0);
        assert(fe[1] == 1);

        constexpr int acc = util::accumulate(type_list<int, int>{}, [](auto& sum, auto x) {
            sum += x;
        }, 5);
        static_assert(acc == 5);
        
        constexpr int acc2 = util::accumulate(type_list<int, int>{}, [](auto& sum, auto, size_t i) {
            sum += i;
        }, 5);
        static_assert(acc2 == 6);
        
        constexpr int cnt = util::count_if(type_list<int, float>{}, [](auto x) {
            return std::is_integral_v<decltype(x)>;
        });
        static_assert(cnt == 1);
        
        constexpr type_list<int> flt = util::filter(type_list<int, float>{}, [](auto x) {
            return std::is_integral_v<decltype(x)>;
        });
        static_assert(flt.size == 1);
        
        constexpr bool con = util::contains(type_list<int, float>{}, [](auto x) {
            return std::is_same_v<decltype(x), float>;
        });
        static_assert(con);
        
        constexpr int apl = util::apply(type_list<int, float>{}, [](int, float) {
            return 0;
        });
        static_assert(apl == 0);

        static_assert(std::is_same_v<decltype(type_descriptor<int>::attributes), const std::tuple<>>);
        static_assert(std::is_same_v<decltype(type_descriptor<int*>::attributes), const std::tuple<>>);
        static_assert(std::is_same_v<decltype(type_descriptor<int* const>::attributes), const std::tuple<>>);
        
        static_assert(std::is_same_v<decltype(type_descriptor<int>::members), const type_list<>>);
        static_assert(std::is_same_v<decltype(type_descriptor<int*>::members), const type_list<>>);
        static_assert(std::is_same_v<decltype(type_descriptor<int* const>::members), const type_list<>>);

        static_assert(field_descriptor<A, 0>::name == "x");
        static_assert(field_descriptor<A, 0>::pointer == &A::x);
        static_assert(!field_descriptor<A, 0>::is_static);
        
        static_assert(function_descriptor<A, 3>::name == "f");
        static_assert(std::is_invocable_v<function_descriptor<A, 3>, A>);
        static_assert(std::is_invocable_v<function_descriptor<A, 3>, A, int>);
        static_assert(!std::is_invocable_v<function_descriptor<A, 3>, A, std::string>);
        
        static_assert(is_reflectable<int>());
        struct dummy {};
        static_assert(!is_reflectable<dummy>());

        static_assert(trait::is_instance_v<std::tuple<>>);
        static_assert(!trait::is_instance_v<int>);
        
        static_assert(trait::is_instance_of_v<std::tuple, std::tuple<>>);
        static_assert(!trait::is_instance_of_v<std::tuple, int>);
        
        static_assert(trait::contains_v<int, type_list<int, float>>);
        static_assert(!trait::contains_v<int, type_list<float, double>>);
        
        static_assert(trait::contains_instance_v<std::tuple, type_list<int, std::tuple<float>>>);
        static_assert(!trait::contains_instance_v<std::tuple, type_list<int, float>>);
        static_assert(!trait::contains_instance_v<attr::debug, trait::get_t<0, member_list<A>>::attribute_types>);
        
        struct B : A {};

        static_assert(trait::contains_base_v<A, type_list<int, A>>);
        static_assert(trait::contains_base_v<A, type_list<int, B>>);
        static_assert(!trait::contains_base_v<A, type_list<int, float>>);
    }

    /* descriptor::* */
    {    
        constexpr auto y_member = trait::get_t<1, member_list<A>>{};
        static_assert(y_member.name == "y");

        static_assert(is_field(y_member));
        static_assert(!is_function(y_member));
        static_assert(is_property(y_member));
        assert(strcmp(get_debug_name(y_member), "A::y") == 0);
        assert(strcmp(get_display_name(y_member), "Y") == 0);
        static_assert(y_member(A{}) == 0);
        static_assert(std::is_same_v<trait::get_t<3, member_list<A>>::return_type<A, int>, void>);
        static_assert(std::is_invocable_v<trait::get_t<0, member_list<A>>, A, int>);

        using f_td = trait::get_t<3, member_list<A>>;
        static_assert(!f_td::is_resolved);
        auto f_ptr = f_td::resolve<void(A::*)()>;

        using g_td = trait::get_t<4, member_list<A>>;
        static_assert(g_td::is_resolved);
        auto g_ptr = g_td::pointer;
    }

    /* runtime::* */
    {
        dummy_proxy<A> pa;
        assert(pa.y() == 0);
        assert(pa.f(1) == 0);
        assert(util::make_const(pa).f(1) == 0);

        static_assert(trait::is_proxy_v<dummy_proxy<A>>);
        static_assert(!trait::is_proxy_v<int>);

        assert(runtime::debug_str(A{}).find("x = ") != std::string::npos);
        assert(runtime::debug_str(A{}).find("Y = ") != std::string::npos);

        assert(runtime::invoke<int>(A{}, "x", 1) == 1);
    }
}

int main()
{
    tests();
}