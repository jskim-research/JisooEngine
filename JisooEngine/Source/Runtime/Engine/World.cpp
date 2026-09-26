#include "Runtime/Engine/World.h"

#include "Runtime/Engine/Actor.h"

#include <algorithm>

UWorld::UWorld(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UWorld::~UWorld() = default;

bool UWorld::DestroyActor(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return false;
    }

    const FObjectHandle ActorHandle = Actor->GetHandle();
    const auto ActorIterator = std::ranges::find(Actors, ActorHandle);
    if (ActorIterator == Actors.end())
    {
        return false;
    }

    Actors.erase(ActorIterator);
    DestroyingActors.push_back(ActorHandle);
    Actor->RouteActorDestroyed();
    DestroyObject(Actor);
    return true;
}

std::vector<AActor*> UWorld::GetActors() const
{
    std::vector<AActor*> Result;
    Result.reserve(Actors.size());
    for (const FObjectHandle Handle : Actors)
    {
        if (AActor* Actor = Cast<AActor>(ResolveObject(Handle)))
        {
            Result.push_back(Actor);
        }
    }
    return Result;
}

bool UWorld::HasBegunPlay() const
{
    return bHasBegunPlay;
}

void UWorld::BeginPlay()
{
    if (bHasBegunPlay || bWorldDestroyRouted)
    {
        return;
    }

    bHasBegunPlay = true;
    const std::vector<FObjectHandle> ActorSnapshot = Actors;
    for (const FObjectHandle Handle : ActorSnapshot)
    {
        if (AActor* Actor = Cast<AActor>(ResolveObject(Handle)))
        {
            Actor->DispatchBeginPlay();
        }
    }
}

void UWorld::Tick(float DeltaSeconds)
{
    if (!bHasBegunPlay || bWorldDestroyRouted)
    {
        return;
    }

    const std::vector<FObjectHandle> ActorSnapshot = Actors;
    for (const FObjectHandle Handle : ActorSnapshot)
    {
        if (AActor* Actor = Cast<AActor>(ResolveObject(Handle)))
        {
            Actor->DispatchTick(DeltaSeconds);
        }
    }
}

void UWorld::EndPlay()
{
    if (!bHasBegunPlay)
    {
        return;
    }

    const std::vector<FObjectHandle> ActorSnapshot = Actors;
    for (const FObjectHandle Handle : ActorSnapshot)
    {
        if (AActor* Actor = Cast<AActor>(ResolveObject(Handle)))
        {
            Actor->DispatchEndPlay();
        }
    }
    bHasBegunPlay = false;
}

void UWorld::BeginDestroy()
{
    RouteWorldDestroyed();
    Super::BeginDestroy();
}

bool UWorld::IsReadyForFinishDestroy() const
{
    if (!Super::IsReadyForFinishDestroy())
    {
        return false;
    }

    const auto IsStillAllocated = [](FObjectHandle Handle)
    {
        return IsObjectAllocated(Handle);
    };
    return std::ranges::none_of(Actors, IsStillAllocated)
        && std::ranges::none_of(DestroyingActors, IsStillAllocated);
}

void UWorld::AddActor(AActor* Actor)
{
    if (!IsValid(Actor) || Actor->GetOuter() != this)
    {
        return;
    }

    Actors.push_back(Actor->GetHandle());
    if (bHasBegunPlay)
    {
        Actor->DispatchBeginPlay();
    }
}

void UWorld::RouteWorldDestroyed()
{
    if (bWorldDestroyRouted)
    {
        return;
    }

    bWorldDestroyRouted = true;
    EndPlay();

    const std::vector<FObjectHandle> ActorSnapshot = Actors;
    for (const FObjectHandle Handle : ActorSnapshot)
    {
        if (AActor* Actor = Cast<AActor>(ResolveObject(Handle)))
        {
            Actor->RouteActorDestroyed();
            DestroyObject(Actor);
        }
    }
    DestroyingActors.insert(
        DestroyingActors.end(),
        Actors.begin(),
        Actors.end());
    Actors.clear();
}
