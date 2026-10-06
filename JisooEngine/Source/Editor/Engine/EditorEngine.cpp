#include "Editor/Engine/EditorEngine.h"

#include "Editor/UI/EditorUI.h"
#include "Editor/UI/ViewportInputRegion.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Engine/Viewport/ViewportClient.h"
#include "Runtime/Engine/World.h"
#include "Runtime/Input/InputTypes.h"
#include "Runtime/Render/Renderer.h"

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

FEditorEngine::FEditorEngine() = default;

FEditorEngine::~FEditorEngine()
{
    Shutdown();
}

bool FEditorEngine::Initialize(const FEngineInitParams& InitParams)
{
    if (EditorUI != nullptr)
    {
        return true;
    }

    if (!FEngine::Initialize(InitParams))
    {
        return false;
    }

    const std::vector<UWorld*> ActiveWorlds = GetWorlds();
    FRenderer* FrameRenderer = GetRenderer();
    auto NewEditorUI = std::make_unique<FEditorUI>();
    if (ActiveWorlds.empty() || FrameRenderer == nullptr || !NewEditorUI->Initialize(
            *FrameRenderer,
            ActiveWorlds.front()->GetScene(),
            ActiveWorlds.front(),
            InitParams.ClientWidth,
            InitParams.ClientHeight))
    {
        FEngine::Shutdown();
        return false;
    }

    EditorUI = std::move(NewEditorUI);
    return true;
}

void FEditorEngine::Shutdown()
{
    CapturedPointerViewport = nullptr;
    EditorUI.reset();
    FEngine::Shutdown();
}

FInputRouteContext FEditorEngine::BuildInputRouteContext(
    const FInputFrame& InputFrame,
    float DeltaSeconds)
{
    FInputRouteContext RouteContext;
    if (EditorUI == nullptr)
    {
        CapturedPointerViewport = nullptr;
        return RouteContext;
    }

    EditorUI->BuildFrame(InputFrame, DeltaSeconds);
    const std::span<const FViewportInputRegion> Regions =
        EditorUI->GetViewportInputRegions();

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

    if (PointerRegion != nullptr)
    {
        RouteContext.PointerTarget = PointerRegion->Viewport->GetClient();
        RouteContext.PointerOriginX = static_cast<std::int32_t>(PointerRegion->MinX);
        RouteContext.PointerOriginY = static_cast<std::int32_t>(PointerRegion->MinY);
        const float RegionWidth = PointerRegion->MaxX - PointerRegion->MinX;
        const float RegionHeight = PointerRegion->MaxY - PointerRegion->MinY;
        const FViewportOutput& Output = PointerRegion->Viewport->GetOutput();
        if (RegionWidth > 0.0f && RegionHeight > 0.0f)
        {
            RouteContext.PointerScaleX =
                static_cast<float>(Output.Width) / RegionWidth;
            RouteContext.PointerScaleY =
                static_cast<float>(Output.Height) / RegionHeight;
            RouteContext.bTransformPointerToTarget = true;
        }
    }
    if (FViewport* ActiveViewport = EditorUI->GetActiveViewport();
        ActiveViewport != nullptr && !EditorUI->WantsKeyboardCapture())
    {
        RouteContext.KeyboardTarget = ActiveViewport->GetClient();
    }

    // Button을 놓은 프레임까지 Capture 대상에 전달한 뒤 다음 프레임부터 Hover 판정으로 돌아간다.
    if (CapturedPointerViewport != nullptr && !IsAnyPointerButtonDown(InputFrame))
    {
        CapturedPointerViewport = nullptr;
    }
    return RouteContext;
}

void FEditorEngine::RenderFrame()
{
    FRenderer* FrameRenderer = GetRenderer();
    if (FrameRenderer == nullptr || EditorUI == nullptr || !FrameRenderer->BeginFrame())
    {
        return;
    }

    for (FViewport* Viewport : EditorUI->GetVisibleViewports())
    {
        Viewport->Draw(*FrameRenderer);
    }
    EditorUI->Render(*FrameRenderer);
    FrameRenderer->EndFrame();
}
