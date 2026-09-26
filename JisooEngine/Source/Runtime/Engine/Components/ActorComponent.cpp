#include "Runtime/Engine/Components/ActorComponent.h"

#include "Runtime/CoreUObject/ObjectGlobals.h"
#include "Runtime/Engine/Actor.h"

UActorComponent::UActorComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UActorComponent::~UActorComponent() = default;

AActor* UActorComponent::GetOwner() const
{
    return Cast<AActor>(ResolveObject(OwnerHandle));
}

UWorld* UActorComponent::GetWorld() const
{
    AActor* Owner = GetOwner();
    return Owner != nullptr ? Owner->GetWorld() : nullptr;
}

bool UActorComponent::IsRegistered() const
{
    return bRegistered;
}

bool UActorComponent::HasBegunPlay() const
{
    return bHasBegunPlay;
}

bool UActorComponent::IsComponentTickEnabled() const
{
    return bTickEnabled;
}

void UActorComponent::SetComponentTickEnabled(bool bEnabled)
{
    bTickEnabled = bEnabled;
}

void UActorComponent::BeginPlay()
{
}

void UActorComponent::TickComponent(float)
{
}

void UActorComponent::EndPlay()
{
}

void UActorComponent::BeginDestroy()
{
    UnregisterComponent();
    Super::BeginDestroy();
}

void UActorComponent::OnRegister()
{
}

void UActorComponent::OnUnregister()
{
}

void UActorComponent::RegisterComponent(AActor* InOwner)
{
    if (bRegistered || InOwner == nullptr)
    {
        return;
    }

    OwnerHandle = InOwner->GetHandle();
    bRegistered = true;
    OnRegister();
}

void UActorComponent::UnregisterComponent()
{
    if (!bRegistered)
    {
        return;
    }

    DispatchEndPlay();
    OnUnregister();
    bRegistered = false;
    OwnerHandle = {};
}

void UActorComponent::DispatchBeginPlay()
{
    if (!bRegistered || bHasBegunPlay)
    {
        return;
    }

    bHasBegunPlay = true;
    BeginPlay();
}

void UActorComponent::DispatchTick(float DeltaSeconds)
{
    if (bRegistered && bHasBegunPlay && bTickEnabled)
    {
        TickComponent(DeltaSeconds);
    }
}

void UActorComponent::DispatchEndPlay()
{
    if (!bHasBegunPlay)
    {
        return;
    }

    EndPlay();
    bHasBegunPlay = false;
}
