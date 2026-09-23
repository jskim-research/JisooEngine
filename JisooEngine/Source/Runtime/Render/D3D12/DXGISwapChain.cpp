#include "Runtime/Render/D3D12/DXGISwapChain.h"

FDXGISwapChain::~FDXGISwapChain()
{
    Shutdown();
}

bool FDXGISwapChain::Initialize(
    IDXGIFactory6* Factory,
    ID3D12Device* Device,
    ID3D12CommandQueue* CommandQueue,
    HWND WindowHandle,
    std::uint32_t Width,
    std::uint32_t Height)
{
    if (SwapChain != nullptr)
    {
        return true;
    }

    if (Factory == nullptr || Device == nullptr || CommandQueue == nullptr ||
        WindowHandle == nullptr || Width == 0 || Height == 0)
    {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 SwapChainDescription{};
    SwapChainDescription.Width = Width;
    SwapChainDescription.Height = Height;
    SwapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.Stereo = FALSE;
    SwapChainDescription.SampleDesc.Count = 1;
    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.BufferCount = BufferCount;
    SwapChainDescription.Scaling = DXGI_SCALING_STRETCH;
    SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> BaseSwapChain;
    if (FAILED(Factory->CreateSwapChainForHwnd(
            CommandQueue,
            WindowHandle,
            &SwapChainDescription,
            nullptr,
            nullptr,
            &BaseSwapChain)) ||
        FAILED(Factory->MakeWindowAssociation(WindowHandle, DXGI_MWA_NO_ALT_ENTER)) ||
        FAILED(BaseSwapChain.As(&SwapChain)))
    {
        Shutdown();
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC HeapDescription{};
    HeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HeapDescription.NumDescriptors = BufferCount;
    HeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(Device->CreateDescriptorHeap(
            &HeapDescription,
            IID_PPV_ARGS(&RenderTargetViewHeap))))
    {
        Shutdown();
        return false;
    }

    RenderTargetViewDescriptorSize = Device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();

    if (!CreateRenderTargets(Device))
    {
        Shutdown();
        return false;
    }

    return true;
}

bool FDXGISwapChain::Resize(
    ID3D12Device* Device,
    std::uint32_t Width,
    std::uint32_t Height)
{
    if (Device == nullptr || SwapChain == nullptr || Width == 0 || Height == 0)
    {
        return false;
    }

    ReleaseRenderTargets();

    if (FAILED(SwapChain->ResizeBuffers(
            BufferCount,
            Width,
            Height,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            0)))
    {
        return false;
    }

    CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
    return CreateRenderTargets(Device);
}

void FDXGISwapChain::Shutdown()
{
    ReleaseRenderTargets();
    RenderTargetViewHeap.Reset();
    SwapChain.Reset();
    RenderTargetViewDescriptorSize = 0;
    CurrentBackBufferIndex = 0;
}

bool FDXGISwapChain::Present()
{
    if (SwapChain == nullptr || FAILED(SwapChain->Present(1, 0)))
    {
        return false;
    }

    CurrentBackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
    return true;
}

std::uint32_t FDXGISwapChain::GetCurrentBackBufferIndex() const noexcept
{
    return CurrentBackBufferIndex;
}

ID3D12Resource* FDXGISwapChain::GetCurrentBackBuffer() const noexcept
{
    return BackBuffers[CurrentBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE FDXGISwapChain::GetCurrentRenderTargetView() const noexcept
{
    D3D12_CPU_DESCRIPTOR_HANDLE Handle =
        RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
    Handle.ptr += static_cast<SIZE_T>(CurrentBackBufferIndex) *
        RenderTargetViewDescriptorSize;
    return Handle;
}

bool FDXGISwapChain::CreateRenderTargets(ID3D12Device* Device)
{
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle =
        RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();

    for (std::uint32_t BufferIndex = 0; BufferIndex < BufferCount; ++BufferIndex)
    {
        if (FAILED(SwapChain->GetBuffer(
                BufferIndex,
                IID_PPV_ARGS(&BackBuffers[BufferIndex]))))
        {
            ReleaseRenderTargets();
            return false;
        }

        Device->CreateRenderTargetView(
            BackBuffers[BufferIndex].Get(),
            nullptr,
            RenderTargetViewHandle);
        RenderTargetViewHandle.ptr += RenderTargetViewDescriptorSize;
    }

    return true;
}

void FDXGISwapChain::ReleaseRenderTargets()
{
    for (Microsoft::WRL::ComPtr<ID3D12Resource>& BackBuffer : BackBuffers)
    {
        BackBuffer.Reset();
    }
}
