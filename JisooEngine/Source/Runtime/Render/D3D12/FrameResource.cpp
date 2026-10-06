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

    constexpr std::size_t DynamicUploadCapacity = 64 * 1024;
    if (Device == nullptr ||
        FAILED(Device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&CommandAllocator))) ||
        !DynamicUploadBuffer.Initialize(Device, DynamicUploadCapacity))
    {
        Shutdown();
        return false;
    }

    return true;
}

void FFrameResource::Shutdown()
{
    DynamicUploadBuffer.Shutdown();
    CommandAllocator.Reset();
    FenceValue = 0;
}

ID3D12CommandAllocator* FFrameResource::GetCommandAllocator() const noexcept
{
    return CommandAllocator.Get();
}

FD3D12UploadBuffer& FFrameResource::GetDynamicUploadBuffer()
{
    return DynamicUploadBuffer;
}

std::uint64_t FFrameResource::GetFenceValue() const noexcept
{
    return FenceValue;
}

void FFrameResource::SetFenceValue(std::uint64_t Value) noexcept
{
    FenceValue = Value;
}

void FFrameResource::ResetDynamicUploadBuffer()
{
    DynamicUploadBuffer.Reset();
}
