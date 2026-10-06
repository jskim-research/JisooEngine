#pragma once

#include "Runtime/Core/Math/MathTypes.h"
#include "Runtime/Engine/Viewport/SceneView.h"
#include "Runtime/Input/InputInterfaces.h"

class FRenderer;
class FScene;
class FViewport;

/**
 * Viewport의 카메라·ViewMode 정책을 지속하고 프레임별 렌더 요청을 만든다.
 * Scene은 소유하지 않으며 Draw가 호출되는 동안 유효한 Render Scene을 참조한다.
 */
class FViewportClient : public IInputReceiver
{
public:
    virtual ~FViewportClient() = default;

    /** 기본 Client는 입력을 해석하지 않으며 파생 Client가 필요한 정책만 구현한다. */
    void ProcessInput(const FInputFrame& InputFrame, float DeltaSeconds) override;

    /** 현재 정책으로 ViewFamily와 SceneView를 만들어 Renderer에 즉시 제출한다. */
    virtual void Draw(FViewport& Viewport, FRenderer& Renderer);

    void SetScene(const FScene* InScene) noexcept;
    [[nodiscard]] const FScene* GetScene() const noexcept;

    void SetViewMode(EViewMode InViewMode) noexcept;
    [[nodiscard]] EViewMode GetViewMode() const noexcept;

    void SetCameraPosition(const FVector& InCameraPosition) noexcept;
    [[nodiscard]] const FVector& GetCameraPosition() const noexcept;

    /** Pitch와 Yaw를 degree 단위로 설정하며 범위 제한은 구체 Client 정책이 결정한다. */
    void SetCameraRotationDegrees(float InPitchDegrees, float InYawDegrees) noexcept;
    [[nodiscard]] float GetCameraPitchDegrees() const noexcept;
    [[nodiscard]] float GetCameraYawDegrees() const noexcept;

protected:
    [[nodiscard]] virtual FSceneView BuildSceneView(const FViewport& Viewport) const;

    [[nodiscard]] FVector GetCameraForwardVector() const noexcept;
    [[nodiscard]] FVector GetCameraRightVector() const noexcept;
    [[nodiscard]] FVector GetCameraUpVector() const noexcept;

private:
    const FScene* Scene = nullptr;
    EViewMode ViewMode = EViewMode::Lit;
    FVector CameraPosition{};
    float CameraPitchDegrees = 0.0f;
    float CameraYawDegrees = 0.0f;
    float VerticalFieldOfViewDegrees = 60.0f;
    float NearPlane = 1.0f;
    float FarPlane = 100000.0f;
};
