#pragma once

#include "Runtime/Render/D3D12/D3D12CommandContext.h"
#include "Runtime/Render/D3D12/D3D12Device.h"
#include "Runtime/Render/D3D12/DXGISwapChain.h"
#include "Runtime/Render/D3D12/FrameResource.h"
#include "Runtime/Render/Mesh/MeshPassPipeline.h"
#include "Runtime/Render/Target/RenderTargetHandle.h"

#include <array>
#include <cstdint>
#include <vector>

class FScene;
struct FSceneViewFamily;

/**
 * D3D12 렌더링 하위 객체, RenderTarget 슬롯과 MeshPassPipeline의 생애주기를 조정한다.
 * ViewFamily의 Target Handle을 실제 GPU 출력 자원으로 해석하고 프레임 제출 순서를 관리한다.
 */
class FRenderer
{
public:
    using FExternalRenderPassRecorder = void(*)(
        ID3D12GraphicsCommandList* CommandList,
        void* UserData);

    FRenderer() = default;
    ~FRenderer();

    FRenderer(const FRenderer&) = delete;
    FRenderer& operator=(const FRenderer&) = delete;

    /**
     * Native Window에 연결된 D3D12 Device, Command Context와 SwapChain을 생성한다.
     * 초기화가 완료된 뒤 다시 호출하면 현재 상태를 유지하고 성공을 반환한다.
     */
    bool Initialize(
        void* NativeWindowHandle,
        std::uint32_t Width,
        std::uint32_t Height);

    /** Initialize 이후 생성된 Window SwapChain 출력의 안정적인 논리 Handle을 반환한다. */
    [[nodiscard]] FRenderTargetHandle GetMainRenderTargetHandle() const noexcept;

    /** Shader Resource로 표시할 수 있는 Renderer 소유 Color Target을 생성한다. */
    [[nodiscard]] FRenderTargetHandle CreateTextureRenderTarget(
        std::uint32_t Width,
        std::uint32_t Height);

    /** GPU 사용을 완료한 뒤 Target 자원과 Descriptor를 반환하며 Main Target은 해제하지 않는다. */
    bool ReleaseRenderTarget(FRenderTargetHandle Handle);

    /** GPU 사용을 완료한 뒤 기존 Handle을 유지하면서 Texture Target을 새 크기로 다시 생성한다. */
    bool ResizeRenderTarget(
        FRenderTargetHandle Handle,
        std::uint32_t Width,
        std::uint32_t Height);

    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetRenderTargetShaderResourceView(
        FRenderTargetHandle Handle) const noexcept;

    /**
     * 현재 BackBuffer용 FrameResource를 확보하고 명령 기록을 시작한다.
     * 성공한 호출은 RenderViewFamily를 0회 이상 호출한 뒤 EndFrame으로 닫아야 한다.
     */
    bool BeginFrame();

    /**
     * 프레임 로컬 ViewFamily를 현재 CommandList에 기록한다.
     * BeginFrame과 EndFrame 사이에서만 호출하며 Family와 Scene은 반환할 때까지 유효해야 한다.
     */
    void RenderViewFamily(const FSceneViewFamily& ViewFamily);

    /** 현재 프레임에서 그린 Texture Target을 이후 Pass가 읽을 수 있는 상태로 전환한다. */
    bool PrepareRenderTargetForSampling(FRenderTargetHandle Handle);

    /** 지정 Target을 준비하고 RTV를 연결한 상태에서 외부 Pass의 명령 기록을 동기 호출한다. */
    bool RecordExternalRenderPass(
        FRenderTargetHandle Handle,
        FExternalRenderPassRecorder Recorder,
        void* UserData);

    /** 사용한 Target을 최종 상태로 전환하고 명령 제출·Present·Fence 기록을 수행한다. */
    bool EndFrame();

    /**
     * 제출된 GPU 작업을 완료한 뒤 SwapChain BackBuffer를 새 크기로 다시 생성한다.
     * Width와 Height가 0인 최소화 상태는 처리하지 않는다.
     */
    bool Resize(std::uint32_t Width, std::uint32_t Height);

    /** 현재까지 제출한 Renderer 작업이 끝날 때까지 현재 Thread를 block한다. */
    bool WaitForGpu();

    [[nodiscard]] ID3D12Device* GetD3D12Device() const noexcept;
    [[nodiscard]] ID3D12CommandQueue* GetD3D12CommandQueue() const noexcept;
    [[nodiscard]] ID3D12DescriptorHeap* GetShaderResourceViewHeap() const noexcept;

    /** Editor Backend용 Shader-visible Descriptor 한 칸을 할당한다. */
    bool AllocateShaderResourceDescriptor(
        D3D12_CPU_DESCRIPTOR_HANDLE& OutCpuHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE& OutGpuHandle);

    /** AllocateShaderResourceDescriptor로 받은 Descriptor를 반환한다. */
    void FreeShaderResourceDescriptor(
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle);

    /** 제출된 GPU 작업을 완료한 뒤 렌더링 자원을 의존 관계의 역순으로 해제한다. */
    void Shutdown();

private:
    static constexpr std::uint32_t MaxRenderTargetCount = 8;
    static constexpr std::uint32_t MaxShaderResourceDescriptorCount = 64;

    struct FRenderTargetSlot
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> OwnedResource;
        ID3D12Resource* Resource = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView{};
        D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceViewCpu{};
        D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceViewGpu{};
        D3D12_RESOURCE_STATES CurrentState = D3D12_RESOURCE_STATE_COMMON;
        D3D12_RESOURCE_STATES FinalState = D3D12_RESOURCE_STATE_COMMON;
        std::uint32_t Width = 0;
        std::uint32_t Height = 0;
        std::uint32_t Generation = 0;
        bool bAllocated = false;
        bool bUsedThisFrame = false;
    };

    [[nodiscard]] FRenderTargetHandle RegisterRenderTarget(
        std::uint32_t Width,
        std::uint32_t Height,
        D3D12_RESOURCE_STATES FinalState);
    [[nodiscard]] FRenderTargetSlot* ResolveRenderTarget(FRenderTargetHandle Handle);
    [[nodiscard]] const FRenderTargetSlot* ResolveRenderTarget(
        FRenderTargetHandle Handle) const;
    bool CreateOwnedRenderTargetResource(FRenderTargetSlot& Target, std::uint32_t SlotIndex);
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetOwnedRenderTargetView(
        std::uint32_t SlotIndex) const noexcept;
    bool PrepareRenderTarget(FRenderTargetSlot& Target);

    FD3D12Device Device;
    FD3D12CommandContext CommandContext;
    FDXGISwapChain SwapChain;
    std::array<FFrameResource, FDXGISwapChain::BufferCount> FrameResources;
    FMeshPassPipeline MeshPassPipeline;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> OwnedRenderTargetViewHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ShaderResourceViewHeap;
    std::vector<FRenderTargetSlot> RenderTargetSlots;
    std::vector<std::uint32_t> FreeShaderResourceDescriptorIndices;
    std::array<bool, MaxShaderResourceDescriptorCount> AllocatedShaderResourceDescriptors{};
    FRenderTargetHandle MainRenderTargetHandle;
    FFrameResource* ActiveFrameResource = nullptr;
    std::uint32_t RenderTargetViewDescriptorSize = 0;
    std::uint32_t ShaderResourceViewDescriptorSize = 0;
    std::uint32_t NextRenderTargetGeneration = 0;
    bool bInitialized = false;
};
