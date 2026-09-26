#include "Runtime/CoreUObject/Class.h"

#include <cassert>

UClass::UClass(
    std::string_view InName,
    UClass* InSuperClass,
    std::size_t InClassSize,
    std::size_t InClassAlignment,
    FConstructObject InConstructor,
    FDestroyObject InDestructor)
    : Name(InName)
    , SuperClass(InSuperClass)
    , ClassSize(InClassSize)
    , ClassAlignment(InClassAlignment)
    , Constructor(InConstructor)
    , Destructor(InDestructor)
{
    assert(!Name.empty());
    assert(ClassSize > 0);
    assert(ClassAlignment > 0);
    assert(Constructor != nullptr);
    assert(Destructor != nullptr);
}

std::string_view UClass::GetName() const
{
    return Name;
}

UClass* UClass::GetSuperClass() const
{
    return SuperClass;
}

std::size_t UClass::GetClassSize() const
{
    return ClassSize;
}

std::size_t UClass::GetClassAlignment() const
{
    return ClassAlignment;
}

bool UClass::IsChildOf(const UClass* TargetClass) const
{
    if (TargetClass == nullptr)
    {
        return false;
    }

    for (const UClass* CurrentClass = this;
         CurrentClass != nullptr;
         CurrentClass = CurrentClass->GetSuperClass())
    {
        if (CurrentClass == TargetClass)
        {
            return true;
        }
    }

    return false;
}

void UClass::Construct(void* Memory, const FObjectInitializer& ObjectInitializer) const
{
    assert(Memory != nullptr);
    Constructor(Memory, ObjectInitializer);
}

void UClass::Destroy(UObject* Object) const
{
    assert(Object != nullptr);
    Destructor(Object);
}
