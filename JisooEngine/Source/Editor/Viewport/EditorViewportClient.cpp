#include "Editor/Viewport/EditorViewportClient.h"

#include <algorithm>
#include <cmath>

void FEditorViewportClient::ProcessInput(const FInputFrame& InputFrame, float DeltaSeconds)
{
    if (!InputFrame.IsWindowFocused())
    {
        return;
    }

    if (InputFrame.IsDown(EInputKey::MouseRight))
    {
        constexpr float MaxPitchDegrees = 89.0f;
        const float PitchDegrees = (std::clamp)(
            GetCameraPitchDegrees() +
                static_cast<float>(InputFrame.GetPointerDeltaY()) * LookSensitivityDegreesPerPixel,
            -MaxPitchDegrees,
            MaxPitchDegrees);
        const float YawDegrees = std::remainder(
            GetCameraYawDegrees() +
                static_cast<float>(InputFrame.GetPointerDeltaX()) * LookSensitivityDegreesPerPixel,
            360.0f);
        SetCameraRotationDegrees(PitchDegrees, YawDegrees);
    }

    FVector MovementDirection{};
    const FVector Forward = GetCameraForwardVector();
    const FVector Right = GetCameraRightVector();
    if (InputFrame.IsDown(EInputKey::W))
    {
        MovementDirection = MovementDirection + Forward;
    }
    if (InputFrame.IsDown(EInputKey::S))
    {
        MovementDirection = MovementDirection - Forward;
    }
    if (InputFrame.IsDown(EInputKey::D))
    {
        MovementDirection = MovementDirection + Right;
    }
    if (InputFrame.IsDown(EInputKey::A))
    {
        MovementDirection = MovementDirection - Right;
    }
    if (InputFrame.IsDown(EInputKey::E))
    {
        MovementDirection.Z += 1.0f;
    }
    if (InputFrame.IsDown(EInputKey::Q))
    {
        MovementDirection.Z -= 1.0f;
    }

    const float LengthSquared = MovementDirection.X * MovementDirection.X +
        MovementDirection.Y * MovementDirection.Y +
        MovementDirection.Z * MovementDirection.Z;
    if (LengthSquared <= 0.0f || DeltaSeconds <= 0.0f)
    {
        return;
    }

    const float Distance = MovementSpeedCentimetersPerSecond * DeltaSeconds;
    const float Scale = Distance / std::sqrt(LengthSquared);
    SetCameraPosition(GetCameraPosition() + MovementDirection * Scale);
}
