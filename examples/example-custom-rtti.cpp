/**
 * ***README***
 * This example showcases how a runtime reflection system can be implemented on
 * top of refl-cpp (here only the name of the type is being stored, but this can be extended).
 *
 * We are storing the runtime information in TypeInfo. TypeInfo also has a factory method called
 * Get<T>, which creates an instance of TypeInfo by using metadata from refl-cpp.
 */
#include <cassert>
#include "refl.hpp"

// create a class to hold runtime type information
class TypeInfo
{
public:

    // instances can be obtained only through calls to Get()
    template <typename T>
    static const TypeInfo& Get()
    {
        // here we create the singleton instance for this particular type
        static const TypeInfo ti(refl::reflect<T>());
        return ti;
    }

    const std::string& Name() const
    {
        return name_;
    }

private:

    // were only storing the name for demonstration purposes,
    // but this can be extended to hold other properties as well
    std::string name_;

    // given a type_descriptor, we construct a TypeInfo
    // with all the metadata we care about (currently only name)
    template <typename T, typename... Fields>
    TypeInfo(refl::type_descriptor<T> td)
        : name_(td.name)
    {
    }

};

// we will be using this base interface to specify that a type is
// reflectable at runtime using our custom system above
class Reflectable
{
public:
    virtual const TypeInfo& GetTypeInfo() const = 0;
};

// define a convenience macro to autoimplement GetTypeInfo()
#define MYLIB_REFLECTABLE() \
    virtual const TypeInfo& GetTypeInfo() const override \
    { \
        return TypeInfo::Get<::refl::trait::remove_qualifiers_t<decltype(*this)>>(); \
    }

class Actor : Reflectable
{
public:
    // inject GetTypeInfo impl
    MYLIB_REFLECTABLE()

    virtual ~Actor() noexcept
    {
    }
};

// create refl-cpp metadata
REFL_AUTO(type(Actor))

// inherit reflectable type
class Pawn : public Actor
{
public:
    // override GetTypeInfo with derived impl
    MYLIB_REFLECTABLE()

    virtual ~Pawn() noexcept
    {
    }
};

REFL_AUTO(type(Pawn))

class FirstPersonController : public Pawn
{
public:
    // again, override GetTypeInfo with proper impl
    MYLIB_REFLECTABLE()

    int health = 100;

    virtual ~FirstPersonController() noexcept
    {
    }
};

REFL_AUTO(
    type(FirstPersonController),
    field(health)
)

int main()
{
    FirstPersonController fpc;
    Pawn& pawn = fpc; // refer through parent type
    const TypeInfo& pawnTypeInfo = pawn.GetTypeInfo(); // get custom type info

    // access the name through our TypeInfo object
    assert(pawnTypeInfo.Name() == "FirstPersonController");
}