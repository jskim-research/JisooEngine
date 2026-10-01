#pragma once

#include "Runtime/Engine/Components/SceneComponent.h"
#include "Runtime/Render/Scene/PrimitiveSceneHandle.h"

#include <memory>

#include "Runtime/Engine/Components/PrimitiveComponent.generated.h"

class FPrimitiveSceneProxy;
struct FPrimitiveSceneDescription;

UCLASS()
/**
 * 렌더링·충돌에 사용될 공간 경계와 기본 렌더 의도를 보관한다.
 * 등록된 동안 FScene Handle로 Render Scene 상태를 동기화하며 Proxy를 직접 참조하지 않는다.
 */
class UPrimitiveComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    [[nodiscard]] bool IsVisible() const;
    void SetVisibility(bool bInVisible);
    [[nodiscard]] bool CastsShadow() const;
    void SetCastShadow(bool bInCastShadow);

    [[nodiscard]] const FBox& GetLocalBounds() const;
    void SetLocalBounds(const FBox& InBounds);
    [[nodiscard]] const FBox& GetWorldBounds() const;
    [[nodiscard]] FPrimitiveSceneHandle GetPrimitiveSceneHandle() const;

protected:
    explicit UPrimitiveComponent(const FObjectInitializer& ObjectInitializer);
    ~UPrimitiveComponent() override;

    virtual std::unique_ptr<FPrimitiveSceneProxy> CreateSceneProxy() const;
    [[nodiscard]] FPrimitiveSceneDescription BuildSceneDescription() const;

    void OnRegister() override;
    void OnUnregister() override;
    void OnTransformChanged() override;

private:
    void MarkBoundsDirty();
    void SendRenderTransform();
    void SendRenderBounds();
    void SendRenderFlags();

    bool bVisible = true;
    bool bCastShadow = true;
    FBox LocalBounds;
    mutable FBox WorldBounds;
    mutable bool bWorldBoundsDirty = true;
    FPrimitiveSceneHandle PrimitiveSceneHandle;
};
