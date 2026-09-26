#include "Runtime/Engine/Components/PrimitiveComponent.h"

UPrimitiveComponent::UPrimitiveComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UPrimitiveComponent::~UPrimitiveComponent() = default;

bool UPrimitiveComponent::IsVisible() const
{
    return bVisible;
}

void UPrimitiveComponent::SetVisibility(bool bInVisible)
{
    bVisible = bInVisible;
}

bool UPrimitiveComponent::CastsShadow() const
{
    return bCastShadow;
}

void UPrimitiveComponent::SetCastShadow(bool bInCastShadow)
{
    bCastShadow = bInCastShadow;
}

const FBox& UPrimitiveComponent::GetLocalBounds() const
{
    return LocalBounds;
}

void UPrimitiveComponent::SetLocalBounds(const FBox& InBounds)
{
    LocalBounds = InBounds;
    MarkBoundsDirty();
}

const FBox& UPrimitiveComponent::GetWorldBounds() const
{
    if (bWorldBoundsDirty)
    {
        WorldBounds = LocalBounds.TransformBy(GetComponentToWorld());
        bWorldBoundsDirty = false;
    }
    return WorldBounds;
}

void UPrimitiveComponent::OnTransformChanged()
{
    Super::OnTransformChanged();
    MarkBoundsDirty();
}

void UPrimitiveComponent::MarkBoundsDirty()
{
    bWorldBoundsDirty = true;
}
