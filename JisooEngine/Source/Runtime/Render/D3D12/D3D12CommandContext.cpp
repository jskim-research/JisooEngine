#include "Runtime/Render/D3D12/D3D12CommandContext.h"

#include "Runtime/Render/D3D12/FrameResource.h"

FD3D12CommandContext::~FD3D12CommandContext()
{
    Shutdown();
}

bool FD3D12CommandContext::Initialize(
    ID3D12Device* Device,
    ID3D12CommandAllocator* InitialAllocator)
{
    if (CommandQueue != nullptr)
    {
        return true;
    }

    if (Device == nullptr || InitialAllocator == nullptr)
    {
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC QueueDescription{};
    QueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    QueueDescription.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    QueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    // CreateCommandList는 열린 상태를 반환하므로 첫 BeginFrame에서 Reset할 수 있게 즉시 닫아 둔다.
    if (FAILED(Device->CreateCommandQueue(
            &QueueDescription,
            IID_PPV_ARGS(&CommandQueue))) ||
        FAILED(Device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            InitialAllocator,
            nullptr,
            IID_PPV_ARGS(&CommandList))) ||
        FAILED(CommandList->Close()) ||
        FAILED(Device->CreateFence(
            0,
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&Fence))))
    {
        Shutdown();
        return false;
    }

    FenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (FenceEvent == nullptr)
    {
        Shutdown();
        return false;
    }

    return true;
}

void FD3D12CommandContext::Shutdown()
{
    if (FenceEvent != nullptr)
    {
        CloseHandle(FenceEvent);
        FenceEvent = nullptr;
    }

    Fence.Reset();
    CommandList.Reset();
    CommandQueue.Reset();
    NextFenceValue = 1;
}

bool FD3D12CommandContext::BeginFrame(FFrameResource& FrameResource)
{
    // GPU가 이전 명령을 참조하는 동안 같은 Frame Slot의 Allocator를 Reset하면 안 된다.
    if (!WaitForFence(FrameResource.GetFenceValue()))
    {
        return false;
    }

    ID3D12CommandAllocator* CommandAllocator = FrameResource.GetCommandAllocator();
    if (CommandAllocator == nullptr ||
        FAILED(CommandAllocator->Reset()) ||
        FAILED(CommandList->Reset(CommandAllocator, nullptr)))
    {
        return false;
    }

    return true;
}

void FD3D12CommandContext::TransitionResource(
    ID3D12Resource* Resource,
    D3D12_RESOURCE_STATES StateBefore,
    D3D12_RESOURCE_STATES StateAfter)
{
    D3D12_RESOURCE_BARRIER Barrier{};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    Barrier.Transition.pResource = Resource;
    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    Barrier.Transition.StateBefore = StateBefore;
    Barrier.Transition.StateAfter = StateAfter;

    CommandList->ResourceBarrier(1, &Barrier);
}

bool FD3D12CommandContext::ExecuteCommandList()
{
    if (CommandQueue == nullptr || CommandList == nullptr || FAILED(CommandList->Close()))
    {
        return false;
    }

    ID3D12CommandList* CommandLists[] = {CommandList.Get()};
    CommandQueue->ExecuteCommandLists(1, CommandLists);
    return true;
}

bool FD3D12CommandContext::Signal(FFrameResource& FrameResource)
{
    if (CommandQueue == nullptr || Fence == nullptr)
    {
        return false;
    }

    const std::uint64_t FenceValue = NextFenceValue++;
    if (FAILED(CommandQueue->Signal(Fence.Get(), FenceValue)))
    {
        return false;
    }

    FrameResource.SetFenceValue(FenceValue);
    return true;
}

bool FD3D12CommandContext::WaitForGpu()
{
    if (CommandQueue == nullptr || Fence == nullptr || FenceEvent == nullptr)
    {
        return true;
    }

    const std::uint64_t FenceValue = NextFenceValue++;
    if (FAILED(CommandQueue->Signal(Fence.Get(), FenceValue)))
    {
        return false;
    }

    return WaitForFence(FenceValue);
}

ID3D12CommandQueue* FD3D12CommandContext::GetCommandQueue() const noexcept
{
    return CommandQueue.Get();
}

ID3D12GraphicsCommandList* FD3D12CommandContext::GetCommandList() const noexcept
{
    return CommandList.Get();
}

bool FD3D12CommandContext::WaitForFence(std::uint64_t FenceValue)
{
    if (FenceValue == 0)
    {
        return true;
    }

    if (Fence == nullptr || FenceEvent == nullptr)
    {
        return false;
    }

    if (Fence->GetCompletedValue() >= FenceValue)
    {
        return true;
    }

    if (FAILED(Fence->SetEventOnCompletion(FenceValue, FenceEvent)))
    {
        return false;
    }

    return WaitForSingleObject(FenceEvent, INFINITE) == WAIT_OBJECT_0;
}
