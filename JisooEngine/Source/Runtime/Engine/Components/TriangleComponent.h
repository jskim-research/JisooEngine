#pragma once

#include "Runtime/Engine/Components/PrimitiveComponent.h"

#include <memory>

#include "Runtime/Engine/Components/TriangleComponent.generated.h"

UCLASS()
/** Mesh 제출 경로를 수직 검증하기 위한 단일 삼각형 Primitive이다. */
class UTriangleComponent final : public UPrimitiveComponent
{
    GENERATED_BODY()

protected:
    explicit UTriangleComponent(const FObjectInitializer& ObjectInitializer);
    ~UTriangleComponent() override;

    std::unique_ptr<FPrimitiveSceneProxy> CreateSceneProxy() const override;
};
