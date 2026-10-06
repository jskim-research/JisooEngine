#include "Runtime/Render/Renderer.h"

#include "Runtime/Engine/Viewport/SceneView.h"
#include "Runtime/Render/Mesh/MeshBatchCollection.h"
#include "Runtime/Render/Scene/Scene.h"

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

    MainRenderTargetHandle = RegisterRenderTarget(
        Width,
        Height,
        D3D12_RESOURCE_STATE_PRESENT);
    if (!MainRenderTargetHandle.IsSet())
    {
        Shutdown();
        return false;
    }

    bInitialized = true;
    return true;
}

FRenderTargetHandle FRenderer::GetMainRenderTargetHandle() const noexcept
{
    return MainRenderTargetHandle;
}

bool FRenderer::BeginFrame()
{
    if (!bInitialized || ActiveFrameResource != nullptr)
    {
        return false;
    }

    FRenderTargetSlot* MainTarget = ResolveRenderTarget(MainRenderTargetHandle);
    if (MainTarget == nullptr)
    {
        return false;
    }

    const std::uint32_t FrameIndex = SwapChain.GetCurrentBackBufferIndex();
    FFrameResource& FrameResource = FrameResources[FrameIndex];
    if (!CommandContext.BeginFrame(FrameResource))
    {
        return false;
    }

    for (FRenderTargetSlot& Target : RenderTargetSlots)
    {
        Target.bUsedThisFrame = false;
    }

    MainTarget->Resource = SwapChain.GetCurrentBackBuffer();
    MainTarget->RenderTargetView = SwapChain.GetCurrentRenderTargetView();
    MainTarget->CurrentState = D3D12_RESOURCE_STATE_PRESENT;
    ActiveFrameResource = &FrameResource;
    return true;
}

void FRenderer::RenderViewFamily(const FSceneViewFamily& ViewFamily)
{
    if (ActiveFrameResource == nullptr || ViewFamily.Scene == nullptr)
    {
        return;
    }

    FRenderTargetSlot* Target = ResolveRenderTarget(ViewFamily.Output.Target);
    if (Target == nullptr || ViewFamily.Output.Width == 0 ||
        ViewFamily.Output.Height == 0 || ViewFamily.Output.Width > Target->Width ||
        ViewFamily.Output.Height > Target->Height || !PrepareRenderTarget(*Target))
    {
        return;
    }

    ID3D12GraphicsCommandList* CommandList = CommandContext.GetCommandList();
    for (const FSceneView& View : ViewFamily.Views)
    {
        if (View.ViewRect.Width == 0 || View.ViewRect.Height == 0 ||
            View.ViewRect.X > ViewFamily.Output.Width ||
            View.ViewRect.Y > ViewFamily.Output.Height ||
            View.ViewRect.Width > ViewFamily.Output.Width - View.ViewRect.X ||
            View.ViewRect.Height > ViewFamily.Output.Height - View.ViewRect.Y)
        {
            continue;
        }

        FMeshBatchCollector Collector;
        GatherVisibleMeshBatches(*ViewFamily.Scene, Collector);

        MeshPassPipeline.Execute(
            Collector,
            {
                *CommandList,
                *ActiveFrameResource,
                Target->RenderTargetView,
                View.ViewRect.X,
                View.ViewRect.Y,
                View.ViewRect.Width,
                View.ViewRect.Height,
                View.GetViewProjectionMatrix()
            });
    }
}

bool FRenderer::EndFrame()
{
    if (ActiveFrameResource == nullptr)
    {
        return false;
    }

    for (FRenderTargetSlot& Target : RenderTargetSlots)
    {
        if (!Target.bUsedThisFrame || Target.Resource == nullptr)
        {
            continue;
        }

        if (Target.CurrentState != Target.FinalState)
        {
            CommandContext.TransitionResource(
                Target.Resource,
                Target.CurrentState,
                Target.FinalState);
            Target.CurrentState = Target.FinalState;
        }
    }

    if (!CommandContext.ExecuteCommandList())
    {
        ActiveFrameResource = nullptr;
        return false;
    }

    const bool bPresented = SwapChain.Present();
    const bool bSignaled = CommandContext.Signal(*ActiveFrameResource);
    ActiveFrameResource = nullptr;
    return bPresented && bSignaled;
}

