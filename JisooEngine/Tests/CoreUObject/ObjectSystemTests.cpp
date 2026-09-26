#include "CoreUObject/ExampleObject.h"

#include "Runtime/CoreUObject/Class.h"
#include "Runtime/CoreUObject/ObjectArray.h"
#include "Runtime/CoreUObject/ObjectGlobals.h"
#include "Runtime/Engine/Components/PrimitiveComponent.h"
#include "Runtime/Engine/Components/SceneComponent.h"
#include "Runtime/Engine/Engine.h"
#include "Runtime/Engine/World.h"

#include <cmath>
#include <iostream>
#include <string_view>

namespace
{
    int FailureCount = 0;

    void Expect(bool Condition, std::string_view Message)
    {
        if (Condition)
        {
            return;
        }

        ++FailureCount;
        std::cerr << "FAILED: " << Message << '\n';
    }

    bool NearlyEqual(float Left, float Right)
    {
        return std::abs(Left - Right) < 0.0001f;
    }
}

int main()
{
    UExampleObject::ResetCounters();

    UObject* BaseObject = NewObject<UObject>(nullptr, "Base");
    UExampleObject* DerivedObject = NewObject<UExampleObject>(BaseObject, "Derived");

    Expect(BaseObject->GetClass() == UObject::StaticClass(), "UObject runtime class");
    Expect(
        DerivedObject->GetClass() == UExampleObject::StaticClass(),
        "derived runtime class");
    Expect(BaseObject->IsA(UObject::StaticClass()), "UObject IsA UObject");
    Expect(!BaseObject->IsA(UExampleObject::StaticClass()), "UObject is not derived");
    Expect(DerivedObject->IsA(UObject::StaticClass()), "derived IsA UObject");
    Expect(
        DerivedObject->IsA(UExampleObject::StaticClass()),
        "derived IsA own class");
    Expect(Cast<UExampleObject>(BaseObject) == nullptr, "invalid Cast returns nullptr");
    Expect(Cast<UExampleObject>(DerivedObject) == DerivedObject, "valid Cast succeeds");
    Expect(DerivedObject->GetOuter() == BaseObject, "Outer is preserved");
    Expect(DerivedObject->GetPathName() == "Base.Derived", "path follows Outer chain");

    const FObjectHandle OldHandle = DerivedObject->GetHandle();
    Expect(IsValid(DerivedObject), "new object is valid");
    Expect(ResolveObject(OldHandle) == DerivedObject, "handle resolves live object");

    DestroyObject(DerivedObject);
    DestroyObject(DerivedObject);
    Expect(!IsValid(DerivedObject), "pending destroy object is invalid");
    Expect(ResolveObject(OldHandle) == nullptr, "pending destroy handle does not resolve");
    FlushPendingDestroyObjects();

    Expect(UExampleObject::GetBeginDestroyCount() == 1, "BeginDestroy runs once");
    Expect(UExampleObject::GetFinishDestroyCount() == 1, "FinishDestroy runs once");
    Expect(UExampleObject::GetDestructorCount() == 1, "destructor runs once");
    Expect(!IsValid(OldHandle), "destroyed handle is stale");

    UExampleObject* Replacement = NewObject<UExampleObject>(BaseObject, "Replacement");
    const FObjectHandle ReplacementHandle = Replacement->GetHandle();
    Expect(ReplacementHandle.Index == OldHandle.Index, "freed slot is reused");
    Expect(
        ReplacementHandle.SerialNumber != OldHandle.SerialNumber,
        "reused slot receives a new serial");
    Expect(ResolveObject(OldHandle) == nullptr, "old serial cannot resolve replacement");

    DestroyObject(Replacement);
    FlushPendingDestroyObjects();
    DestroyObject(BaseObject);
    FlushPendingDestroyObjects();
    Expect(GUObjectArray.GetObjectCount() == 0, "all test objects are released");

    UTestActor::ResetCounters();
    UTestActorComponent::ResetCounters();

    {
        FEngine Engine;
        UWorld* World = Engine.CreateWorld("TestWorld");
        Expect(World != nullptr, "engine creates world");
        World->BeginPlay();

        UTestActor* Actor = World->SpawnActor<UTestActor>("Actor");
        UTestActorComponent* TickComponent = Actor->AddComponent<UTestActorComponent>("TickComponent");
        USceneComponent* RootComponent = Actor->AddComponent<USceneComponent>("Root");
        UPrimitiveComponent* PrimitiveComponent = Actor->AddComponent<UPrimitiveComponent>("Primitive");

        Expect(Actor->GetWorld() == World, "actor resolves owning world");
        Expect(TickComponent->GetOwner() == Actor, "component resolves owning actor");
        Expect(TickComponent->GetWorld() == World, "component resolves owning world");
        Expect(TickComponent->IsRegistered(), "component registers with actor");
        Expect(Actor->GetRootComponent() == RootComponent, "first scene component becomes root");
        Expect(UTestActor::GetBeginPlayCount() == 1, "spawned actor begins play in active world");
        Expect(UTestActorComponent::GetBeginPlayCount() == 1, "component added during play begins play");

        Expect(PrimitiveComponent->AttachToComponent(RootComponent), "scene component attaches to same actor");
        Expect(!RootComponent->AttachToComponent(PrimitiveComponent), "scene attachment rejects cycles");
        PrimitiveComponent->SetVisibility(false);
        PrimitiveComponent->SetCastShadow(false);
        Expect(!PrimitiveComponent->IsVisible(), "primitive stores visibility intent");
        Expect(!PrimitiveComponent->CastsShadow(), "primitive stores shadow intent");

        FTransform RootTransform;
        RootTransform.Translation = FVector{10.0f, 0.0f, 0.0f};
        RootComponent->SetRelativeTransform(RootTransform);

        FTransform PrimitiveTransform;
        PrimitiveTransform.Translation = FVector{5.0f, 0.0f, 0.0f};
        PrimitiveComponent->SetRelativeTransform(PrimitiveTransform);
        PrimitiveComponent->SetLocalBounds(FBox{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}});

        const FMatrix& ComponentToWorld = PrimitiveComponent->GetComponentToWorld();
        Expect(NearlyEqual(ComponentToWorld.M[3][0], 15.0f), "child transform composes local and parent world");
        const FBox& WorldBounds = PrimitiveComponent->GetWorldBounds();
        Expect(NearlyEqual(WorldBounds.Min.X, 14.0f), "primitive world bounds minimum follows transform");
        Expect(NearlyEqual(WorldBounds.Max.X, 16.0f), "primitive world bounds maximum follows transform");

        Engine.Tick(0.25f);
        Expect(UTestActor::GetTickCount() == 1, "engine ticks world actor");
        Expect(UTestActorComponent::GetTickCount() == 1, "actor ticks component");

        UTestActorComponent* RemovedComponent =
            Actor->AddComponent<UTestActorComponent>("RemovedComponent");
        const FObjectHandle RemovedComponentHandle = RemovedComponent->GetHandle();
        Expect(Actor->DestroyComponent(RemovedComponent), "actor removes owned component");
        Expect(!IsValid(RemovedComponentHandle), "removed component immediately becomes invalid");
        Engine.Tick(0.0f);
        Expect(!IsObjectAllocated(RemovedComponentHandle), "engine tick flushes removed component");

        const int TickCountBeforeSpawn = UTestActor::GetTickCount();
        UTestActor::RequestSpawnOnNextTick();
        Engine.Tick(0.25f);
        Expect(
            UTestActor::GetTickCount() == TickCountBeforeSpawn + 1,
            "actor spawned during tick waits until next frame");
        Expect(World->GetActors().size() == 2, "spawn during tick joins world container");

        Engine.Tick(0.25f);
        Expect(
            UTestActor::GetTickCount() == TickCountBeforeSpawn + 3,
            "spawned actor ticks on following frame");

        UTestActor::RequestDestroySelfOnNextTick();
        const FObjectHandle DestroyedActorHandle = Actor->GetHandle();
        const FObjectHandle DestroyedComponentHandle = TickComponent->GetHandle();
        Engine.Tick(0.25f);
        Expect(!IsObjectAllocated(DestroyedActorHandle), "engine tick flushes destroyed actor");
        Expect(!IsObjectAllocated(DestroyedComponentHandle), "actor destruction flushes owned component");
        Expect(World->GetActors().size() == 1, "destroyed actor leaves world container immediately");

        Engine.DestroyWorld(World);
        Engine.Tick(0.0f);
        Expect(GUObjectArray.GetObjectCount() == 0, "world destruction releases actor and component hierarchy");
    }

    UWorld* CascadingWorld = NewObject<UWorld>(nullptr, "CascadingWorld");
    AActor* CascadingActor = CascadingWorld->SpawnActor<AActor>("CascadingActor");
    [[maybe_unused]] UActorComponent* CascadingComponent =
        CascadingActor->AddComponent<UActorComponent>("CascadingComponent");
    DestroyObject(CascadingWorld);
    FlushPendingDestroyObjects();
    Expect(
        GUObjectArray.GetObjectCount() == 0,
        "flush preserves destroy requests queued by BeginDestroy and drains hierarchy");

    if (FailureCount != 0)
    {
        std::cerr << FailureCount << " object system test(s) failed.\n";
        return 1;
    }

    std::cout << "Object system tests passed.\n";
    return 0;
}
