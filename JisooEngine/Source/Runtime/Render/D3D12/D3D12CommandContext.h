#pragma once

#include <Windows.h>

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

class FFrameResource;

class FD3D12CommandContext
{
public:
    FD3D12CommandContext() = default;
    ~FD3D12CommandContext();

    FD3D12CommandContext(const FD3D12CommandContext&) = delete;
    FD3D12CommandContext& operator=(const FD3D12CommandContext&) = delete;

    bool Initialize(ID3D12Device* Device, ID3D12CommandAllocator* InitialAllocator);
    void Shutdown();

    bool BeginFrame(FFrameResource& FrameResource);
    void TransitionResource(
        ID3D12Resource* Resource,
        D3D12_RESOURCE_STATES StateBefore,
        D3D12_RESOURCE_STATES StateAfter);
    bool ExecuteCommandList();
    bool Signal(FFrameResource& FrameResource);
    bool WaitForGpu();

    ID3D12CommandQueue* GetCommandQueue() const noexcept;
    ID3D12GraphicsCommandList* GetCommandList() const noexcept;

private:
    bool WaitForFence(std::uint64_t FenceValue);

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
    HANDLE FenceEvent = nullptr;
    std::uint64_t NextFenceValue = 1;
};
