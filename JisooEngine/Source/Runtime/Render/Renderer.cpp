#include "Runtime/Render/Renderer.h"

#include "Runtime/Core/Math/MathTypes.h"
#include "Runtime/Render/Mesh/MeshBatchCollection.h"
#include "Runtime/Render/Scene/Scene.h"

#include <cmath>
#include <numbers>

namespace
{
    FMatrix MakeFixedViewProjection(std::uint32_t Width, std::uint32_t Height)
    {
        FMatrix View{};
        View.M[0][2] = 1.0f;
        View.M[1][0] = 1.0f;
        View.M[2][1] = 1.0f;
        View.M[3][3] = 1.0f;

        constexpr float FieldOfViewDegrees = 60.0f;
        constexpr float NearPlane = 1.0f;
        constexpr float FarPlane = 100000.0f;
        const float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
        const float FieldOfViewRadians =
            FieldOfViewDegrees * std::numbers::pi_v<float> / 180.0f;
        const float YScale = 1.0f / std::tan(FieldOfViewRadians * 0.5f);
        const float XScale = YScale / AspectRatio;

        FMatrix Projection{};
        Projection.M[0][0] = XScale;
        Projection.M[1][1] = YScale;
        Projection.M[2][2] = FarPlane / (FarPlane - NearPlane);
        Projection.M[2][3] = 1.0f;
        Projection.M[3][2] = -NearPlane * FarPlane / (FarPlane - NearPlane);
        return View * Projection;
    }
}

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
            Height) ||
        !MeshPassPipeline.Initialize({
            *Device.GetDevice(),
            DXGI_FORMAT_R8G8B8A8_UNORM}))
    {
        Shutdown();
        return false;
    }

    ViewportWidth = Width;
    ViewportHeight = Height;
    bInitialized = true;
    return true;
}

void FRenderer::RenderFrame(const FScene* Scene)
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

    constexpr float ClearColor[] = {0.035f, 0.055f, 0.085f, 1.0f};
    CommandList->ClearRenderTargetView(RenderTargetView, ClearColor, 0, nullptr);

    if (Scene != nullptr)
    {
        FMeshBatchCollector Collector;
        GatherVisibleMeshBatches(*Scene, Collector);

        const FMatrix ViewProjection = MakeFixedViewProjection(
            ViewportWidth,
            ViewportHeight);

        MeshPassPipeline.Execute(
            Collector,
            {
                *CommandList,
                FrameResource,
                RenderTargetView,
                ViewportWidth,
                ViewportHeight,
                ViewProjection
            });
    }

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

    if (!SwapChain.Resize(Device.GetDevice(), Width, Height))
    {
        return false;
    }

    ViewportWidth = Width;
    ViewportHeight = Height;
    return true;
}

void FRenderer::Shutdown()
{
    // GPU가 참조할 수 있는 자원을 해제하기 전에 제출된 작업을 모두 완료한다.
    CommandContext.WaitForGpu();

    MeshPassPipeline.Shutdown();
    SwapChain.Shutdown();
    CommandContext.Shutdown();
    for (FFrameResource& FrameResource : FrameResources)
    {
        FrameResource.Shutdown();
    }
    Device.Shutdown();

    ViewportWidth = 0;
    ViewportHeight = 0;
    bInitialized = false;
}
