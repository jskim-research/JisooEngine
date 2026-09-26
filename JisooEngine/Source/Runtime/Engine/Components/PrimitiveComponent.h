#pragma once

#include "Runtime/Engine/Components/SceneComponent.h"

#include "Runtime/Engine/Components/PrimitiveComponent.generated.h"

UCLASS()
/**
 * 렌더링·충돌에 사용될 공간 경계와 기본 렌더 의도를 보관한다.
 * 현재 단계에서는 FScene이나 GPU 자원을 직접 참조하지 않는다.
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

protected:
    explicit UPrimitiveComponent(const FObjectInitializer& ObjectInitializer);
    ~UPrimitiveComponent() override;

    void OnTransformChanged() override;

private:
    void MarkBoundsDirty();

    bool bVisible = true;
    bool bCastShadow = true;
    FBox LocalBounds;
    mutable FBox WorldBounds;
    mutable bool bWorldBoundsDirty = true;
};
