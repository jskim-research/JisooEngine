#include "Runtime/Engine/Actor.h"

#include "Runtime/Engine/Components/ActorComponent.h"
#include "Runtime/Engine/Components/SceneComponent.h"
#include "Runtime/Engine/World.h"

#include <algorithm>

AActor::AActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

AActor::~AActor() = default;

UWorld* AActor::GetWorld() const
{
    return Cast<UWorld>(GetOuter());
}

bool AActor::DestroyComponent(UActorComponent* Component)
{
    if (!IsValid(Component) || !OwnsComponent(Component))
    {
        return false;
    }

    const FObjectHandle ComponentHandle = Component->GetHandle();
    if (RootComponentHandle == ComponentHandle)
    {
        RootComponentHandle = {};
    }

    if (USceneComponent* SceneComponent = Cast<USceneComponent>(Component))
    {
        const std::vector<USceneComponent*> Children = SceneComponent->GetAttachChildren();
        for (USceneComponent* Child : Children)
        {
            Child->DetachFromParent();
        }
        SceneComponent->DetachFromParent();
    }

    Component->UnregisterComponent();
    std::erase(OwnedComponents, ComponentHandle);
    DestroyingComponents.push_back(ComponentHandle);
    DestroyObject(Component);
    return true;
}

std::vector<UActorComponent*> AActor::GetComponents() const
{
    std::vector<UActorComponent*> Result;
    Result.reserve(OwnedComponents.size());
    for (const FObjectHandle Handle : OwnedComponents)
    {
        if (UActorComponent* Component = Cast<UActorComponent>(ResolveObject(Handle)))
        {
            Result.push_back(Component);
        }
    }
    return Result;
}

bool AActor::SetRootComponent(USceneComponent* Component)
{
    if (Component == nullptr)
    {
        RootComponentHandle = {};
        return true;
    }

    if (!OwnsComponent(Component))
    {
        return false;
    }

    Component->DetachFromParent();
    RootComponentHandle = Component->GetHandle();
    return true;
}

USceneComponent* AActor::GetRootComponent() const
{
    return Cast<USceneComponent>(ResolveObject(RootComponentHandle));
}

bool AActor::HasBegunPlay() const
{
    return bHasBegunPlay;
}

void AActor::BeginPlay()
{
}

void AActor::Tick(float)
{
}

void AActor::EndPlay()
{
}

void AActor::BeginDestroy()
{
    RouteActorDestroyed();
    Super::BeginDestroy();
}

bool AActor::IsReadyForFinishDestroy() const
{
    if (!Super::IsReadyForFinishDestroy())
    {
        return false;
    }

    const auto IsStillAllocated = [](FObjectHandle Handle)
    {
        return IsObjectAllocated(Handle);
    };
    return std::ranges::none_of(OwnedComponents, IsStillAllocated)
        && std::ranges::none_of(DestroyingComponents, IsStillAllocated);
}

bool AActor::AddOwnedComponent(UActorComponent* Component)
{
    if (!IsValid(Component)
        || Component->GetOuter() != this
        || OwnsComponent(Component))
    {
        return false;
    }

    Component->RegisterComponent(this);
    OwnedComponents.push_back(Component->GetHandle());

    if (!RootComponentHandle.IsSet())
    {
        if (USceneComponent* SceneComponent = Cast<USceneComponent>(Component))
        {
            RootComponentHandle = SceneComponent->GetHandle();
        }
    }

    if (bHasBegunPlay)
    {
        Component->DispatchBeginPlay();
    }
    return true;
}

bool AActor::OwnsComponent(const UActorComponent* Component) const
{
    if (Component == nullptr)
    {
        return false;
    }
    return std::ranges::find(OwnedComponents, Component->GetHandle()) != OwnedComponents.end();
}

void AActor::DispatchBeginPlay()
{
    if (bHasBegunPlay || bActorDestroyRouted)
    {
        return;
    }

    bHasBegunPlay = true;
    const std::vector<FObjectHandle> ComponentSnapshot = OwnedComponents;
    for (const FObjectHandle Handle : ComponentSnapshot)
    {
        if (UActorComponent* Component = Cast<UActorComponent>(ResolveObject(Handle)))
        {
            Component->DispatchBeginPlay();
        }
    }
    BeginPlay();
}

void AActor::DispatchTick(float DeltaSeconds)
{
    if (!bHasBegunPlay || bActorDestroyRouted)
    {
        return;
    }

    const std::vector<FObjectHandle> ComponentSnapshot = OwnedComponents;
    Tick(DeltaSeconds);

    for (const FObjectHandle Handle : ComponentSnapshot)
    {
        if (UActorComponent* Component = Cast<UActorComponent>(ResolveObject(Handle)))
        {
            Component->DispatchTick(DeltaSeconds);
        }
    }
}

void AActor::DispatchEndPlay()
{
    if (!bHasBegunPlay)
    {
        return;
    }

    const std::vector<FObjectHandle> ComponentSnapshot = OwnedComponents;
    EndPlay();
    for (const FObjectHandle Handle : ComponentSnapshot)
    {
        if (UActorComponent* Component = Cast<UActorComponent>(ResolveObject(Handle)))
        {
            Component->DispatchEndPlay();
        }
    }
    bHasBegunPlay = false;
}

void AActor::RouteActorDestroyed()
{
    if (bActorDestroyRouted)
    {
        return;
    }

    bActorDestroyRouted = true;
    DispatchEndPlay();

    const std::vector<FObjectHandle> ComponentSnapshot = OwnedComponents;
    for (const FObjectHandle Handle : ComponentSnapshot)
    {
        if (UActorComponent* Component = Cast<UActorComponent>(ResolveObject(Handle)))
        {
            Component->UnregisterComponent();
            DestroyObject(Component);
        }
    }
    DestroyingComponents.insert(
        DestroyingComponents.end(),
        OwnedComponents.begin(),
        OwnedComponents.end());
    OwnedComponents.clear();
    RootComponentHandle = {};
}
