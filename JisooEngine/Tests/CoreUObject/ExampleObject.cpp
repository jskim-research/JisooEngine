#include "CoreUObject/ExampleObject.h"

int UExampleObject::BeginDestroyCount = 0;
int UExampleObject::FinishDestroyCount = 0;
int UExampleObject::DestructorCount = 0;

UExampleObject::UExampleObject(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UExampleObject::~UExampleObject()
{
    ++DestructorCount;
}

void UExampleObject::BeginDestroy()
{
    ++BeginDestroyCount;
}

void UExampleObject::FinishDestroy()
{
    ++FinishDestroyCount;
}

void UExampleObject::ResetCounters()
{
    BeginDestroyCount = 0;
    FinishDestroyCount = 0;
    DestructorCount = 0;
}

int UExampleObject::GetBeginDestroyCount()
{
    return BeginDestroyCount;
}

int UExampleObject::GetFinishDestroyCount()
{
    return FinishDestroyCount;
}

int UExampleObject::GetDestructorCount()
{
    return DestructorCount;
}
