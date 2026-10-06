#pragma once

#include "Runtime/Input/InputRouter.h"

#include <span>

class FInputFrame;
class FViewport;
struct FViewportInputRegion;

struct FEditorViewportInputRoutingResult
{
    FInputRouteContext Context;
    FViewport* ActivatedViewport = nullptr;
};

/**
 * Editor Viewport의 Active·Hover·Pointer Capture 상태를 입력 대상 결정으로 변환한다.
 * Region과 Viewport는 호출 동안 유효해야 하며 Capture는 Pointer Button을 놓는 프레임까지 유지한다.
 */
class FEditorViewportInputRouting
{
public:
    [[nodiscard]] FEditorViewportInputRoutingResult Route(
        const FInputFrame& InputFrame,
        std::span<const FViewportInputRegion> Regions,
        FViewport* ActiveViewport,
        bool bWantsKeyboardCapture);

    void Reset() noexcept;

private:
    FViewport* CapturedPointerViewport = nullptr;
};
