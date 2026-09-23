#pragma once

#include <cstdint>
#include <memory>

class FRenderer;

struct FEngineInitParams
{
    void* NativeWindowHandle = nullptr;
    std::uint32_t ClientWidth = 0;
    std::uint32_t ClientHeight = 0;
};

class FEngine
{
public:
    FEngine();
    virtual ~FEngine();

    virtual bool Initialize(const FEngineInitParams& InitParams);
    virtual void Tick();
    virtual void Shutdown();

private:
    std::unique_ptr<FRenderer> Renderer;
};
