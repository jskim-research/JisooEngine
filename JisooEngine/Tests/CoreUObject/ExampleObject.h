#pragma once

#include "Runtime/CoreUObject/Object.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/Components/ActorComponent.h"

#include "CoreUObject/ExampleObject.generated.h"

UCLASS()
class UExampleObject final : public UObject
{
    GENERATED_BODY()

public:
    ~UExampleObject() override;

    void BeginDestroy() override;
    void FinishDestroy() override;

    static void ResetCounters();
    [[nodiscard]] static int GetBeginDestroyCount();
    [[nodiscard]] static int GetFinishDestroyCount();
    [[nodiscard]] static int GetDestructorCount();

protected:
    explicit UExampleObject(const FObjectInitializer& ObjectInitializer);

private:
    static int BeginDestroyCount;
    static int FinishDestroyCount;
    static int DestructorCount;
};

UCLASS()
class UTestActor final : public AActor
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
    explicit UTestActor(const FObjectInitializer& ObjectInitializer);

private:
    static int BeginPlayCount;
    static int TickCount;
    static int EndPlayCount;
    static bool bSpawnOnNextTick;
    static bool bDestroySelfOnNextTick;
};

UCLASS()
class UTestActorComponent final : public UActorComponent
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
    explicit UTestActorComponent(const FObjectInitializer& ObjectInitializer);

private:
    static int BeginPlayCount;
    static int TickCount;
    static int EndPlayCount;
};
