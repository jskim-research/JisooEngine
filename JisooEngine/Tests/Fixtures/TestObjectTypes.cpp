#include "Fixtures/TestObjectTypes.h"

#include "Runtime/Engine/World.h"
#include "Runtime/Render/Scene/Scene.h"

int ULifecycleTestObject::BeginDestroyCount = 0;
int ULifecycleTestObject::FinishDestroyCount = 0;
int ULifecycleTestObject::DestructorCount = 0;

ULifecycleTestObject::ULifecycleTestObject(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

ULifecycleTestObject::~ULifecycleTestObject()
{
    ++DestructorCount;
}

void ULifecycleTestObject::BeginDestroy()
{
    ++BeginDestroyCount;
}

void ULifecycleTestObject::FinishDestroy()
{
    ++FinishDestroyCount;
}

void ULifecycleTestObject::ResetCounters()
{
    BeginDestroyCount = 0;
    FinishDestroyCount = 0;
    DestructorCount = 0;
}

int ULifecycleTestObject::GetBeginDestroyCount()
{
    return BeginDestroyCount;
}

int ULifecycleTestObject::GetFinishDestroyCount()
{
    return FinishDestroyCount;
}

int ULifecycleTestObject::GetDestructorCount()
{
    return DestructorCount;
}

int ULifecycleTestActor::BeginPlayCount = 0;
int ULifecycleTestActor::TickCount = 0;
int ULifecycleTestActor::EndPlayCount = 0;
bool ULifecycleTestActor::bSpawnOnNextTick = false;
bool ULifecycleTestActor::bDestroySelfOnNextTick = false;

ULifecycleTestActor::ULifecycleTestActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void ULifecycleTestActor::BeginPlay()
{
    ++BeginPlayCount;
}

void ULifecycleTestActor::Tick(float)
{
    ++TickCount;

    if (bSpawnOnNextTick)
    {
        bSpawnOnNextTick = false;
        [[maybe_unused]] ULifecycleTestActor* SpawnedActor =
            GetWorld()->SpawnActor<ULifecycleTestActor>("SpawnedDuringTick");
    }

    if (bDestroySelfOnNextTick)
    {
        bDestroySelfOnNextTick = false;
        GetWorld()->DestroyActor(this);
    }
}

void ULifecycleTestActor::EndPlay()
{
    ++EndPlayCount;
}

void ULifecycleTestActor::ResetCounters()
{
    BeginPlayCount = 0;
    TickCount = 0;
    EndPlayCount = 0;
    bSpawnOnNextTick = false;
    bDestroySelfOnNextTick = false;
}

void ULifecycleTestActor::RequestSpawnOnNextTick()
{
    bSpawnOnNextTick = true;
}

void ULifecycleTestActor::RequestDestroySelfOnNextTick()
{
    bDestroySelfOnNextTick = true;
}

int ULifecycleTestActor::GetBeginPlayCount()
{
    return BeginPlayCount;
}

int ULifecycleTestActor::GetTickCount()
{
    return TickCount;
}

int ULifecycleTestActor::GetEndPlayCount()
{
    return EndPlayCount;
}

int ULifecycleTestComponent::BeginPlayCount = 0;
int ULifecycleTestComponent::TickCount = 0;
int ULifecycleTestComponent::EndPlayCount = 0;

ULifecycleTestComponent::ULifecycleTestComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void ULifecycleTestComponent::BeginPlay()
{
    ++BeginPlayCount;
}

void ULifecycleTestComponent::TickComponent(float)
{
    ++TickCount;
}

void ULifecycleTestComponent::EndPlay()
{
    ++EndPlayCount;
}

void ULifecycleTestComponent::ResetCounters()
{
    BeginPlayCount = 0;
    TickCount = 0;
    EndPlayCount = 0;
}

int ULifecycleTestComponent::GetBeginPlayCount()
{
    return BeginPlayCount;
}

int ULifecycleTestComponent::GetTickCount()
{
    return TickCount;
}

int ULifecycleTestComponent::GetEndPlayCount()
{
    return EndPlayCount;
}

UWorld* UWorldTrackingComponent::ExpectedWorld = nullptr;
bool UWorldTrackingComponent::bObservedExpectedWorld = false;
int UWorldTrackingComponent::UnregisterCount = 0;

UWorldTrackingComponent::UWorldTrackingComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UWorldTrackingComponent::ResetUnregisterObservation(UWorld* InExpectedWorld)
{
    ExpectedWorld = InExpectedWorld;
    bObservedExpectedWorld = false;
    UnregisterCount = 0;
}

bool UWorldTrackingComponent::ObservedExpectedWorld()
{
    return bObservedExpectedWorld;
}

int UWorldTrackingComponent::GetUnregisterCount()
{
    return UnregisterCount;
}

void UWorldTrackingComponent::OnUnregister()
{
    ++UnregisterCount;
    bObservedExpectedWorld = GetWorld() == ExpectedWorld;
    Super::OnUnregister();
}

UWorld* UProxyTrackingPrimitiveComponent::ExpectedWorld = nullptr;
bool UProxyTrackingPrimitiveComponent::bObservedExpectedWorld = false;
std::size_t UProxyTrackingPrimitiveComponent::PrimitiveCountBeforeUnregister = 0;
std::size_t UProxyTrackingPrimitiveComponent::PrimitiveCountAfterUnregister = 0;

UProxyTrackingPrimitiveComponent::UProxyTrackingPrimitiveComponent(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UProxyTrackingPrimitiveComponent::ResetUnregisterObservation(UWorld* InExpectedWorld)
{
    ExpectedWorld = InExpectedWorld;
    bObservedExpectedWorld = false;
    PrimitiveCountBeforeUnregister = 0;
    PrimitiveCountAfterUnregister = 0;
}

bool UProxyTrackingPrimitiveComponent::ObservedExpectedWorld()
{
    return bObservedExpectedWorld;
}

std::size_t UProxyTrackingPrimitiveComponent::GetPrimitiveCountBeforeUnregister()
{
    return PrimitiveCountBeforeUnregister;
}

std::size_t UProxyTrackingPrimitiveComponent::GetPrimitiveCountAfterUnregister()
{
    return PrimitiveCountAfterUnregister;
}

void UProxyTrackingPrimitiveComponent::OnUnregister()
{
    UWorld* World = GetWorld();
    bObservedExpectedWorld = World == ExpectedWorld;
    PrimitiveCountBeforeUnregister = World != nullptr
        ? World->GetScene()->GetPrimitiveCount()
        : 0;
    Super::OnUnregister();
    PrimitiveCountAfterUnregister = World != nullptr
        ? World->GetScene()->GetPrimitiveCount()
        : 0;
}
