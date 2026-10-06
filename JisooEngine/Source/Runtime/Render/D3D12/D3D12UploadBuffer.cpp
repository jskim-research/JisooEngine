#include "Runtime/Render/D3D12/D3D12UploadBuffer.h"

#include <limits>

FD3D12UploadBuffer::~FD3D12UploadBuffer()
{
    Shutdown();
}

bool FD3D12UploadBuffer::Initialize(
    ID3D12Device* Device,
    std::size_t CapacityInBytes)
{
    if (Resource != nullptr)
    {
        return true;
    }

    if (Device == nullptr || CapacityInBytes == 0 ||
        CapacityInBytes > (std::numeric_limits<UINT64>::max)())
    {
        return false;
    }

    D3D12_HEAP_PROPERTIES HeapProperties{};
    HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProperties.CreationNodeMask = 1;
    HeapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC ResourceDescription{};
    ResourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDescription.Alignment = 0;
    ResourceDescription.Width = static_cast<UINT64>(CapacityInBytes);
    ResourceDescription.Height = 1;
    ResourceDescription.DepthOrArraySize = 1;
    ResourceDescription.MipLevels = 1;
    ResourceDescription.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDescription.SampleDesc.Count = 1;
    ResourceDescription.SampleDesc.Quality = 0;
    ResourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (FAILED(Device->CreateCommittedResource(
            &HeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDescription,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&Resource))))
    {
        Shutdown();
        return false;
    }

    D3D12_RANGE ReadRange{0, 0};
    if (FAILED(Resource->Map(
            0,
            &ReadRange,
            reinterpret_cast<void**>(&MappedAddress))))
    {
        Shutdown();
        return false;
    }

    Capacity = CapacityInBytes;
    Offset = 0;
    return true;
}

void FD3D12UploadBuffer::Shutdown()
{
    if (Resource != nullptr && MappedAddress != nullptr)
    {
        Resource->Unmap(0, nullptr);
    }

    MappedAddress = nullptr;
    Resource.Reset();
    Capacity = 0;
    Offset = 0;
}

FD3D12UploadAllocation FD3D12UploadBuffer::Allocate(
    std::size_t SizeInBytes,
    std::size_t Alignment)
{
    if (MappedAddress == nullptr || SizeInBytes == 0 || Alignment == 0 ||
        (Alignment & (Alignment - 1)) != 0)
    {
        return {};
    }

    const std::size_t AlignedOffset = (Offset + Alignment - 1) & ~(Alignment - 1);
    if (AlignedOffset > Capacity || SizeInBytes > Capacity - AlignedOffset)
    {
        return {};
    }

    FD3D12UploadAllocation Allocation;
    Allocation.CpuAddress = MappedAddress + AlignedOffset;
    Allocation.GpuAddress = Resource->GetGPUVirtualAddress() + AlignedOffset;
    Allocation.SizeInBytes = SizeInBytes;
    Offset = AlignedOffset + SizeInBytes;
    return Allocation;
}

void FD3D12UploadBuffer::Reset()
{
    Offset = 0;
}

std::size_t FD3D12UploadBuffer::GetCapacity() const
{
    return Capacity;
}

std::size_t FD3D12UploadBuffer::GetUsedSize() const
{
    return Offset;
}
