#pragma once

#include "Runtime/Render/D3D12/D3D12CommandContext.h"
#include "Runtime/Render/D3D12/D3D12Device.h"
#include "Runtime/Render/D3D12/DXGISwapChain.h"
#include "Runtime/Render/D3D12/FrameResource.h"

#include <array>
#include <cstdint>

/**
 * D3D12 렌더링 하위 객체의 생애주기와 프레임 기록·제출 순서를 조정한다.
 * 현재는 BackBuffer Clear와 Present, FrameResource 재사용 동기화까지 담당한다.
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

    /** 현재 BackBuffer를 지우고 화면에 표시한 뒤 FrameResource 완료 지점을 기록한다. */
    void RenderFrame();

    /**
     * 제출된 GPU 작업을 완료한 뒤 SwapChain BackBuffer를 새 크기로 다시 생성한다.
     * Width와 Height가 0인 최소화 상태는 처리하지 않는다.
     */
    bool Resize(std::uint32_t Width, std::uint32_t Height);

    /** 제출된 GPU 작업을 완료한 뒤 렌더링 자원을 의존 관계의 역순으로 해제한다. */
    void Shutdown();

    FD3D12Device Device;
    FD3D12CommandContext CommandContext;
    FDXGISwapChain SwapChain;
    std::array<FFrameResource, FDXGISwapChain::BufferCount> FrameResources;
    bool bInitialized = false;
};
