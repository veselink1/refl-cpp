#include <iostream>
#include "refl.hpp"

class Animal
{
};

REFL_TYPE(Animal, bases<>)
    REFL_FUNC(species)
REFL_END

class Dog : public Animal
{
};

REFL_TYPE(Dog, bases<Animal>)
    REFL_FUNC(species)
REFL_END

template <typename T>
void print_bases()
{
    using refl::reflect;                    // shorthand for refl::descriptor::type_descriptor<T>
    using refl::descriptor::get_attribute;  // get_attribute<A, T>, get_attribute<A<Ts...>, T>
                                            // for variadic and non-variadic attributes

    constexpr auto type = reflect<T>();
    std::cout << type.name << " inherits from ";

    constexpr auto bases = get_bases(type);
    if constexpr (bases.size)
    {
        for_each(bases, [](auto t)
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
    using refl::reflect;                    // shorthand for refl::descriptor::type_descriptor<T>
    using refl::descriptor::get_attribute;  // get_attribute<A, T>, get_attribute<A<Ts...>, T>
                                            // for variadic and non-variadic attributes

    print_bases<Animal>();
    print_bases<Dog>();
}