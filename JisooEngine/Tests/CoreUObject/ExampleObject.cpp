#include "CoreUObject/ExampleObject.h"

#include "Runtime/Engine/World.h"

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

int UTestActor::BeginPlayCount = 0;
int UTestActor::TickCount = 0;
int UTestActor::EndPlayCount = 0;
bool UTestActor::bSpawnOnNextTick = false;
bool UTestActor::bDestroySelfOnNextTick = false;

UTestActor::UTestActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UTestActor::BeginPlay()
{
    ++BeginPlayCount;
}

void UTestActor::Tick(float)
{
    ++TickCount;

    if (bSpawnOnNextTick)
    {
        bSpawnOnNextTick = false;
        [[maybe_unused]] UTestActor* SpawnedActor =
            GetWorld()->SpawnActor<UTestActor>("SpawnedDuringTick");
    }

    if (bDestroySelfOnNextTick)
    {
        bDestroySelfOnNextTick = false;
        GetWorld()->DestroyActor(this);
    }
}

void UTestActor::EndPlay()
{
    ++EndPlayCount;
}

void UTestActor::ResetCounters()
{
    BeginPlayCount = 0;
    TickCount = 0;
    EndPlayCount = 0;
    bSpawnOnNextTick = false;
    bDestroySelfOnNextTick = false;
}

void UTestActor::RequestSpawnOnNextTick()
{
    bSpawnOnNextTick = true;
}

void UTestActor::RequestDestroySelfOnNextTick()
{
    bDestroySelfOnNextTick = true;
}

int UTestActor::GetBeginPlayCount()
{
    return BeginPlayCount;
}

int UTestActor::GetTickCount()
{
    return TickCount;
}

int UTestActor::GetEndPlayCount()
{
    return EndPlayCount;
}

int UTestActorComponent::BeginPlayCount = 0;
int UTestActorComponent::TickCount = 0;
int UTestActorComponent::EndPlayCount = 0;

UTestActorComponent::UTestActorComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UTestActorComponent::BeginPlay()
{
    ++BeginPlayCount;
}

void UTestActorComponent::TickComponent(float)
{
    ++TickCount;
}

void UTestActorComponent::EndPlay()
{
    ++EndPlayCount;
}

void UTestActorComponent::ResetCounters()
{
    BeginPlayCount = 0;
    TickCount = 0;
    EndPlayCount = 0;
}

int UTestActorComponent::GetBeginPlayCount()
{
    return BeginPlayCount;
}

int UTestActorComponent::GetTickCount()
{
    return TickCount;
}

int UTestActorComponent::GetEndPlayCount()
{
    return EndPlayCount;
}
