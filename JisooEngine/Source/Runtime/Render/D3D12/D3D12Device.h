#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

class FD3D12Device
{
public:
    FD3D12Device() = default;
    ~FD3D12Device();

    FD3D12Device(const FD3D12Device&) = delete;
    FD3D12Device& operator=(const FD3D12Device&) = delete;

    bool Initialize();
    void Shutdown();

    IDXGIFactory6* GetFactory() const noexcept;
    ID3D12Device* GetDevice() const noexcept;

private:
    Microsoft::WRL::ComPtr<IDXGIFactory6> Factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter;
    Microsoft::WRL::ComPtr<ID3D12Device> Device;
};
