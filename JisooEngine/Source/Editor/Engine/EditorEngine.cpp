#include "Editor/Engine/EditorEngine.h"

#include "Editor/UI/EditorUI.h"
#include "Editor/UI/ViewportInputRegion.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Engine/Viewport/ViewportClient.h"
#include "Runtime/Engine/World.h"
#include "Runtime/Input/InputTypes.h"
#include "Runtime/Render/Renderer.h"

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
    ViewportInputRouting.Reset();
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
        ViewportInputRouting.Reset();
        return RouteContext;
    }

    EditorUI->BuildFrame(InputFrame, DeltaSeconds);
    const std::span<const FViewportInputRegion> Regions =
        EditorUI->GetViewportInputRegions();

    FEditorViewportInputRoutingResult Result = ViewportInputRouting.Route(
        InputFrame,
        Regions,
        EditorUI->GetActiveViewport(),
        EditorUI->WantsKeyboardCapture());
    if (Result.ActivatedViewport != nullptr)
    {
        EditorUI->SetActiveViewport(Result.ActivatedViewport);
    }
    return Result.Context;
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
    FrameRenderer->EndFrame(&FEditorUI::RecordFinalOverlay, EditorUI.get());
}

void FEditorEngine::OnWindowResized(std::uint32_t Width, std::uint32_t Height)
{
    if (EditorUI != nullptr)
    {
        EditorUI->SetDisplaySize(Width, Height);
    }
}
