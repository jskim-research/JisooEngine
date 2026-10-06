#pragma once

#include "Runtime/Core/Math/MathTypes.h"
#include "Runtime/Render/Target/RenderTargetHandle.h"

#include <cstdint>
#include <vector>

class FScene;

enum class EViewMode : std::uint8_t
{
    Lit,
    Wireframe,
    Depth,
    WorldNormal
};

/** Renderer가 소유한 출력 자원을 프레임 요청에서 식별하는 값 스냅숏이다. */
struct FViewportOutput
{
    FRenderTargetHandle Target;
    std::uint32_t Width = 0;
    std::uint32_t Height = 0;
};

/** 하나의 SceneView가 RenderTarget에서 차지할 pixel 영역이다. */
struct FViewRect
{
    std::uint32_t X = 0;
    std::uint32_t Y = 0;
    std::uint32_t Width = 0;
    std::uint32_t Height = 0;
};

/** 한 프레임에 사용할 카메라 변환과 출력 영역을 값으로 보관한다. */
struct FSceneView
{
    FMatrix ViewMatrix = FMatrix::Identity();
    FMatrix ProjectionMatrix = FMatrix::Identity();
    FViewRect ViewRect{};
    FVector CameraPosition{};

    [[nodiscard]] FMatrix GetViewProjectionMatrix() const
    {
        return ViewMatrix * ProjectionMatrix;
    }
};

/**
 * 동일한 Scene, 출력 대상과 ViewMode를 공유하는 프레임 로컬 SceneView 묶음이다.
 * Renderer 호출이 끝날 때까지 Scene이 유효해야 하며 현재 Single Thread에서 즉시 소비된다.
 */
struct FSceneViewFamily
{
    const FScene* Scene = nullptr;
    FViewportOutput Output{};
    EViewMode ViewMode = EViewMode::Lit;
    std::vector<FSceneView> Views;
};
