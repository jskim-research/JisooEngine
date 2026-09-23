#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

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
    std::uint64_t GetFenceValue() const noexcept;
    void SetFenceValue(std::uint64_t Value) noexcept;

private:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
    std::uint64_t FenceValue = 0;
};
