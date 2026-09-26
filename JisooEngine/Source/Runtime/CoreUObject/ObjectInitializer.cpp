#include "Runtime/CoreUObject/ObjectInitializer.h"

#include <utility>

FObjectInitializer::FObjectInitializer(
    UClass* InClass,
    UObject* InOuter,
    std::string InName,
    FObjectHandle InHandle)
    : Class(InClass)
    , Outer(InOuter)
    , Name(std::move(InName))
    , Handle(InHandle)
{
}

UClass* FObjectInitializer::GetClass() const
{
    return Class;
}

UObject* FObjectInitializer::GetOuter() const
{
    return Outer;
}

const std::string& FObjectInitializer::GetName() const
{
    return Name;
}

FObjectHandle FObjectInitializer::GetHandle() const
{
    return Handle;
}
