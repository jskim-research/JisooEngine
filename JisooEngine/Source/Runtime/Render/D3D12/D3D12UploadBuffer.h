#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstddef>
#include <cstdint>

struct FD3D12UploadAllocation
{
    void* CpuAddress = nullptr;
    D3D12_GPU_VIRTUAL_ADDRESS GpuAddress = 0;
    std::size_t SizeInBytes = 0;

    [[nodiscard]] bool IsValid() const
    {
        return CpuAddress != nullptr && GpuAddress != 0 && SizeInBytes > 0;
    }
};

/**
 * 한 FrameResource 안에서 CPU가 순차 기록하는 Persistently Mapped Upload Heap이다.
 * Reset은 해당 FrameResource의 이전 GPU 사용이 Fence로 완료된 뒤에만 호출해야 한다.
 */
class FD3D12UploadBuffer
{
public:
    FD3D12UploadBuffer() = default;
    ~FD3D12UploadBuffer();

    FD3D12UploadBuffer(const FD3D12UploadBuffer&) = delete;
    FD3D12UploadBuffer& operator=(const FD3D12UploadBuffer&) = delete;

    bool Initialize(ID3D12Device* Device, std::size_t CapacityInBytes);
    void Shutdown();

    /** 지정한 Alignment에 맞춘 연속 영역을 할당하며 공간이 부족하면 빈 Allocation을 반환한다. */
    [[nodiscard]] FD3D12UploadAllocation Allocate(
        std::size_t SizeInBytes,
        std::size_t Alignment);

    void Reset();
    [[nodiscard]] std::size_t GetCapacity() const;
    [[nodiscard]] std::size_t GetUsedSize() const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
    std::byte* MappedAddress = nullptr;
    std::size_t Capacity = 0;
    std::size_t Offset = 0;
};
