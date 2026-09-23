#include "Runtime/Render/Renderer.h"

FRenderer::~FRenderer()
{
    Shutdown();
}

bool FRenderer::Initialize(
    void* NativeWindowHandle,
    std::uint32_t Width,
    std::uint32_t Height)
{
    if (bInitialized)
    {
        return true;
    }

    if (NativeWindowHandle == nullptr || Width == 0 || Height == 0 ||
        !Device.Initialize())
    {
        Shutdown();
        return false;
    }

    for (FFrameResource& FrameResource : FrameResources)
    {
        if (!FrameResource.Initialize(Device.GetDevice()))
        {
            Shutdown();
            return false;
        }
    }

    if (!CommandContext.Initialize(
            Device.GetDevice(),
            FrameResources[0].GetCommandAllocator()) ||
        !SwapChain.Initialize(
            Device.GetFactory(),
            Device.GetDevice(),
            CommandContext.GetCommandQueue(),
            static_cast<HWND>(NativeWindowHandle),
            Width,
            Height))
    {
        Shutdown();
        return false;
    }

    bInitialized = true;
    return true;
}

void FRenderer::RenderFrame()
{
    if (!bInitialized)
    {
        return;
    }

    const std::uint32_t FrameIndex = SwapChain.GetCurrentBackBufferIndex();
    FFrameResource& FrameResource = FrameResources[FrameIndex];
    if (!CommandContext.BeginFrame(FrameResource))
    {
        return;
    }

    ID3D12GraphicsCommandList* CommandList = CommandContext.GetCommandList();
    ID3D12Resource* BackBuffer = SwapChain.GetCurrentBackBuffer();
    const D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView =
        SwapChain.GetCurrentRenderTargetView();

    CommandContext.TransitionResource(
        BackBuffer,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    CommandList->OMSetRenderTargets(1, &RenderTargetView, FALSE, nullptr);
    constexpr float ClearColor[] = {0.035f, 0.055f, 0.085f, 1.0f};
    CommandList->ClearRenderTargetView(RenderTargetView, ClearColor, 0, nullptr);

    CommandContext.TransitionResource(
        BackBuffer,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    if (!CommandContext.ExecuteCommandList())
    {
        return;
    }

    const bool bPresented = SwapChain.Present();
    const bool bSignaled = CommandContext.Signal(FrameResource);
    if (!bPresented || !bSignaled)
    {
        return;
    }
}

bool FRenderer::Resize(std::uint32_t Width, std::uint32_t Height)
{
    if (!bInitialized || Width == 0 || Height == 0)
    {
        return false;
    }

    if (!CommandContext.WaitForGpu())
    {
        return false;
    }

    for (FFrameResource& FrameResource : FrameResources)
    {
        FrameResource.SetFenceValue(0);
    }

    return SwapChain.Resize(Device.GetDevice(), Width, Height);
}

void FRenderer::Shutdown()
{
    CommandContext.WaitForGpu();

    SwapChain.Shutdown();
    CommandContext.Shutdown();
    for (FFrameResource& FrameResource : FrameResources)
    {
        FrameResource.Shutdown();
    }
    Device.Shutdown();

    bInitialized = false;
}
