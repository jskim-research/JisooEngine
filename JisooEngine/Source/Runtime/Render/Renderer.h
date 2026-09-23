#pragma once

#include "Runtime/Render/D3D12/D3D12CommandContext.h"
#include "Runtime/Render/D3D12/D3D12Device.h"
#include "Runtime/Render/D3D12/DXGISwapChain.h"
#include "Runtime/Render/D3D12/FrameResource.h"

#include <array>
#include <cstdint>

class FRenderer
{
public:
    FRenderer() = default;
    ~FRenderer();

    FRenderer(const FRenderer&) = delete;
    FRenderer& operator=(const FRenderer&) = delete;

    bool Initialize(
        void* NativeWindowHandle,
        std::uint32_t Width,
        std::uint32_t Height);
    void RenderFrame();
    bool Resize(std::uint32_t Width, std::uint32_t Height);
    void Shutdown();

    FD3D12Device Device;
    FD3D12CommandContext CommandContext;
    FDXGISwapChain SwapChain;
    std::array<FFrameResource, FDXGISwapChain::BufferCount> FrameResources;
    bool bInitialized = false;
};
