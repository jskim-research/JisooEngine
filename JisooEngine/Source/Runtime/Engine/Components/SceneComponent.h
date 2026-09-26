#pragma once

#include "Runtime/Core/Math/MathTypes.h"
#include "Runtime/Engine/Components/ActorComponent.h"

#include <vector>

#include "Runtime/Engine/Components/SceneComponent.generated.h"

UCLASS()
/**
 * Actor 안의 공간 계층과 상대 Transform을 관리한다.
 * 행벡터 규칙에 따라 ComponentToWorld는 Local * ParentWorld 순서로 계산한다.
 */
class USceneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /** 같은 Actor가 소유한 SceneComponent 아래에 부착한다. nullptr이면 현재 부모에서 분리한다. */
    bool AttachToComponent(USceneComponent* InParent);
    void DetachFromParent();

    [[nodiscard]] USceneComponent* GetAttachParent() const;
    [[nodiscard]] std::vector<USceneComponent*> GetAttachChildren() const;

    [[nodiscard]] const FTransform& GetRelativeTransform() const;
    void SetRelativeTransform(const FTransform& InTransform);
    [[nodiscard]] const FMatrix& GetComponentToWorld() const;

    void BeginDestroy() override;

protected:
    explicit USceneComponent(const FObjectInitializer& ObjectInitializer);
    ~USceneComponent() override;

    virtual void OnTransformChanged();

private:
    [[nodiscard]] bool IsDescendantOf(const USceneComponent* Component) const;
    void MarkWorldTransformDirty();

    FObjectHandle AttachParentHandle;
    std::vector<FObjectHandle> AttachChildren;
    FTransform RelativeTransform;
    mutable FMatrix ComponentToWorld = FMatrix::Identity();
    mutable bool bWorldTransformDirty = true;
};
