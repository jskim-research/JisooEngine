#pragma once

#include "Runtime/Engine/Viewport/ViewportClient.h"

/** Editor 비행 카메라의 이동·회전 입력 정책을 보관하는 ViewportClient다. */
class FEditorViewportClient : public FViewportClient
{
public:
    /** 우클릭 회전과 WASD·QE 이동을 현재 카메라 Transform에 반영한다. */
    void ProcessInput(const FInputFrame& InputFrame, float DeltaSeconds) override;

private:
    float MovementSpeedCentimetersPerSecond = 500.0f;
    float LookSensitivityDegreesPerPixel = 0.15f;
};
