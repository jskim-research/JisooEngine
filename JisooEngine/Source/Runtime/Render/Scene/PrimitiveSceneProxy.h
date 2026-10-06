#pragma once

#include "Runtime/Core/Math/MathTypes.h"

class FMeshBatchCollector;

struct FPrimitiveSceneDescription
{
    FMatrix LocalToWorld = FMatrix::Identity();
    FBox LocalBounds;
    FBox WorldBounds;
    bool bVisible = true;
    bool bCastShadow = true;
};

struct FPrimitiveTransformUpdate
{
    FMatrix LocalToWorld = FMatrix::Identity();
    FBox WorldBounds;
};

struct FPrimitiveBoundsUpdate
{
    FBox LocalBounds;
    FBox WorldBounds;
};

struct FPrimitiveFlagsUpdate
{
    bool bVisible = true;
    bool bCastShadow = true;
};

/**
 * Renderer가 Game Scene 객체를 역참조하지 않고 읽을 Primitive 상태를 보관한다.
 * FScene만 상태를 갱신하며 파생 Proxy는 이후 Geometry·Material 표현을 확장한다.
 */
class FPrimitiveSceneProxy
{
public:
    explicit FPrimitiveSceneProxy(const FPrimitiveSceneDescription& Description);
    virtual ~FPrimitiveSceneProxy();

    FPrimitiveSceneProxy(const FPrimitiveSceneProxy&) = delete;
    FPrimitiveSceneProxy& operator=(const FPrimitiveSceneProxy&) = delete;

    [[nodiscard]] const FMatrix& GetLocalToWorld() const;
    [[nodiscard]] const FBox& GetLocalBounds() const;
    [[nodiscard]] const FBox& GetWorldBounds() const;
    [[nodiscard]] bool IsVisible() const;
    [[nodiscard]] bool CastsShadow() const;

    /** 현재 Proxy가 표현하는 Pass 독립적인 Mesh Draw 후보를 Collector에 제출한다. */
    virtual void GatherMeshBatches(FMeshBatchCollector& Collector) const;

private:
    void ApplyTransform(const FPrimitiveTransformUpdate& Update);
    void ApplyBounds(const FPrimitiveBoundsUpdate& Update);
    void ApplyFlags(const FPrimitiveFlagsUpdate& Update);

    FMatrix LocalToWorld = FMatrix::Identity();
    FBox LocalBounds;
    FBox WorldBounds;
    bool bVisible = true;
    bool bCastShadow = true;

    friend class FScene;
};
