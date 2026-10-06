#include "Runtime/Engine/Viewport/ViewportClient.h"

#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Render/Renderer.h"

#include <cmath>
#include <numbers>

void FViewportClient::Draw(FViewport& Viewport, FRenderer& Renderer)
{
    if (Scene == nullptr || !Viewport.IsRenderable())
    {
        return;
    }

    FSceneViewFamily ViewFamily;
    ViewFamily.Scene = Scene;
    ViewFamily.Output = Viewport.GetOutput();
    ViewFamily.ViewMode = ViewMode;
    ViewFamily.Views.push_back(BuildSceneView(Viewport));
    Renderer.RenderViewFamily(ViewFamily);
}

void FViewportClient::SetScene(const FScene* InScene) noexcept
{
    Scene = InScene;
}

const FScene* FViewportClient::GetScene() const noexcept
{
    return Scene;
}

void FViewportClient::SetViewMode(EViewMode InViewMode) noexcept
{
    ViewMode = InViewMode;
}

EViewMode FViewportClient::GetViewMode() const noexcept
{
    return ViewMode;
}

void FViewportClient::SetCameraPosition(const FVector& InCameraPosition) noexcept
{
    CameraPosition = InCameraPosition;
}

const FVector& FViewportClient::GetCameraPosition() const noexcept
{
    return CameraPosition;
}

FSceneView FViewportClient::BuildSceneView(const FViewport& Viewport) const
{
    const FViewportOutput& Output = Viewport.GetOutput();

    FMatrix CameraTranslation = FMatrix::Identity();
    CameraTranslation.M[3][0] = -CameraPosition.X;
    CameraTranslation.M[3][1] = -CameraPosition.Y;
    CameraTranslation.M[3][2] = -CameraPosition.Z;

    // +X Forward, +Y Right, +Z Up 월드 축을 D3D View의 +Z Forward, +X Right, +Y Up으로 옮긴다.
    FMatrix AxisMapping{};
    AxisMapping.M[0][2] = 1.0f;
    AxisMapping.M[1][0] = 1.0f;
    AxisMapping.M[2][1] = 1.0f;
    AxisMapping.M[3][3] = 1.0f;

    const float AspectRatio =
        static_cast<float>(Output.Width) / static_cast<float>(Output.Height);
    const float FieldOfViewRadians =
        VerticalFieldOfViewDegrees * std::numbers::pi_v<float> / 180.0f;
    const float YScale = 1.0f / std::tan(FieldOfViewRadians * 0.5f);
    const float XScale = YScale / AspectRatio;

    FMatrix Projection{};
    Projection.M[0][0] = XScale;
    Projection.M[1][1] = YScale;
    Projection.M[2][2] = FarPlane / (FarPlane - NearPlane);
    Projection.M[2][3] = 1.0f;
    Projection.M[3][2] = -NearPlane * FarPlane / (FarPlane - NearPlane);

    FSceneView View;
    View.ViewMatrix = CameraTranslation * AxisMapping;
    View.ProjectionMatrix = Projection;
    View.ViewRect = {0, 0, Output.Width, Output.Height};
    View.CameraPosition = CameraPosition;
    return View;
}
