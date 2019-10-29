#include <cstdint>
#include <sstream>
#include <iostream>

#define REFL_EXPERIMENTAL_AUTO
#include "refl.hpp"

struct Table : refl::attr::usage::type
{
    const char* name;
    
    constexpr Table(const char* name) noexcept 
        : name(name)
    { 
    }
};

enum class DataType
{
    ID,
    TEXT,
};

struct Column : refl::attr::usage::field
{
    const char* name;
    const DataType data_type;
    
    constexpr Column(const char* name, DataType dataType) noexcept 
        : name(name), data_type(dataType) 
    { }
};

struct User
{
    std::uint32_t id;
    std::string email;
};

REFL_AUTO(
    type(User, Table{"Users"}),
    field(id, Column{"ID", DataType::ID}),
    field(email, Column{"Email", DataType::TEXT})
);

template <typename Member>
constexpr auto make_sql_field_spec(Member)
{
    using namespace refl;

    constexpr auto col = descriptor::get_attribute<Column>(Member{});
    
    if constexpr (DataType::ID == col.data_type) {
        return Member::name + " int PRIMARY KEY";
    } 
    else if constexpr (DataType::TEXT == col.data_type) {
        return Member::name + " TEXT";
    }
}

template <typename T>
constexpr auto make_sql_create_table()
{
    using namespace refl;
    using Td = type_descriptor<T>;

    static_assert(descriptor::has_attribute<Table>(Td{}));
    constexpr auto fields = accumulate(Td::members, 
        [](auto acc, auto member) {
            return acc + ",\n\t" + make_sql_field_spec(member);
        }, make_const_string())
        .template substr<2>();

    return "CREATE TABLE " + Td::name + " (\n" + fields + "\n);";
}

int main()
{
    constexpr auto sql = make_sql_create_table<User>();
    std::cout << sql << "\n";
    std::cout << "Number of characters: " << sizeof(sql) - 1 << "\n";
}
