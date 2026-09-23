#include "Runtime/Render/D3D12/D3D12Device.h"

FD3D12Device::~FD3D12Device()
{
    Shutdown();
}

bool FD3D12Device::Initialize()
{
    if (Device != nullptr)
    {
        return true;
    }

    UINT FactoryFlags = 0;

#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
    {
        DebugController->EnableDebugLayer();
        FactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    if (FAILED(CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory))))
    {
        Shutdown();
        return false;
    }

    for (UINT AdapterIndex = 0; ; ++AdapterIndex)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> CandidateAdapter;
        const HRESULT EnumerateResult = Factory->EnumAdapterByGpuPreference(
            AdapterIndex,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS(&CandidateAdapter));

        if (EnumerateResult == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }

        if (FAILED(EnumerateResult))
        {
            Shutdown();
            return false;
        }

        DXGI_ADAPTER_DESC1 AdapterDescription{};
        if (FAILED(CandidateAdapter->GetDesc1(&AdapterDescription)) ||
            (AdapterDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
        {
            continue;
        }

        Microsoft::WRL::ComPtr<ID3D12Device> CandidateDevice;
        if (SUCCEEDED(D3D12CreateDevice(
                CandidateAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&CandidateDevice))))
        {
            Adapter = CandidateAdapter;
            Device = CandidateDevice;
            return true;
        }
    }

    Shutdown();
    return false;
}

void FD3D12Device::Shutdown()
{
    Device.Reset();
    Adapter.Reset();
    Factory.Reset();
}

IDXGIFactory6* FD3D12Device::GetFactory() const noexcept
{
    return Factory.Get();
}

ID3D12Device* FD3D12Device::GetDevice() const noexcept
{
    return Device.Get();
}
