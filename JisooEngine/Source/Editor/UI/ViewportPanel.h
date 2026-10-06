#pragma once

#include "Editor/UI/ViewportInputRegion.h"
#include "Runtime/Render/Target/RenderTargetHandle.h"

#include <cstdint>
#include <memory>
#include <vector>

class FEditorViewportLayout;
class FRenderer;
class FScene;
class FViewport;

/**
 * Editor Viewport Window와 그 안의 Layout 수명을 소유한다.
 * 닫히면 Layout과 출력 Target을 해제하며 다시 열 때 기본 상태로 새로 생성한다.
 */
class FViewportPanel
{
public:
    FViewportPanel();
    ~FViewportPanel();

    FViewportPanel(const FViewportPanel&) = delete;
    FViewportPanel& operator=(const FViewportPanel&) = delete;

    bool Initialize(FRenderer& Renderer, const FScene* Scene);
    void Shutdown();

    /** 현재 ImGui 배치 결과를 OutRegions에 추가하고 이번 프레임 표시할 Viewport를 확정한다. */
    void Draw(std::vector<FViewportInputRegion>& OutRegions);

    void SetOpen(bool bInOpen);
    [[nodiscard]] bool IsOpen() const noexcept;
    [[nodiscard]] FViewport* GetActiveViewport() const noexcept;
    [[nodiscard]] const std::vector<FViewport*>& GetVisibleViewports() const noexcept;

private:
    bool EnsureLayout(std::uint32_t Width, std::uint32_t Height);
    void ReleaseLayout();

    FRenderer* Renderer = nullptr;
    const FScene* Scene = nullptr;
    std::unique_ptr<FEditorViewportLayout> Layout;
    std::vector<FViewport*> VisibleViewports;
    FRenderTargetHandle RenderTarget;
    bool bOpen = true;
};
