/**
 * ***README***
 * This example showcases how proxies can be used to augment a type to create useful
 * utility types. Here we define a "partial" proxy type, which like all proxies,
 * has member functions with equivalent names to the members of the target, but interestingly,
 * those member accesstors return std::optionals. This can be useful to provide partial
 * values of an object.
 *
 * Take a look at the update() function below to see how
 * such partial objects can be used to selectively set a whole object's members.
 */
#include "refl.hpp"
#include <iostream>
#include <optional>

/**
 * Used with trait::map_t to provide storage type for the member.
 */
template <typename Member>
struct make_optional_storage
{
    using underlying_type = decltype(Member{}(std::declval<const typename Member::declaring_type&>()));
    using type = std::optional<refl::trait::remove_qualifiers_t<underlying_type>>;
};

/**
 * A proxy which stores properties of the target type as std::optionals.
 */
template <typename T>
class partial : public refl::runtime::proxy<partial<T>, T>
{
public:

    // Fields and property getters.
    static constexpr auto members = filter(refl::member_list<T>{}, [](auto member) { return is_readable(member) && has_writer(member); });

    using member_list = std::remove_cv_t<decltype(members)>;

    // Trap getter calls.
    template <typename Member, typename Self, typename... Args>
    static decltype(auto) invoke_impl(Self&& self)
    {
        static_assert(is_readable(Member{}));
        return self.template get<Member>();
    }

    // Trap setter calls.
    template <typename Member, typename Self, typename Value>
    static void invoke_impl(Self&& self, Value&& value)
    {
        static_assert(is_writable(Member{}));
        using getter_t = decltype(get_reader(Member{}));
        self.template get<getter_t>() = std::forward<Value>(value);
    }

    template <typename Member>
    auto& get()
    {
        constexpr size_t idx = refl::trait::index_of_v<Member, member_list>;
        static_assert(idx != -1);
        return refl::util::get<idx>(data);
    }

    template <typename Member>
    const auto& get() const
    {
        constexpr size_t idx = refl::trait::index_of_v<Member, member_list>;
        static_assert(idx != -1);
        return refl::util::get<idx>(data);
    }

private:

    using member_storage_list = refl::trait::map_t<make_optional_storage, member_list>;

    refl::trait::as_tuple_t<member_storage_list> data;

};

template <typename T>
void update(T& target, const partial<T>& source)
{
    for_each(source.members, [&](auto member) {
        using member_type = decltype(member);
        constexpr auto writer = get_writer(member);
        auto opt_value = source.template get<member_type>();
        if (opt_value) {
            writer(target, *opt_value);
        }
    });
}

struct Book
{
public:

    Book(const std::string& title, const std::string& author, int pages)
        : title_(title), author_(author), pages_(pages)
    {
    }

    const std::string& title() const { return title_; }
    void set_title(const std::string& value) { title_ = value; }

    const std::string& author() const { return author_; }
    void set_author(const std::string& value) { author_ = value; }

    int pages() const { return pages_; }
    void set_pages(int value) { pages_ = value; }

private:
    std::string title_;
    std::string author_;
    int pages_;
};

REFL_AUTO(
    type(Book),
    func(title, property()), func(set_title, property()),
    func(author, property()), func(set_author, property()),
    func(pages, property()), func(set_pages, property())
)

int main()
{
    Book book("Title", "Author", 0);

    partial<Book> partial_book;

    std::cout << "title of partial before set_title = " << partial_book.title().value_or("<nullopt>") << '\n';
    partial_book.set_title("New Title");
    std::cout << "title of partial after set_title = " << partial_book.title().value_or("<nullopt>") << '\n';

    std::cout << "title of book before update = " << book.title() << '\n';
    update(book, partial_book);
    std::cout << "title of book after update = " << book.title() << '\n';

    std::cout << std::endl;
}