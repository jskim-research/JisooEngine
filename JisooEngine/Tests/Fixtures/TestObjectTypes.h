#pragma once

#include "Runtime/CoreUObject/Object.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/Components/ActorComponent.h"
#include "Runtime/Engine/Components/PrimitiveComponent.h"

#include <cstddef>

#include "Fixtures/TestObjectTypes.generated.h"

class UWorld;

UCLASS()
class ULifecycleTestObject final : public UObject
{
    GENERATED_BODY()

public:
    ~ULifecycleTestObject() override;

    void BeginDestroy() override;
    void FinishDestroy() override;

    static void ResetCounters();
    [[nodiscard]] static int GetBeginDestroyCount();
    [[nodiscard]] static int GetFinishDestroyCount();
    [[nodiscard]] static int GetDestructorCount();

protected:
    explicit ULifecycleTestObject(const FObjectInitializer& ObjectInitializer);

private:
    static int BeginDestroyCount;
    static int FinishDestroyCount;
    static int DestructorCount;
};

UCLASS()
class ULifecycleTestActor final : public AActor
{
    GENERATED_BODY()

public:
    void BeginPlay() override;
    void Tick(float DeltaSeconds) override;
    void EndPlay() override;

    static void ResetCounters();
    static void RequestSpawnOnNextTick();
    static void RequestDestroySelfOnNextTick();
    [[nodiscard]] static int GetBeginPlayCount();
    [[nodiscard]] static int GetTickCount();
    [[nodiscard]] static int GetEndPlayCount();

protected:
    explicit ULifecycleTestActor(const FObjectInitializer& ObjectInitializer);

private:
    static int BeginPlayCount;
    static int TickCount;
    static int EndPlayCount;
    static bool bSpawnOnNextTick;
    static bool bDestroySelfOnNextTick;
};

UCLASS()
class ULifecycleTestComponent final : public UActorComponent
{
    GENERATED_BODY()

public:
    void BeginPlay() override;
    void TickComponent(float DeltaSeconds) override;
    void EndPlay() override;

    static void ResetCounters();
    [[nodiscard]] static int GetBeginPlayCount();
    [[nodiscard]] static int GetTickCount();
    [[nodiscard]] static int GetEndPlayCount();

protected:
    explicit ULifecycleTestComponent(const FObjectInitializer& ObjectInitializer);

private:
    static int BeginPlayCount;
    static int TickCount;
    static int EndPlayCount;
};

UCLASS()
class UWorldTrackingComponent final : public UActorComponent
{
    GENERATED_BODY()

public:
    static void ResetUnregisterObservation(UWorld* InExpectedWorld);
    [[nodiscard]] static bool ObservedExpectedWorld();
    [[nodiscard]] static int GetUnregisterCount();

protected:
    explicit UWorldTrackingComponent(const FObjectInitializer& ObjectInitializer);
    void OnUnregister() override;

private:
    static UWorld* ExpectedWorld;
    static bool bObservedExpectedWorld;
    static int UnregisterCount;
};

UCLASS()
class UProxyTrackingPrimitiveComponent final : public UPrimitiveComponent
{
    GENERATED_BODY()

public:
    static void ResetUnregisterObservation(UWorld* InExpectedWorld);
    [[nodiscard]] static bool ObservedExpectedWorld();
    [[nodiscard]] static std::size_t GetPrimitiveCountBeforeUnregister();
    [[nodiscard]] static std::size_t GetPrimitiveCountAfterUnregister();

protected:
    explicit UProxyTrackingPrimitiveComponent(const FObjectInitializer& ObjectInitializer);
    void OnUnregister() override;

private:
    static UWorld* ExpectedWorld;
    static bool bObservedExpectedWorld;
    static std::size_t PrimitiveCountBeforeUnregister;
    static std::size_t PrimitiveCountAfterUnregister;
};
