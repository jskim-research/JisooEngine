#include "Runtime/Engine/Engine.h"

#include "Runtime/Render/Renderer.h"

FEngine::FEngine() = default;

FEngine::~FEngine()
{
    Shutdown();
}

bool FEngine::Initialize(const FEngineInitParams& InitParams)
{
    if (Renderer != nullptr)
    {
        return true;
    }

    std::unique_ptr<FRenderer> NewRenderer = std::make_unique<FRenderer>();
    if (!NewRenderer->Initialize(
            InitParams.NativeWindowHandle,
            InitParams.ClientWidth,
            InitParams.ClientHeight))
    {
        return false;
    }

    Renderer = std::move(NewRenderer);
    return true;
}

void FEngine::Tick()
{
    if (Renderer != nullptr)
    {
        Renderer->RenderFrame();
    }
}

void FEngine::Shutdown()
{
    if (Renderer != nullptr)
    {
        Renderer->Shutdown();
        Renderer.reset();
    }
}
