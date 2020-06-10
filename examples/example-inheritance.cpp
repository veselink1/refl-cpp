#include <iostream>
#include "refl.hpp"

class Animal
{
public:
    virtual std::string get_species() const = 0;
};

REFL_AUTO(
    type(Animal),
    func(get_species, property())
)

class Wolf : public Animal
{
public:
    std::string get_species() const override
    {
        return "Canis lupus";
    }
};

REFL_AUTO(type(Wolf, bases<Animal>))

class Dog : public Wolf
{
public:
    std::string get_species() const override
    {
        return "Canis lupus familiaris";
    }
};

REFL_AUTO(type(Dog, bases<Wolf>))

template <typename T>
void print_bases()
{
    constexpr auto type = refl::reflect<T>();
    std::cout << type.name << " inherits from ";

    if constexpr (type.declared_bases.size)
    {
        for_each(reflect_types(type.declared_bases), [](auto t)
        {
            std::cout << t.name << " ";
        });
        std::cout << '\n';
    }
    else
    {
        std::cout << "nothing\n";
    }
}

int main()
{
    print_bases<Animal>();
    print_bases<Wolf>();
    print_bases<Dog>();

    Dog d{};
    std::cout << "type Dog:\n";
    for_each(refl::reflect<Dog>().members, [&](auto member) {
        std::cout << "  " << get_display_name(member) << " (" << member.name << ") = ";
        refl::runtime::debug(std::cout, member(d));
        std::cout << "\n";
    });
}