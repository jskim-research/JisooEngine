#include "Runtime/Engine/GameEngine.h"

#include "Runtime/Engine/Viewport/GameViewportClient.h"
#include "Runtime/Engine/Viewport/Viewport.h"
#include "Runtime/Engine/World.h"
#include "Runtime/Render/Renderer.h"

FGameEngine::FGameEngine() = default;

FGameEngine::~FGameEngine()
{
    Shutdown();
}

bool FGameEngine::Initialize(const FEngineInitParams& InitParams)
{
    if (Viewport != nullptr)
    {
        return true;
    }

    if (!FEngine::Initialize(InitParams))
    {
        return false;
    }

    const std::vector<UWorld*> ActiveWorlds = GetWorlds();
    if (ActiveWorlds.empty())
    {
        FEngine::Shutdown();
        return false;
    }

    FRenderer* FrameRenderer = GetRenderer();
    if (FrameRenderer == nullptr)
    {
        FEngine::Shutdown();
        return false;
    }

    Viewport = std::make_unique<FViewport>(
        FrameRenderer->GetMainRenderTargetHandle(),
        InitParams.ClientWidth,
        InitParams.ClientHeight);
    ViewportClient = std::make_unique<FGameViewportClient>();
    ViewportClient->SetScene(ActiveWorlds.front()->GetScene());
    Viewport->SetClient(ViewportClient.get());
    return true;
}

void FGameEngine::Shutdown()
{
    Viewport.reset();
    ViewportClient.reset();
    FEngine::Shutdown();
}

FInputRouteContext FGameEngine::BuildInputRouteContext(const FInputFrame&, float)
{
    return {ViewportClient.get(), ViewportClient.get()};
}

void FGameEngine::RenderFrame()
{
    FRenderer* FrameRenderer = GetRenderer();
    if (FrameRenderer == nullptr || Viewport == nullptr || !FrameRenderer->BeginFrame())
    {
        return;
    }

    Viewport->Draw(*FrameRenderer);
    FrameRenderer->EndFrame();
}
