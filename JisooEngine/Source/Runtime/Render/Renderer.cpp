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

    // CommandList 생성에는 Allocator가 필요하므로 첫 FrameResource의 Allocator를 초기 생성에 사용한다.
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

    // ResizeBuffers 전에 BackBuffer를 참조하는 GPU 작업과 CPU 소유 참조를 모두 끝내야 한다.
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
    // GPU가 참조할 수 있는 자원을 해제하기 전에 제출된 작업을 모두 완료한다.
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
