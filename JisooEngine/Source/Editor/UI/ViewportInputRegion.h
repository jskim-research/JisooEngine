#pragma once

class FViewport;

/** ImGui가 배치한 Viewport Image와 엔진 Viewport를 한 프레임 동안 연결하는 입력 영역이다. */
struct FViewportInputRegion
{
    FViewport* Viewport = nullptr;
    float MinX = 0.0f;
    float MinY = 0.0f;
    float MaxX = 0.0f;
    float MaxY = 0.0f;
    bool bHovered = false;
};
