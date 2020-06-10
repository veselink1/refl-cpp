#include "refl.hpp"
#include <iostream>
#include <optional>

/**
 * A pair of types.
 */
template <typename T1, typename T2>
struct type_pair
{
    using first_type = T1;
    using second_type = T2;
};

template <typename Member>
struct optional_member
{
    using member_type = Member;
    std::optional<typename Member::value_type> value;
};

template <typename T>
struct make_optional_member
{
    using type = optional_member<T>;
};

template <typename Member>
struct optional_member_predicate
{
    template <typename OptionalMember>
    struct match : std::is_same<Member, typename OptionalMember::member_type> {};
};

template <typename T>
struct as_tuple;

template <template <typename...> typename T, typename... Ts>
struct as_tuple<T<Ts...>>
{
    using type = std::tuple<Ts...>;
};

template <typename T>
using as_tuple_t = typename as_tuple<T>::type;

template <typename T>
class partial : public refl::runtime::proxy<partial<T>, T>
{
public:

    template <typename Member, typename Self, typename... Args>
    static auto invoke_impl(Self&& self, Args&&... args) -> decltype(auto)
    {
        static_assert(sizeof...(Args) < 2);
        if constexpr (sizeof...(Args) == 0) {
            return self.template get<Member>();
        }
        else if constexpr (sizeof...(Args) == 1) {
            self.template get<Member>() = refl::util::identity(std::forward<Args>(args)...);
            return self;
        }
    }

    template <typename Member>
    auto& get()
    {
        return std::get<find_optional_by_member<Member>>(opt_member_tuple).value;
    }

    template <typename Member>
    const auto& get() const
    {
        return std::get<find_optional_by_member<Member>>(opt_member_tuple).value;
    }

private:

    using opt_member_list = refl::trait::map_t<make_optional_member, refl::member_list<T>>;
    as_tuple_t<opt_member_list> opt_member_tuple;

    template <typename Member>
    using find_optional_by_member = refl::trait::first_t<
        refl::trait::filter_t<
            optional_member_predicate<Member>::template match, opt_member_list>>;

};

struct Point
{
    float x, y;
};

REFL_AUTO(type(Point), field(x), field(y))

template <typename T>
void update(T& target, const partial<T>& source)
{
    for_each(refl::member_list<T>{}, [&](auto member) {
        using member_type = decltype(member);
        auto opt_value = source.template get<member_type>();
        if (opt_value) {
            member(target, *opt_value);
        }
    });
}

int main()
{
    Point pt{1, 2};

    partial<Point> partial_pt;

    std::cout << "x = " << partial_pt.x().value_or(-1) << '\n';
    partial_pt.x(20);
    std::cout << "x = " << partial_pt.x().value_or(-1) << '\n';

    update(pt, partial_pt);
    std::cout << "x = " << pt.x << '\n';

    std::cout << std::endl;
}