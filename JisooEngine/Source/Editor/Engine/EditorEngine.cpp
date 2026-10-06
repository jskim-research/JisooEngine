#include "Editor/Engine/EditorEngine.h"

#include "Editor/Viewport/EditorViewportLayout.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Engine/World.h"
#include "Runtime/Render/Renderer.h"

FEditorEngine::FEditorEngine() = default;

FEditorEngine::~FEditorEngine()
{
    Shutdown();
}

bool FEditorEngine::Initialize(const FEngineInitParams& InitParams)
{
    if (ViewportLayout != nullptr)
    {
        return true;
    }

    if (!FEngine::Initialize(InitParams))
    {
        return false;
    }

    const std::vector<UWorld*> ActiveWorlds = GetWorlds();
    FRenderer* FrameRenderer = GetRenderer();
    auto NewLayout = std::make_unique<FEditorViewportLayout>();
    if (ActiveWorlds.empty() || FrameRenderer == nullptr || !NewLayout->Initialize(
            ActiveWorlds.front()->GetScene(),
            FrameRenderer->GetMainRenderTargetHandle(),
            InitParams.ClientWidth,
            InitParams.ClientHeight))
    {
        FEngine::Shutdown();
        return false;
    }

    ViewportLayout = std::move(NewLayout);
    return true;
}

void FEditorEngine::Shutdown()
{
    ViewportLayout.reset();
    FEngine::Shutdown();
}

void FEditorEngine::RenderFrame()
{
    FRenderer* FrameRenderer = GetRenderer();
    if (FrameRenderer == nullptr || ViewportLayout == nullptr || !FrameRenderer->BeginFrame())
    {
        return;
    }

    for (FViewport* Viewport : ViewportLayout->GetVisibleViewports())
    {
        Viewport->Draw(*FrameRenderer);
    }
    FrameRenderer->EndFrame();
}
