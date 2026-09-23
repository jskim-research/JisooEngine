#pragma once

#include <Windows.h>

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

class FFrameResource;

/**
 * 그래픽 명령 기록·제출과 GPU 동기화를 관리한다.
 * Direct CommandQueue, 재사용하는 CommandList와 Fence의 생애주기를 소유한다.
 */
class FD3D12CommandContext
{
public:
    FD3D12CommandContext() = default;
    ~FD3D12CommandContext();

    FD3D12CommandContext(const FD3D12CommandContext&) = delete;
    FD3D12CommandContext& operator=(const FD3D12CommandContext&) = delete;

    /**
     * Direct CommandQueue, CommandList와 Fence를 생성한다.
     * InitialAllocator는 CommandList를 최초 생성할 때만 사용하며 소유권을 가져오지 않는다.
     */
    bool Initialize(ID3D12Device* Device, ID3D12CommandAllocator* InitialAllocator);
    void Shutdown();

    /**
     * FrameResource의 이전 GPU 사용이 끝날 때까지 기다린 뒤 Allocator와 CommandList를 Reset한다.
     * 해당 Frame Slot이 아직 사용 중이면 현재 Thread를 block할 수 있다.
     */
    bool BeginFrame(FFrameResource& FrameResource);

    /**
     * Resource의 모든 Subresource에 Transition Barrier를 기록한다.
     * 호출자는 StateBefore가 실제 현재 상태와 일치하도록 보장해야 한다.
     */
    void TransitionResource(
        ID3D12Resource* Resource,
        D3D12_RESOURCE_STATES StateBefore,
        D3D12_RESOURCE_STATES StateAfter);

    /** 열려 있는 CommandList를 닫고 Direct CommandQueue에 제출한다. */
    bool ExecuteCommandList();

    /**
     * 현재까지 CommandQueue에 제출된 명령 뒤에 Fence 신호를 예약하고,
     * FrameResource가 다시 사용될 수 있는 완료 지점을 저장한다.
     */
    bool Signal(FFrameResource& FrameResource);

    /** 현재까지 CommandQueue에 제출된 모든 명령이 완료될 때까지 현재 Thread를 block한다. */
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
