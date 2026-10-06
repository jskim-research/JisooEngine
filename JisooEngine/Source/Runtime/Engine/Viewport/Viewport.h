#pragma once

#include "Runtime/Engine/Viewport/SceneView.h"

#include <cstdint>

class FRenderer;
class FViewportClient;

/**
 * Renderer가 발급한 출력 Target Handle, 논리적 크기와 ViewportClient 연결을 보관한다.
 * Draw는 렌더 정책을 구현하지 않고 연결된 Client의 Draw 진입점만 호출한다.
 */
class FViewport
{
public:
    FViewport(
        FRenderTargetHandle Target,
        std::uint32_t Width,
        std::uint32_t Height);

    void SetClient(FViewportClient* InClient) noexcept;
    [[nodiscard]] FViewportClient* GetClient() const noexcept;

    /** 연결된 Client가 이 출력 표면에 대한 렌더 요청을 제출하게 한다. */
    void Draw(FRenderer& Renderer);

    /** pixel 크기를 갱신하며 0 크기는 렌더 불가능한 상태로 유지한다. */
    void Resize(std::uint32_t Width, std::uint32_t Height) noexcept;

    [[nodiscard]] const FViewportOutput& GetOutput() const noexcept;
    [[nodiscard]] bool IsRenderable() const noexcept;

private:
    FViewportClient* Client = nullptr;
    FViewportOutput Output{};
};
