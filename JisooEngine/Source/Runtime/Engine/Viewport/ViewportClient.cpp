#include "Runtime/Engine/Viewport/ViewportClient.h"

#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Render/Renderer.h"

#include <cmath>
#include <numbers>

void FViewportClient::ProcessInput(const FInputFrame&, float)
{
}

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

void FViewportClient::SetCameraRotationDegrees(float InPitchDegrees, float InYawDegrees) noexcept
{
    CameraPitchDegrees = InPitchDegrees;
    CameraYawDegrees = InYawDegrees;
}

float FViewportClient::GetCameraPitchDegrees() const noexcept
{
    return CameraPitchDegrees;
}

float FViewportClient::GetCameraYawDegrees() const noexcept
{
    return CameraYawDegrees;
}

FVector FViewportClient::GetCameraForwardVector() const noexcept
{
    const float PitchRadians = CameraPitchDegrees * std::numbers::pi_v<float> / 180.0f;
    const float YawRadians = CameraYawDegrees * std::numbers::pi_v<float> / 180.0f;
    const float CosPitch = std::cos(PitchRadians);
    return {
        CosPitch * std::cos(YawRadians),
        CosPitch * std::sin(YawRadians),
        -std::sin(PitchRadians)};
}

FVector FViewportClient::GetCameraRightVector() const noexcept
{
    const float YawRadians = CameraYawDegrees * std::numbers::pi_v<float> / 180.0f;
    return {-std::sin(YawRadians), std::cos(YawRadians), 0.0f};
}

FVector FViewportClient::GetCameraUpVector() const noexcept
{
    const float PitchRadians = CameraPitchDegrees * std::numbers::pi_v<float> / 180.0f;
    const float YawRadians = CameraYawDegrees * std::numbers::pi_v<float> / 180.0f;
    const float SinPitch = std::sin(PitchRadians);
    return {
        SinPitch * std::cos(YawRadians),
        SinPitch * std::sin(YawRadians),
        std::cos(PitchRadians)};
}

FSceneView FViewportClient::BuildSceneView(const FViewport& Viewport) const
{
    const FViewportOutput& Output = Viewport.GetOutput();
    const FVector Forward = GetCameraForwardVector();
    const FVector Right = GetCameraRightVector();
    const FVector Up = GetCameraUpVector();

    FMatrix ViewMatrix = FMatrix::Identity();
    ViewMatrix.M[0][0] = Right.X;
    ViewMatrix.M[1][0] = Right.Y;
    ViewMatrix.M[2][0] = Right.Z;
    ViewMatrix.M[0][1] = Up.X;
    ViewMatrix.M[1][1] = Up.Y;
    ViewMatrix.M[2][1] = Up.Z;
    ViewMatrix.M[0][2] = Forward.X;
    ViewMatrix.M[1][2] = Forward.Y;
    ViewMatrix.M[2][2] = Forward.Z;
    ViewMatrix.M[3][0] = -(CameraPosition.X * Right.X + CameraPosition.Y * Right.Y + CameraPosition.Z * Right.Z);
    ViewMatrix.M[3][1] = -(CameraPosition.X * Up.X + CameraPosition.Y * Up.Y + CameraPosition.Z * Up.Z);
    ViewMatrix.M[3][2] = -(CameraPosition.X * Forward.X + CameraPosition.Y * Forward.Y + CameraPosition.Z * Forward.Z);

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
    View.ViewMatrix = ViewMatrix;
    View.ProjectionMatrix = Projection;
    View.ViewRect = {0, 0, Output.Width, Output.Height};
    View.CameraPosition = CameraPosition;
    return View;
}
