#include "Runtime/CoreUObject/Object.h"

#include "Runtime/CoreUObject/Class.h"
#include "Runtime/CoreUObject/ObjectInitializer.h"

UObject::UObject(const FObjectInitializer& ObjectInitializer)
{
    ClassPrivate = ObjectInitializer.GetClass();
    OuterPrivate = ObjectInitializer.GetOuter();
    NamePrivate = ObjectInitializer.GetName();
    HandlePrivate = ObjectInitializer.GetHandle();
}

UObject::~UObject() = default;

UClass* UObject::GetClass() const
{
    return ClassPrivate;
}

UObject* UObject::GetOuter() const
{
    return OuterPrivate;
}

const std::string& UObject::GetName() const
{
    return NamePrivate;
}

std::string UObject::GetPathName() const
{
    if (OuterPrivate == nullptr)
    {
        return NamePrivate;
    }

    return OuterPrivate->GetPathName() + "." + NamePrivate;
}

FObjectHandle UObject::GetHandle() const
{
    return HandlePrivate;
}

EObjectState UObject::GetState() const
{
    return State;
}

bool UObject::IsA(const UClass* TargetClass) const
{
    return ClassPrivate != nullptr && ClassPrivate->IsChildOf(TargetClass);
}

void UObject::BeginDestroy()
{
}

bool UObject::IsReadyForFinishDestroy() const
{
    return true;
}

void UObject::FinishDestroy()
{
}

void UObject::SetState(EObjectState InState)
{
    State = InState;
}
