#pragma once

#include "Runtime/Render/D3D12/D3D12CommandContext.h"
#include "Runtime/Render/D3D12/D3D12Device.h"
#include "Runtime/Render/D3D12/DXGISwapChain.h"
#include "Runtime/Render/D3D12/FrameResource.h"
#include "Runtime/Render/Mesh/MeshPassPipeline.h"

#include <array>
#include <cstdint>

class FScene;

/**
 * D3D12 렌더링 하위 객체와 MeshPassPipeline의 생애주기 및 프레임 제출 순서를 조정한다.
 * 구체 Mesh Pass 정책은 소유하지 않고 BackBuffer와 FrameResource 재사용을 관리한다.
 */
class FRenderer
{
public:
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

    /** Scene의 Visible Mesh를 그린 뒤 현재 BackBuffer를 표시하고 FrameResource 완료 지점을 기록한다. */
    void RenderFrame(const FScene* Scene);

    /**
     * 제출된 GPU 작업을 완료한 뒤 SwapChain BackBuffer를 새 크기로 다시 생성한다.
     * Width와 Height가 0인 최소화 상태는 처리하지 않는다.
     */
    bool Resize(std::uint32_t Width, std::uint32_t Height);

    /** 제출된 GPU 작업을 완료한 뒤 렌더링 자원을 의존 관계의 역순으로 해제한다. */
    void Shutdown();

private:
    FD3D12Device Device;
    FD3D12CommandContext CommandContext;
    FDXGISwapChain SwapChain;
    std::array<FFrameResource, FDXGISwapChain::BufferCount> FrameResources;
    FMeshPassPipeline MeshPassPipeline;
    std::uint32_t ViewportWidth = 0;
    std::uint32_t ViewportHeight = 0;
    bool bInitialized = false;
};
