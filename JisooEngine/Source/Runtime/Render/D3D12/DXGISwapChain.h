#pragma once

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <array>
#include <cstdint>

class FDXGISwapChain
{
public:
    static constexpr std::uint32_t BufferCount = 2;

    FDXGISwapChain() = default;
    ~FDXGISwapChain();

    FDXGISwapChain(const FDXGISwapChain&) = delete;
    FDXGISwapChain& operator=(const FDXGISwapChain&) = delete;

    bool Initialize(
        IDXGIFactory6* Factory,
        ID3D12Device* Device,
        ID3D12CommandQueue* CommandQueue,
        HWND WindowHandle,
        std::uint32_t Width,
        std::uint32_t Height);
    bool Resize(ID3D12Device* Device, std::uint32_t Width, std::uint32_t Height);
    void Shutdown();

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
