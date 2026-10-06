#pragma once

#include "Editor/UI/ViewportInputRegion.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

class FInputFrame;
class FRenderer;
class FScene;
class FViewport;
class FViewportPanel;
class FWorldOutlinerPanel;
class UWorld;

/**
 * Editor 전용 ImGui Context와 DX12 Backend를 소유하고 Panel 구성 결과를 Engine에 제공한다.
 * UI 명령 생성은 입력 라우팅 전에 수행하고 실제 GPU 명령 기록은 Scene 렌더 뒤에 수행한다.
 */
class FEditorUI
{
public:
    FEditorUI();
    ~FEditorUI();

    FEditorUI(const FEditorUI&) = delete;
    FEditorUI& operator=(const FEditorUI&) = delete;

    bool Initialize(
        FRenderer& Renderer,
        const FScene* Scene,
        UWorld* World,
        std::uint32_t DisplayWidth,
        std::uint32_t DisplayHeight);
    void Shutdown();

    /** 입력을 ImGui에 전달하고 모든 Panel을 구성해 이번 프레임 Region과 DrawData를 확정한다. */
    void BuildFrame(const FInputFrame& InputFrame, float DeltaSeconds);

    /** 표시 중인 Viewport Target을 sampling 상태로 바꾸고 Main Target에 ImGui DrawData를 기록한다. */
    bool Render(FRenderer& Renderer);

    [[nodiscard]] std::span<const FViewportInputRegion> GetViewportInputRegions() const noexcept;
    [[nodiscard]] std::span<FViewport* const> GetVisibleViewports() const noexcept;
    [[nodiscard]] FViewport* GetActiveViewport() const noexcept;
    [[nodiscard]] bool WantsKeyboardCapture() const noexcept;

private:
    void FeedInput(const FInputFrame& InputFrame);
    void BuildMainMenu();
    void BuildDockSpace();

    FRenderer* Renderer = nullptr;
    UWorld* World = nullptr;
    std::unique_ptr<FViewportPanel> ViewportPanel;
    std::unique_ptr<FWorldOutlinerPanel> WorldOutlinerPanel;
    std::vector<FViewportInputRegion> ViewportInputRegions;
    bool bWantsKeyboardCapture = false;
    bool bLastWindowFocused = false;
    bool bInitialized = false;
    bool bFrameBuilt = false;
};
