#include "Runtime/Engine/Components/PrimitiveComponent.h"

#include "Runtime/Engine/World.h"
#include "Runtime/Render/Scene/PrimitiveSceneProxy.h"
#include "Runtime/Render/Scene/Scene.h"

#include <memory>

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
    if (bVisible == bInVisible)
    {
        return;
    }

    bVisible = bInVisible;
    SendRenderFlags();
}

bool UPrimitiveComponent::CastsShadow() const
{
    return bCastShadow;
}

void UPrimitiveComponent::SetCastShadow(bool bInCastShadow)
{
    if (bCastShadow == bInCastShadow)
    {
        return;
    }

    bCastShadow = bInCastShadow;
    SendRenderFlags();
}

const FBox& UPrimitiveComponent::GetLocalBounds() const
{
    return LocalBounds;
}

void UPrimitiveComponent::SetLocalBounds(const FBox& InBounds)
{
    LocalBounds = InBounds;
    MarkBoundsDirty();
    SendRenderBounds();
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

FPrimitiveSceneHandle UPrimitiveComponent::GetPrimitiveSceneHandle() const
{
    return PrimitiveSceneHandle;
}

std::unique_ptr<FPrimitiveSceneProxy> UPrimitiveComponent::CreateSceneProxy() const
{
    return std::make_unique<FPrimitiveSceneProxy>(BuildSceneDescription());
}

FPrimitiveSceneDescription UPrimitiveComponent::BuildSceneDescription() const
{
    return {
        GetComponentToWorld(),
        GetLocalBounds(),
        GetWorldBounds(),
        IsVisible(),
        CastsShadow()};
}

void UPrimitiveComponent::OnRegister()
{
    Super::OnRegister();

    UWorld* World = GetWorld();
    FScene* Scene = World != nullptr ? World->GetScene() : nullptr;
    if (Scene == nullptr || PrimitiveSceneHandle.IsSet())
    {
        return;
    }

    std::unique_ptr<FPrimitiveSceneProxy> Proxy = CreateSceneProxy();
    if (Proxy != nullptr)
    {
        PrimitiveSceneHandle = Scene->AddPrimitive(std::move(Proxy));
    }
}

void UPrimitiveComponent::OnUnregister()
{
    if (PrimitiveSceneHandle.IsSet())
    {
        UWorld* World = GetWorld();
        FScene* Scene = World != nullptr ? World->GetScene() : nullptr;
        if (Scene != nullptr)
        {
            Scene->RemovePrimitive(PrimitiveSceneHandle);
        }
        PrimitiveSceneHandle = {};
    }

    Super::OnUnregister();
}

void UPrimitiveComponent::OnTransformChanged()
{
    Super::OnTransformChanged();
    MarkBoundsDirty();
    SendRenderTransform();
}

void UPrimitiveComponent::MarkBoundsDirty()
{
    bWorldBoundsDirty = true;
}

void UPrimitiveComponent::SendRenderTransform()
{
    if (!PrimitiveSceneHandle.IsSet())
    {
        return;
    }

    UWorld* World = GetWorld();
    FScene* Scene = World != nullptr ? World->GetScene() : nullptr;
    if (Scene != nullptr)
    {
        Scene->UpdatePrimitiveTransform(
            PrimitiveSceneHandle,
            {GetComponentToWorld(), GetWorldBounds()});
    }
}

void UPrimitiveComponent::SendRenderBounds()
{
    if (!PrimitiveSceneHandle.IsSet())
    {
        return;
    }

    UWorld* World = GetWorld();
    FScene* Scene = World != nullptr ? World->GetScene() : nullptr;
    if (Scene != nullptr)
    {
        Scene->UpdatePrimitiveBounds(
            PrimitiveSceneHandle,
            {GetLocalBounds(), GetWorldBounds()});
    }
}

void UPrimitiveComponent::SendRenderFlags()
{
    if (!PrimitiveSceneHandle.IsSet())
    {
        return;
    }

    UWorld* World = GetWorld();
    FScene* Scene = World != nullptr ? World->GetScene() : nullptr;
    if (Scene != nullptr)
    {
        Scene->UpdatePrimitiveFlags(
            PrimitiveSceneHandle,
            {IsVisible(), CastsShadow()});
    }
}
