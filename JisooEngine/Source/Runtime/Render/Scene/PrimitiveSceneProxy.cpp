#include "Runtime/Render/Scene/PrimitiveSceneProxy.h"

FPrimitiveSceneProxy::FPrimitiveSceneProxy(const FPrimitiveSceneDescription& Description)
    : LocalToWorld(Description.LocalToWorld)
    , LocalBounds(Description.LocalBounds)
    , WorldBounds(Description.WorldBounds)
    , bVisible(Description.bVisible)
    , bCastShadow(Description.bCastShadow)
{
}

FPrimitiveSceneProxy::~FPrimitiveSceneProxy() = default;

const FMatrix& FPrimitiveSceneProxy::GetLocalToWorld() const
{
    return LocalToWorld;
}

const FBox& FPrimitiveSceneProxy::GetLocalBounds() const
{
    return LocalBounds;
}

const FBox& FPrimitiveSceneProxy::GetWorldBounds() const
{
    return WorldBounds;
}

bool FPrimitiveSceneProxy::IsVisible() const
{
    return bVisible;
}

bool FPrimitiveSceneProxy::CastsShadow() const
{
    return bCastShadow;
}

void FPrimitiveSceneProxy::ApplyTransform(const FPrimitiveTransformUpdate& Update)
{
    LocalToWorld = Update.LocalToWorld;
    WorldBounds = Update.WorldBounds;
}

void FPrimitiveSceneProxy::ApplyBounds(const FPrimitiveBoundsUpdate& Update)
{
    LocalBounds = Update.LocalBounds;
    WorldBounds = Update.WorldBounds;
}

void FPrimitiveSceneProxy::ApplyFlags(const FPrimitiveFlagsUpdate& Update)
{
    bVisible = Update.bVisible;
    bCastShadow = Update.bCastShadow;
}
