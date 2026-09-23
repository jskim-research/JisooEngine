#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

/**
 * DXGI Factory와 Hardware Adapter를 선택하고 D3D12 Device를 소유한다.
 * Debug 빌드에서는 Device 생성 전에 D3D12 Debug Layer를 활성화한다.
 */
class FD3D12Device
{
public:
    FD3D12Device() = default;
    ~FD3D12Device();

    FD3D12Device(const FD3D12Device&) = delete;
    FD3D12Device& operator=(const FD3D12Device&) = delete;

    /** High Performance 순서에서 Feature Level 11_0을 지원하는 첫 Hardware Adapter를 선택한다. */
    bool Initialize();
    void Shutdown();

    IDXGIFactory6* GetFactory() const noexcept;
    ID3D12Device* GetDevice() const noexcept;

private:
    Microsoft::WRL::ComPtr<IDXGIFactory6> Factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter;
    Microsoft::WRL::ComPtr<ID3D12Device> Device;
};
