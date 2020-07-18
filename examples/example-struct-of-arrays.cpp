#include "refl.hpp"
#include <iostream>
#include <vector>

/********************************/
template <typename T>
constexpr auto readable_members = filter(refl::member_list<T>{}, [](auto member) { return is_readable(member); });

template <typename T>
using readable_member_list = std::remove_const_t<decltype(readable_members<T>)>;

/**
 * The type the readable member returns when invoked.
 */
template <typename ReadableMember>
using underlying_type = refl::trait::remove_qualifiers_t<decltype(ReadableMember()(std::declval<const typename ReadableMember::declaring_type&>()))>;

template <typename T>
struct struct_of_arrays : refl::runtime::proxy<struct_of_arrays<T>, std::remove_cv_t<T>> {
public:

    using readable_members = readable_member_list<std::remove_cv_t<T>>;
    static_assert(readable_members::size > 0, "Type has no fields!");

    struct_of_arrays(size_t capacity = 4)
    {
    }

    void push_back(const T& value)
    {
        for_each(readable_members {}, [&](auto member, size_t index) {
            constexpr auto i = refl::trait::index_of_v<decltype(member), readable_members>;
            std::get<i>(storage_).push_back(member(value));
        });
    }

    void pop_back()
    {
        for_each(readable_members{}, [&](auto member) {
            constexpr auto i = refl::trait::index_of_v<decltype(member), readable_members>;
            std::get<i>(storage_).pop_back();
        });
    }

    T at(size_t index) const
    {
        T t{};
        for_each(readable_members{}, [&](auto member) {
            constexpr auto i = refl::trait::index_of_v<decltype(member), readable_members>;
            std::get<i>(storage_).pop_back();
        });
    }

    template <typename Member>
    auto& operator[](Member)
    {
        constexpr auto i = refl::trait::index_of_v<Member, readable_members>;
        return std::get<i>(storage_);
    }

    template <typename Member>
    const auto& operator[](Member) const
    {
        constexpr auto i = refl::trait::index_of_v<Member, readable_members>;
        return std::get<i>(storage_);
    }

    size_t size() const
    {
        return std::get<0>(storage_).size();
    }

    template <typename Member, typename Self>
    static auto invoke_impl(Self&& self, size_t index) -> decltype(auto)
    {
        constexpr auto i = refl::trait::index_of_v<Member, readable_members>;
        auto&& vec = std::get<i>(self.storage_);
        return vec.at(index);
    }

private:

    template <typename ReadableMember>
    struct make_storage
    {
        using type = std::vector<underlying_type<ReadableMember>>;
    };

    refl::trait::as_tuple_t<refl::trait::map_t<make_storage, readable_members>> storage_;
};

template <size_t N, typename T>
auto& get(struct_of_arrays<T>& soa)
{
    using member = refl::trait::get_t<N, typename struct_of_arrays<T>::readable_members>;
    return soa[member{}];
}

template <size_t N, typename T>
const auto& get(const struct_of_arrays<T>& soa)
{
    using member = refl::trait::get_t<N, typename struct_of_arrays<T>::readable_members>;
    return soa[member{}];
}

namespace std
{
    template <typename T>
    struct tuple_size<struct_of_arrays<T>>
    {
        static constexpr size_t value = struct_of_arrays<T>::readable_members::size;
    };

    template <size_t N, typename T>
    struct tuple_element<N, struct_of_arrays<T>>
    {
        using descriptor = refl::trait::get_t<N, typename struct_of_arrays<T>::readable_members>;
        using type = decltype(std::declval<struct_of_arrays<T>>()[descriptor{}]);
    };
}

/********************************/

class color
{
public:

    color() = default;

    color(float r, float g, float b)
        : r(r), g(g), b(b)
    {
    }

    float red() const { return r; }
    void set_red(float value) { r = value; }

    float green() const { return g; }
    void set_green(float value) { g = value; }

    float blue() const { return b; }
    void set_blue(float value) { b = value; }

private:
    float r, g, b;
};

REFL_AUTO(
    type(color),
    func(red, property()), func(set_red, property()),
    func(green, property()), func(set_green, property()),
    func(blue, property()), func(set_blue, property())
)
/********************************/

int main()
{
    struct_of_arrays<color> colors;
    colors.push_back(color{ .0f, .5f, .5f });
    colors.push_back(color{ .0f, .5f, .0f });
    colors.push_back(color{ .5f, .5f, .5f });

    auto& [red_chan, green_chan, blue_chan] = colors;

    std::cout << "size=" << colors.size() << "\n";
    for (size_t i = 0; i < colors.size(); i++) {
        std::cout << "r=" << red_chan[i] << ",g=" << green_chan[i] << ",b=" << blue_chan[i] << "\n";
    }
}
