#include <cassert>
#include "refl.hpp"

class TypeInfo
{
public:

    template <typename T>
    static const TypeInfo& Get()
    {
        static const TypeInfo ti(refl::reflect<T>());  
        return ti; 
    }

    const std::string& Name() const
    {
        return name_;
    }

private:

    std::string name_;

    template <typename T>
    TypeInfo(const refl::type_descriptor<T>& td)
        : name_(td.name)
    {
    }

};

class Reflectable
{
public:
    virtual const TypeInfo& GetTypeInfo() const = 0;
};

#define MYLIB_REFLECTABLE() \
    virtual const TypeInfo& GetTypeInfo() const override \
    { \
        return TypeInfo::Get<::refl::trait::remove_qualifiers_t<decltype(*this)>>(); \
    }

class Actor : Reflectable
{
public:
    MYLIB_REFLECTABLE()

    virtual ~Actor() noexcept 
    {
    }
};

REFL_TYPE(Actor)
REFL_END

class Pawn : public Actor
{
public:
    MYLIB_REFLECTABLE()

    virtual ~Pawn() noexcept 
    {
    }
};

REFL_TYPE(Pawn)
REFL_END

class FirstPersonController : public Pawn
{
public:
    MYLIB_REFLECTABLE()

    virtual ~FirstPersonController() noexcept 
    {
    }
};

REFL_TYPE(FirstPersonController)
REFL_END

int main()
{
    FirstPersonController fpc;
    Pawn& pawn = fpc;
    const TypeInfo& pawnTypeInfo = pawn.GetTypeInfo();

    assert(pawnTypeInfo.Name() == "FirstPersonController");
}