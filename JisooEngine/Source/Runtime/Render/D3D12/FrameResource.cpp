#include "Runtime/Render/D3D12/FrameResource.h"

FFrameResource::~FFrameResource()
{
    Shutdown();
}

bool FFrameResource::Initialize(ID3D12Device* Device)
{
    if (CommandAllocator != nullptr)
    {
        return true;
    }

    if (Device == nullptr ||
        FAILED(Device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&CommandAllocator))))
    {
        Shutdown();
        return false;
    }

    return true;
}

void FFrameResource::Shutdown()
{
    CommandAllocator.Reset();
    FenceValue = 0;
}

ID3D12CommandAllocator* FFrameResource::GetCommandAllocator() const noexcept
{
    return CommandAllocator.Get();
}

std::uint64_t FFrameResource::GetFenceValue() const noexcept
{
    return FenceValue;
}

void FFrameResource::SetFenceValue(std::uint64_t Value) noexcept
{
    FenceValue = Value;
}
