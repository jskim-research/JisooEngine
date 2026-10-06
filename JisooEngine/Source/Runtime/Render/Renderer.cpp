#include "Runtime/Render/Renderer.h"

#include "Runtime/Engine/Viewport/SceneView.h"
#include "Runtime/Render/Mesh/MeshBatchCollection.h"
#include "Runtime/Render/Scene/Scene.h"

#include <algorithm>

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

    D3D12_DESCRIPTOR_HEAP_DESC RenderTargetViewHeapDescription{};
    RenderTargetViewHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RenderTargetViewHeapDescription.NumDescriptors = MaxRenderTargetCount;

    D3D12_DESCRIPTOR_HEAP_DESC ShaderResourceViewHeapDescription{};
    ShaderResourceViewHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    ShaderResourceViewHeapDescription.NumDescriptors = MaxShaderResourceDescriptorCount;
    ShaderResourceViewHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(Device.GetDevice()->CreateDescriptorHeap(
            &RenderTargetViewHeapDescription,
            IID_PPV_ARGS(&OwnedRenderTargetViewHeap))) ||
        FAILED(Device.GetDevice()->CreateDescriptorHeap(
            &ShaderResourceViewHeapDescription,
            IID_PPV_ARGS(&ShaderResourceViewHeap))))
    {
        Shutdown();
        return false;
    }

    RenderTargetViewDescriptorSize = Device.GetDevice()->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    ShaderResourceViewDescriptorSize = Device.GetDevice()->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    FreeShaderResourceDescriptorIndices.reserve(MaxShaderResourceDescriptorCount);
    for (std::uint32_t Index = MaxShaderResourceDescriptorCount; Index > 0; --Index)
    {
        FreeShaderResourceDescriptorIndices.push_back(Index - 1);
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

FRenderTargetHandle FRenderer::CreateTextureRenderTarget(
    std::uint32_t Width,
    std::uint32_t Height)
{
    if (!bInitialized || ActiveFrameResource != nullptr)
    {
        return {};
    }

    const FRenderTargetHandle Handle = RegisterRenderTarget(
        Width,
        Height,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    FRenderTargetSlot* Target = ResolveRenderTarget(Handle);
    if (Target == nullptr || !CreateOwnedRenderTargetResource(*Target, Handle.Index))
    {
        if (Target != nullptr)
        {
            Target->bAllocated = false;
        }
        return {};
    }
    return Handle;
}

bool FRenderer::ReleaseRenderTarget(FRenderTargetHandle Handle)
{
    if (Handle == MainRenderTargetHandle || ActiveFrameResource != nullptr)
    {
        return false;
    }

    FRenderTargetSlot* Target = ResolveRenderTarget(Handle);
    if (Target == nullptr || !CommandContext.WaitForGpu())
    {
        return false;
    }

    if (Target->ShaderResourceViewCpu.ptr != 0)
    {
        FreeShaderResourceDescriptor(
            Target->ShaderResourceViewCpu,
            Target->ShaderResourceViewGpu);
    }

    const std::uint32_t Generation = Target->Generation;
    *Target = {};
    Target->Generation = Generation;
    return true;
}

bool FRenderer::ResizeRenderTarget(
    FRenderTargetHandle Handle,
    std::uint32_t Width,
    std::uint32_t Height)
{
    if (Handle == MainRenderTargetHandle || ActiveFrameResource != nullptr ||
        Width == 0 || Height == 0)
    {
        return false;
    }

    FRenderTargetSlot* Target = ResolveRenderTarget(Handle);
    if (Target == nullptr || Target->OwnedResource == nullptr)
    {
        return false;
    }
    if (Target->Width == Width && Target->Height == Height)
    {
        return true;
    }
    if (!CommandContext.WaitForGpu())
    {
        return false;
    }

    const std::uint32_t PreviousWidth = Target->Width;
    const std::uint32_t PreviousHeight = Target->Height;
    Target->Width = Width;
    Target->Height = Height;
    Target->bUsedThisFrame = false;
    if (CreateOwnedRenderTargetResource(*Target, Handle.Index))
    {
        return true;
    }

    Target->Width = PreviousWidth;
    Target->Height = PreviousHeight;
    return false;
}

D3D12_GPU_DESCRIPTOR_HANDLE FRenderer::GetRenderTargetShaderResourceView(
    FRenderTargetHandle Handle) const noexcept
{
    const FRenderTargetSlot* Target = ResolveRenderTarget(Handle);
    return Target != nullptr ? Target->ShaderResourceViewGpu : D3D12_GPU_DESCRIPTOR_HANDLE{};
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

bool FRenderer::EndFrame(
    FFinalOverlayPassRecorder FinalOverlayRecorder,
    void* UserData)
{
    if (ActiveFrameResource == nullptr)
    {
        return false;
    }

    FRenderTargetSlot* MainTarget = ResolveRenderTarget(MainRenderTargetHandle);
    if (MainTarget == nullptr)
    {
        ActiveFrameResource = nullptr;
        return false;
    }

    if (FinalOverlayRecorder != nullptr)
    {
        // Overlay가 off-screen 출력을 읽으므로 Main Target을 제외한 사용 Target을 먼저 최종 상태로 만든다.
        for (FRenderTargetSlot& Target : RenderTargetSlots)
        {
            if (&Target == MainTarget || !Target.bUsedThisFrame || Target.Resource == nullptr ||
                Target.CurrentState == Target.FinalState)
            {
                continue;
            }

            CommandContext.TransitionResource(
                Target.Resource,
                Target.CurrentState,
                Target.FinalState);
            Target.CurrentState = Target.FinalState;
        }

        if (!PrepareRenderTarget(*MainTarget))
        {
            ActiveFrameResource = nullptr;
            return false;
        }

        ID3D12GraphicsCommandList* CommandList = CommandContext.GetCommandList();
        CommandList->OMSetRenderTargets(1, &MainTarget->RenderTargetView, FALSE, nullptr);
        FinalOverlayRecorder(CommandList, UserData);
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

    FRenderTargetSlot* MainTarget = ResolveRenderTarget(MainRenderTargetHandle);
    if (MainTarget == nullptr)
    {
        return false;
    }
    if (MainTarget->Width == Width && MainTarget->Height == Height)
    {
        return true;
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

    MainTarget->Resource = nullptr;
    MainTarget->RenderTargetView = {};
    MainTarget->CurrentState = D3D12_RESOURCE_STATE_PRESENT;
    MainTarget->Width = Width;
    MainTarget->Height = Height;
    MainTarget->bUsedThisFrame = false;
    return true;
}

bool FRenderer::WaitForGpu()
{
    return CommandContext.WaitForGpu();
}

ID3D12Device* FRenderer::GetD3D12Device() const noexcept
{
    return Device.GetDevice();
}

ID3D12CommandQueue* FRenderer::GetD3D12CommandQueue() const noexcept
{
    return CommandContext.GetCommandQueue();
}

ID3D12DescriptorHeap* FRenderer::GetShaderResourceViewHeap() const noexcept
{
    return ShaderResourceViewHeap.Get();
}

bool FRenderer::AllocateShaderResourceDescriptor(
    D3D12_CPU_DESCRIPTOR_HANDLE& OutCpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE& OutGpuHandle)
{
    if (ShaderResourceViewHeap == nullptr || FreeShaderResourceDescriptorIndices.empty())
    {
        return false;
    }

    const std::uint32_t Index = FreeShaderResourceDescriptorIndices.back();
    FreeShaderResourceDescriptorIndices.pop_back();
    AllocatedShaderResourceDescriptors[Index] = true;

    OutCpuHandle = ShaderResourceViewHeap->GetCPUDescriptorHandleForHeapStart();
    OutCpuHandle.ptr += static_cast<SIZE_T>(Index) * ShaderResourceViewDescriptorSize;
    OutGpuHandle = ShaderResourceViewHeap->GetGPUDescriptorHandleForHeapStart();
    OutGpuHandle.ptr += static_cast<UINT64>(Index) * ShaderResourceViewDescriptorSize;
    return true;
}

void FRenderer::FreeShaderResourceDescriptor(
    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE)
{
    if (ShaderResourceViewHeap == nullptr || CpuHandle.ptr == 0 ||
        ShaderResourceViewDescriptorSize == 0)
    {
        return;
    }

    const SIZE_T HeapStart =
        ShaderResourceViewHeap->GetCPUDescriptorHandleForHeapStart().ptr;
    if (CpuHandle.ptr < HeapStart)
    {
        return;
    }

    const SIZE_T Offset = CpuHandle.ptr - HeapStart;
    if (Offset % ShaderResourceViewDescriptorSize != 0)
    {
        return;
    }

    const std::uint32_t Index = static_cast<std::uint32_t>(
        Offset / ShaderResourceViewDescriptorSize);
    if (Index >= MaxShaderResourceDescriptorCount ||
        !AllocatedShaderResourceDescriptors[Index])
    {
        return;
    }

    AllocatedShaderResourceDescriptors[Index] = false;
    FreeShaderResourceDescriptorIndices.push_back(Index);
}

void FRenderer::Shutdown()
{
    // GPU가 참조할 수 있는 자원을 해제하기 전에 제출된 작업을 모두 완료한다.
    CommandContext.WaitForGpu();

    MeshPassPipeline.Shutdown();
    RenderTargetSlots.clear();
    FreeShaderResourceDescriptorIndices.clear();
    AllocatedShaderResourceDescriptors.fill(false);
    ShaderResourceViewHeap.Reset();
    OwnedRenderTargetViewHeap.Reset();
    MainRenderTargetHandle = {};
    SwapChain.Shutdown();
    CommandContext.Shutdown();
    for (FFrameResource& FrameResource : FrameResources)
    {
        FrameResource.Shutdown();
    }
    Device.Shutdown();

    ActiveFrameResource = nullptr;
    RenderTargetViewDescriptorSize = 0;
    ShaderResourceViewDescriptorSize = 0;
    NextRenderTargetGeneration = 0;
    bInitialized = false;
}

FRenderTargetHandle FRenderer::RegisterRenderTarget(
    std::uint32_t Width,
    std::uint32_t Height,
    D3D12_RESOURCE_STATES FinalState)
{
    if (Width == 0 || Height == 0)
    {
        return {};
    }

    ++NextRenderTargetGeneration;
    if (NextRenderTargetGeneration == 0)
    {
        ++NextRenderTargetGeneration;
    }

    auto FreeSlot = std::ranges::find_if(
        RenderTargetSlots,
        [](const FRenderTargetSlot& Slot)
        {
            return !Slot.bAllocated;
        });

    std::uint32_t SlotIndex = 0;
    if (FreeSlot != RenderTargetSlots.end())
    {
        SlotIndex = static_cast<std::uint32_t>(
            std::distance(RenderTargetSlots.begin(), FreeSlot));
    }
    else
    {
        if (RenderTargetSlots.size() >= MaxRenderTargetCount)
        {
            return {};
        }
        SlotIndex = static_cast<std::uint32_t>(RenderTargetSlots.size());
        RenderTargetSlots.emplace_back();
    }

    FRenderTargetSlot& Slot = RenderTargetSlots[SlotIndex];
    Slot = {};
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

bool FRenderer::CreateOwnedRenderTargetResource(
    FRenderTargetSlot& Target,
    std::uint32_t SlotIndex)
{
    if (Target.Width == 0 || Target.Height == 0 ||
        SlotIndex >= MaxRenderTargetCount)
    {
        return false;
    }

    const bool bNeedsShaderResourceDescriptor =
        Target.ShaderResourceViewCpu.ptr == 0;
    if (bNeedsShaderResourceDescriptor && !AllocateShaderResourceDescriptor(
            Target.ShaderResourceViewCpu,
            Target.ShaderResourceViewGpu))
    {
        return false;
    }

    D3D12_HEAP_PROPERTIES HeapProperties{};
    HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC ResourceDescription{};
    ResourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    ResourceDescription.Width = Target.Width;
    ResourceDescription.Height = Target.Height;
    ResourceDescription.DepthOrArraySize = 1;
    ResourceDescription.MipLevels = 1;
    ResourceDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ResourceDescription.SampleDesc.Count = 1;
    ResourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    ResourceDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE ClearValue{};
    ClearValue.Format = ResourceDescription.Format;
    ClearValue.Color[0] = 0.035f;
    ClearValue.Color[1] = 0.055f;
    ClearValue.Color[2] = 0.085f;
    ClearValue.Color[3] = 1.0f;

    Microsoft::WRL::ComPtr<ID3D12Resource> NewResource;
    if (FAILED(Device.GetDevice()->CreateCommittedResource(
            &HeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDescription,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &ClearValue,
            IID_PPV_ARGS(&NewResource))))
    {
        if (bNeedsShaderResourceDescriptor)
        {
            FreeShaderResourceDescriptor(
                Target.ShaderResourceViewCpu,
                Target.ShaderResourceViewGpu);
            Target.ShaderResourceViewCpu = {};
            Target.ShaderResourceViewGpu = {};
        }
        return false;
    }

    Target.RenderTargetView = GetOwnedRenderTargetView(SlotIndex);
    Device.GetDevice()->CreateRenderTargetView(
        NewResource.Get(),
        nullptr,
        Target.RenderTargetView);

    D3D12_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDescription{};
    ShaderResourceViewDescription.Format = ResourceDescription.Format;
    ShaderResourceViewDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    ShaderResourceViewDescription.Shader4ComponentMapping =
        D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    ShaderResourceViewDescription.Texture2D.MipLevels = 1;
    Device.GetDevice()->CreateShaderResourceView(
        NewResource.Get(),
        &ShaderResourceViewDescription,
        Target.ShaderResourceViewCpu);

    Target.OwnedResource = std::move(NewResource);
    Target.Resource = Target.OwnedResource.Get();
    Target.CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderer::GetOwnedRenderTargetView(
    std::uint32_t SlotIndex) const noexcept
{
    D3D12_CPU_DESCRIPTOR_HANDLE Handle{};
    if (OwnedRenderTargetViewHeap != nullptr)
    {
        Handle = OwnedRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
        Handle.ptr += static_cast<SIZE_T>(SlotIndex) * RenderTargetViewDescriptorSize;
    }
    return Handle;
}

bool FRenderer::PrepareRenderTarget(FRenderTargetSlot& Target)
{
    if (Target.Resource == nullptr || Target.RenderTargetView.ptr == 0)
    {
        return false;
    }

    if (Target.CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
    {
        CommandContext.TransitionResource(
            Target.Resource,
            Target.CurrentState,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        Target.CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    if (Target.bUsedThisFrame)
    {
        return true;
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
