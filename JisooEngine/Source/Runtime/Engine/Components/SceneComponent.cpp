#include "Runtime/Engine/Components/SceneComponent.h"

#include "Runtime/CoreUObject/ObjectGlobals.h"

#include <algorithm>

USceneComponent::USceneComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

USceneComponent::~USceneComponent() = default;

bool USceneComponent::AttachToComponent(USceneComponent* InParent)
{
    if (InParent == nullptr)
    {
        DetachFromParent();
        return true;
    }

    if (InParent == this
        || InParent->GetOwner() != GetOwner()
        || InParent->IsDescendantOf(this))
    {
        return false;
    }

    if (GetAttachParent() == InParent)
    {
        return true;
    }

    DetachFromParent();
    AttachParentHandle = InParent->GetHandle();
    InParent->AttachChildren.push_back(GetHandle());
    MarkWorldTransformDirty();
    return true;
}

void USceneComponent::DetachFromParent()
{
    USceneComponent* Parent = GetAttachParent();
    if (Parent != nullptr)
    {
        const FObjectHandle ThisHandle = GetHandle();
        std::erase(Parent->AttachChildren, ThisHandle);
    }

    AttachParentHandle = {};
    MarkWorldTransformDirty();
}

USceneComponent* USceneComponent::GetAttachParent() const
{
    return Cast<USceneComponent>(ResolveObject(AttachParentHandle));
}

std::vector<USceneComponent*> USceneComponent::GetAttachChildren() const
{
    std::vector<USceneComponent*> Result;
    Result.reserve(AttachChildren.size());
    for (const FObjectHandle Handle : AttachChildren)
    {
        if (USceneComponent* Child = Cast<USceneComponent>(ResolveObject(Handle)))
        {
            Result.push_back(Child);
        }
    }
    return Result;
}

const FTransform& USceneComponent::GetRelativeTransform() const
{
    return RelativeTransform;
}

void USceneComponent::SetRelativeTransform(const FTransform& InTransform)
{
    RelativeTransform = InTransform;
    MarkWorldTransformDirty();
}

const FMatrix& USceneComponent::GetComponentToWorld() const
{
    if (bWorldTransformDirty)
    {
        ComponentToWorld = RelativeTransform.ToMatrixWithScale();
        if (const USceneComponent* Parent = GetAttachParent())
        {
            ComponentToWorld = ComponentToWorld * Parent->GetComponentToWorld();
        }
        bWorldTransformDirty = false;
    }
    return ComponentToWorld;
}

void USceneComponent::BeginDestroy()
{
    const std::vector<USceneComponent*> Children = GetAttachChildren();
    for (USceneComponent* Child : Children)
    {
        Child->AttachParentHandle = {};
        Child->MarkWorldTransformDirty();
    }
    AttachChildren.clear();
    DetachFromParent();
    Super::BeginDestroy();
}

void USceneComponent::OnTransformChanged()
{
}

bool USceneComponent::IsDescendantOf(const USceneComponent* Component) const
{
    for (const USceneComponent* Current = GetAttachParent();
         Current != nullptr;
         Current = Current->GetAttachParent())
    {
        if (Current == Component)
        {
            return true;
        }
    }
    return false;
}

void USceneComponent::MarkWorldTransformDirty()
{
    bWorldTransformDirty = true;
    OnTransformChanged();

    const std::vector<USceneComponent*> Children = GetAttachChildren();
    for (USceneComponent* Child : Children)
    {
        Child->MarkWorldTransformDirty();
    }
}