bool FRenderer::Resize(std::uint32_t Width, std::uint32_t Height)
{
    if (!bInitialized || ActiveFrameResource != nullptr || Width == 0 || Height == 0)
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

    FRenderTargetSlot* MainTarget = ResolveRenderTarget(MainRenderTargetHandle);
    if (MainTarget == nullptr)
    {
        return false;
    }

    MainTarget->Resource = nullptr;
    MainTarget->RenderTargetView = {};
    MainTarget->CurrentState = D3D12_RESOURCE_STATE_PRESENT;
    MainTarget->Width = Width;
    MainTarget->Height = Height;
    MainTarget->bUsedThisFrame = false;
    return true;
}

void FRenderer::Shutdown()
{
    // GPU가 참조할 수 있는 자원을 해제하기 전에 제출된 작업을 모두 완료한다.
    CommandContext.WaitForGpu();

    MeshPassPipeline.Shutdown();
    RenderTargetSlots.clear();
    MainRenderTargetHandle = {};
    SwapChain.Shutdown();
    CommandContext.Shutdown();
    for (FFrameResource& FrameResource : FrameResources)
    {
        FrameResource.Shutdown();
    }
    Device.Shutdown();

    ActiveFrameResource = nullptr;
    bInitialized = false;
}

FRenderTargetHandle FRenderer::RegisterRenderTarget(
    std::uint32_t Width,
    std::uint32_t Height,
    D3D12_RESOURCE_STATES FinalState)
{
    if (Width == 0 || Height == 0 ||
        RenderTargetSlots.size() >= FRenderTargetHandle::InvalidIndex)
    {
        return {};
    }

    ++NextRenderTargetGeneration;
    if (NextRenderTargetGeneration == 0)
    {
        ++NextRenderTargetGeneration;
    }

    const std::uint32_t SlotIndex =
        static_cast<std::uint32_t>(RenderTargetSlots.size());
    FRenderTargetSlot& Slot = RenderTargetSlots.emplace_back();
    Slot.CurrentState = FinalState;
    Slot.FinalState = FinalState;
    Slot.Width = Width;
    Slot.Height = Height;
    Slot.Generation = NextRenderTargetGeneration;
    Slot.bAllocated = true;
    return {SlotIndex, Slot.Generation};
}

FRenderer::FRenderTargetSlot* FRenderer::ResolveRenderTarget(
    FRenderTargetHandle Handle)
{
    return const_cast<FRenderTargetSlot*>(
        static_cast<const FRenderer*>(this)->ResolveRenderTarget(Handle));
}

const FRenderer::FRenderTargetSlot* FRenderer::ResolveRenderTarget(
    FRenderTargetHandle Handle) const
{
    if (!Handle.IsSet() || Handle.Index >= RenderTargetSlots.size())
    {
        return nullptr;
    }

    const FRenderTargetSlot& Slot = RenderTargetSlots[Handle.Index];
    return Slot.bAllocated && Slot.Generation == Handle.Generation ? &Slot : nullptr;
}

bool FRenderer::PrepareRenderTarget(FRenderTargetSlot& Target)
{
    if (Target.Resource == nullptr || Target.RenderTargetView.ptr == 0)
    {
        return false;
    }

    if (Target.bUsedThisFrame)
    {
        return true;
    }

    if (Target.CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
    {
        CommandContext.TransitionResource(
            Target.Resource,
            Target.CurrentState,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        Target.CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    constexpr float ClearColor[] = {0.035f, 0.055f, 0.085f, 1.0f};
    CommandContext.GetCommandList()->ClearRenderTargetView(
        Target.RenderTargetView,
        ClearColor,
        0,
        nullptr);
    Target.bUsedThisFrame = true;
    return true;
}
