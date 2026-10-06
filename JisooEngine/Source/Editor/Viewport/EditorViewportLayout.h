#pragma once

#include "Runtime/Render/Target/RenderTargetHandle.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class FEditorViewportClient;
class FScene;
class FViewport;

enum class EEditorViewportLayoutMode : std::uint8_t
{
    Single,
    TwoVertical,
    TwoHorizontal,
    Four
};

/** 하나의 안정된 Layout Slot에 대응하는 Viewport와 Client 수명을 함께 소유한다. */
struct FEditorViewportSlot
{
    std::unique_ptr<FEditorViewportClient> Client;
    std::unique_ptr<FViewport> Viewport;
};

/**
 * 최대 네 개 Editor Viewport Slot의 수명, 배치 모드와 표시 여부를 관리한다.
 * Renderer를 참조하지 않으며 호출자는 GetVisibleViewports 결과를 현재 프레임 안에서만 사용해야 한다.
 */
class FEditorViewportLayout
{
public:
    static constexpr std::size_t MaxViewportCount = 4;

    FEditorViewportLayout();
    ~FEditorViewportLayout();

    FEditorViewportLayout(const FEditorViewportLayout&) = delete;
    FEditorViewportLayout& operator=(const FEditorViewportLayout&) = delete;

    /** Single 모드의 Slot 0을 생성하고 Scene과 Renderer 소유 출력 Target을 연결한다. */
    bool Initialize(
        const FScene* Scene,
        FRenderTargetHandle Target,
        std::uint32_t Width,
        std::uint32_t Height);
    void Shutdown();

    [[nodiscard]] std::vector<FViewport*> GetVisibleViewports() const;
    [[nodiscard]] std::size_t GetCreatedSlotCount() const noexcept;
    [[nodiscard]] EEditorViewportLayoutMode GetLayoutMode() const noexcept;

private:
    std::array<std::unique_ptr<FEditorViewportSlot>, MaxViewportCount> Slots;
    EEditorViewportLayoutMode LayoutMode = EEditorViewportLayoutMode::Single;
};
