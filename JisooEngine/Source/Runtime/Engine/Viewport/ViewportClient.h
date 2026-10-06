#pragma once

#include "Runtime/Core/Math/MathTypes.h"
#include "Runtime/Engine/Viewport/SceneView.h"

class FRenderer;
class FScene;
class FViewport;

/**
 * Viewport의 카메라·ViewMode 정책을 지속하고 프레임별 렌더 요청을 만든다.
 * Scene은 소유하지 않으며 Draw가 호출되는 동안 유효한 Render Scene을 참조한다.
 */
class FViewportClient
{
public:
    virtual ~FViewportClient() = default;

    /** 현재 정책으로 ViewFamily와 SceneView를 만들어 Renderer에 즉시 제출한다. */
    virtual void Draw(FViewport& Viewport, FRenderer& Renderer);

    void SetScene(const FScene* InScene) noexcept;
    [[nodiscard]] const FScene* GetScene() const noexcept;

    void SetViewMode(EViewMode InViewMode) noexcept;
    [[nodiscard]] EViewMode GetViewMode() const noexcept;

    void SetCameraPosition(const FVector& InCameraPosition) noexcept;
    [[nodiscard]] const FVector& GetCameraPosition() const noexcept;

protected:
    [[nodiscard]] virtual FSceneView BuildSceneView(const FViewport& Viewport) const;

private:
    const FScene* Scene = nullptr;
    EViewMode ViewMode = EViewMode::Lit;
    FVector CameraPosition{};
    float VerticalFieldOfViewDegrees = 60.0f;
    float NearPlane = 1.0f;
    float FarPlane = 100000.0f;
};
