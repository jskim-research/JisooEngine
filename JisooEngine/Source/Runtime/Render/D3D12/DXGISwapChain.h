#pragma once

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <array>
#include <cstdint>

/**
 * Win32 Window에 연결된 Flip Model SwapChain과 BackBuffer RTV를 소유한다.
 * Present 이후 DXGI가 선택한 현재 BackBuffer Index를 추적한다.
 */
class FDXGISwapChain
{
public:
    static constexpr std::uint32_t BufferCount = 2;

    FDXGISwapChain() = default;
    ~FDXGISwapChain();

    FDXGISwapChain(const FDXGISwapChain&) = delete;
    FDXGISwapChain& operator=(const FDXGISwapChain&) = delete;

    /** 지정한 Window와 Direct CommandQueue에 연결된 Double-buffered SwapChain을 생성한다. */
    bool Initialize(
        IDXGIFactory6* Factory,
        ID3D12Device* Device,
        ID3D12CommandQueue* CommandQueue,
        HWND WindowHandle,
        std::uint32_t Width,
        std::uint32_t Height);

    /**
     * 기존 BackBuffer 참조를 해제하고 새 크기의 Buffer와 RTV를 생성한다.
     * @warning 호출 전에 BackBuffer를 사용하는 GPU 작업이 모두 완료되어야 한다.
     */
    bool Resize(ID3D12Device* Device, std::uint32_t Width, std::uint32_t Height);
    void Shutdown();

    /** VSync Interval 1로 현재 BackBuffer를 표시하고 다음 BackBuffer Index를 갱신한다. */
    bool Present();

    std::uint32_t GetCurrentBackBufferIndex() const noexcept;
    ID3D12Resource* GetCurrentBackBuffer() const noexcept;
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const noexcept;

private:
    bool CreateRenderTargets(ID3D12Device* Device);
    void ReleaseRenderTargets();

    Microsoft::WRL::ComPtr<IDXGISwapChain4> SwapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, BufferCount> BackBuffers;
    std::uint32_t RenderTargetViewDescriptorSize = 0;
    std::uint32_t CurrentBackBufferIndex = 0;
};
