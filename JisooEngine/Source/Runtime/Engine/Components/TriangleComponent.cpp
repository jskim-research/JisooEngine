#include "Runtime/Engine/Components/TriangleComponent.h"

#include "Runtime/Render/Scene/TriangleSceneProxy.h"

UTriangleComponent::UTriangleComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetLocalBounds({{-1.0f, -50.0f, -50.0f}, {1.0f, 50.0f, 50.0f}});
}

UTriangleComponent::~UTriangleComponent() = default;

std::unique_ptr<FPrimitiveSceneProxy> UTriangleComponent::CreateSceneProxy() const
{
    return std::make_unique<FTriangleSceneProxy>(BuildSceneDescription());
}
