#pragma once

#include "Runtime/Render/D3D12/D3D12UploadBuffer.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

/**
 * 한 Frame Slot에서 재사용하는 CommandAllocator와 마지막 제출 Fence 값을 함께 보관한다.
 * Allocator는 저장된 Fence 값이 완료된 뒤에만 Reset할 수 있다.
 */
class FFrameResource
{
public:
    FFrameResource() = default;
    ~FFrameResource();

    FFrameResource(const FFrameResource&) = delete;
    FFrameResource& operator=(const FFrameResource&) = delete;

    bool Initialize(ID3D12Device* Device);
    void Shutdown();

    ID3D12CommandAllocator* GetCommandAllocator() const noexcept;
    FD3D12UploadBuffer& GetDynamicUploadBuffer();
    std::uint64_t GetFenceValue() const noexcept;
    void SetFenceValue(std::uint64_t Value) noexcept;

    /** 이전 GPU 사용 완료가 확인된 Frame Slot의 선형 Upload 할당 위치를 되돌린다. */
    void ResetDynamicUploadBuffer();

private:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
    FD3D12UploadBuffer DynamicUploadBuffer;
    std::uint64_t FenceValue = 0;
};
