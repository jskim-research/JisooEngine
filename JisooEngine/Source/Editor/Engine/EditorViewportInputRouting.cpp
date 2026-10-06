#include "Editor/Engine/EditorViewportInputRouting.h"

#include "Editor/UI/ViewportInputRegion.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Engine/Viewport/ViewportClient.h"
#include "Runtime/Input/InputTypes.h"

#include <algorithm>

namespace
{
bool IsAnyPointerButtonDown(const FInputFrame& InputFrame)
{
    return InputFrame.IsDown(EInputKey::MouseLeft) ||
        InputFrame.IsDown(EInputKey::MouseRight) ||
        InputFrame.IsDown(EInputKey::MouseMiddle) ||
        InputFrame.IsDown(EInputKey::MouseX1) ||
        InputFrame.IsDown(EInputKey::MouseX2);
}

bool WasAnyPointerButtonPressed(const FInputFrame& InputFrame)
{
    return InputFrame.WasPressed(EInputKey::MouseLeft) ||
        InputFrame.WasPressed(EInputKey::MouseRight) ||
        InputFrame.WasPressed(EInputKey::MouseMiddle) ||
        InputFrame.WasPressed(EInputKey::MouseX1) ||
        InputFrame.WasPressed(EInputKey::MouseX2);
}
}

FEditorViewportInputRoutingResult FEditorViewportInputRouting::Route(
    const FInputFrame& InputFrame,
    std::span<const FViewportInputRegion> Regions,
    FViewport* ActiveViewport,
    bool bWantsKeyboardCapture)
{
    FEditorViewportInputRoutingResult Result;

    const auto CapturedRegion = std::ranges::find_if(
        Regions,
        [this](const FViewportInputRegion& Region)
        {
            return Region.Viewport == CapturedPointerViewport;
        });
    if (CapturedPointerViewport != nullptr && CapturedRegion == Regions.end())
    {
        CapturedPointerViewport = nullptr;
    }

    const auto HoveredRegion = std::ranges::find_if(
        Regions,
        [](const FViewportInputRegion& Region)
        {
            return Region.bHovered;
        });
    if (CapturedPointerViewport == nullptr && HoveredRegion != Regions.end() &&
        WasAnyPointerButtonPressed(InputFrame))
    {
        CapturedPointerViewport = HoveredRegion->Viewport;
        Result.ActivatedViewport = HoveredRegion->Viewport;
        ActiveViewport = HoveredRegion->Viewport;
    }

    const FViewportInputRegion* PointerRegion = nullptr;
    if (CapturedPointerViewport != nullptr)
    {
        const auto Region = std::ranges::find_if(
            Regions,
            [this](const FViewportInputRegion& Candidate)
            {
                return Candidate.Viewport == CapturedPointerViewport;
            });
        if (Region != Regions.end())
        {
            PointerRegion = &*Region;
        }
    }
    if (PointerRegion == nullptr && HoveredRegion != Regions.end())
    {
        PointerRegion = &*HoveredRegion;
    }

    if (PointerRegion != nullptr && PointerRegion->Viewport != nullptr)
    {
        Result.Context.PointerTarget = PointerRegion->Viewport->GetClient();
        Result.Context.PointerOriginX = static_cast<std::int32_t>(PointerRegion->MinX);
        Result.Context.PointerOriginY = static_cast<std::int32_t>(PointerRegion->MinY);
        const float RegionWidth = PointerRegion->MaxX - PointerRegion->MinX;
        const float RegionHeight = PointerRegion->MaxY - PointerRegion->MinY;
        const FViewportOutput& Output = PointerRegion->Viewport->GetOutput();
        if (RegionWidth > 0.0f && RegionHeight > 0.0f)
        {
            Result.Context.PointerScaleX = static_cast<float>(Output.Width) / RegionWidth;
            Result.Context.PointerScaleY = static_cast<float>(Output.Height) / RegionHeight;
            Result.Context.bTransformPointerToTarget = true;
        }
    }

    if (ActiveViewport != nullptr && !bWantsKeyboardCapture)
    {
        Result.Context.KeyboardTarget = ActiveViewport->GetClient();
    }

    // Button을 놓은 프레임까지 Capture 대상에 전달한 뒤 다음 프레임부터 Hover 판정으로 돌아간다.
    if (CapturedPointerViewport != nullptr && !IsAnyPointerButtonDown(InputFrame))
    {
        CapturedPointerViewport = nullptr;
    }
    return Result;
}

void FEditorViewportInputRouting::Reset() noexcept
{
    CapturedPointerViewport = nullptr;
}
