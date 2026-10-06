#include "Runtime/Render/Renderer.h"

#include "Runtime/Core/Math/MathTypes.h"
#include "Runtime/Render/Mesh/MeshBatch.h"
#include "Runtime/Render/Mesh/MeshBatchCollection.h"
#include "Runtime/Render/Mesh/MeshDrawCommand.h"
#include "Runtime/Render/Mesh/MeshPass.h"
#include "Runtime/Render/Mesh/OpaqueMeshPassProcessor.h"
#include "Runtime/Render/Scene/Scene.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <numbers>
#include <vector>

namespace
{
    FMatrix MakeFixedViewProjection(std::uint32_t Width, std::uint32_t Height)
    {
        FMatrix View{};
        View.M[0][2] = 1.0f;
        View.M[1][0] = 1.0f;
        View.M[2][1] = 1.0f;
        View.M[3][3] = 1.0f;

        constexpr float FieldOfViewDegrees = 60.0f;
        constexpr float NearPlane = 1.0f;
        constexpr float FarPlane = 100000.0f;
        const float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
        const float FieldOfViewRadians =
            FieldOfViewDegrees * std::numbers::pi_v<float> / 180.0f;
        const float YScale = 1.0f / std::tan(FieldOfViewRadians * 0.5f);
        const float XScale = YScale / AspectRatio;

        FMatrix Projection{};
        Projection.M[0][0] = XScale;
        Projection.M[1][1] = YScale;
        Projection.M[2][2] = FarPlane / (FarPlane - NearPlane);
        Projection.M[2][3] = 1.0f;
        Projection.M[3][2] = -NearPlane * FarPlane / (FarPlane - NearPlane);
        return View * Projection;
    }

    bool CompileShader(
        const std::filesystem::path& ShaderPath,
        const char* EntryPoint,
        const char* Target,
        Microsoft::WRL::ComPtr<ID3DBlob>& ShaderBytecode)
    {
        UINT CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        Microsoft::WRL::ComPtr<ID3DBlob> Errors;
        const HRESULT CompileResult = D3DCompileFromFile(
            ShaderPath.c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            EntryPoint,
            Target,
            CompileFlags,
            0,
            &ShaderBytecode,
            &Errors);

        if (Errors != nullptr)
        {
            OutputDebugStringA(static_cast<const char*>(Errors->GetBufferPointer()));
        }
        return SUCCEEDED(CompileResult);
    }
}

FRenderer::~FRenderer()
{
    Shutdown();
}

bool FRenderer::Initialize(
    void* NativeWindowHandle,
    std::uint32_t Width,
    std::uint32_t Height)
{
    if (bInitialized)
    {
        return true;
    }

    if (NativeWindowHandle == nullptr || Width == 0 || Height == 0 ||
        !Device.Initialize())
    {
        Shutdown();
        return false;
    }

    for (FFrameResource& FrameResource : FrameResources)
    {
        if (!FrameResource.Initialize(Device.GetDevice()))
        {
            Shutdown();
            return false;
        }
    }

    // CommandList 생성에는 Allocator가 필요하므로 첫 FrameResource의 Allocator를 초기 생성에 사용한다.
    if (!CommandContext.Initialize(
            Device.GetDevice(),
            FrameResources[0].GetCommandAllocator()) ||
        !SwapChain.Initialize(
            Device.GetFactory(),
            Device.GetDevice(),
            CommandContext.GetCommandQueue(),
            static_cast<HWND>(NativeWindowHandle),
            Width,
            Height) ||
        !InitializeOpaquePass())
    {
        Shutdown();
        return false;
    }

    ViewportWidth = Width;
    ViewportHeight = Height;
    bInitialized = true;
    return true;
}

void FRenderer::RenderFrame(const FScene* Scene)
{
    if (!bInitialized)
    {
        return;
    }

    const std::uint32_t FrameIndex = SwapChain.GetCurrentBackBufferIndex();
    FFrameResource& FrameResource = FrameResources[FrameIndex];
    if (!CommandContext.BeginFrame(FrameResource))
    {
        return;
    }

    ID3D12GraphicsCommandList* CommandList = CommandContext.GetCommandList();
    ID3D12Resource* BackBuffer = SwapChain.GetCurrentBackBuffer();
    const D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView =
        SwapChain.GetCurrentRenderTargetView();

    CommandContext.TransitionResource(
        BackBuffer,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    CommandList->OMSetRenderTargets(1, &RenderTargetView, FALSE, nullptr);
    constexpr float ClearColor[] = {0.035f, 0.055f, 0.085f, 1.0f};
    CommandList->ClearRenderTargetView(RenderTargetView, ClearColor, 0, nullptr);

    D3D12_VIEWPORT Viewport{};
    Viewport.Width = static_cast<float>(ViewportWidth);
    Viewport.Height = static_cast<float>(ViewportHeight);
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    CommandList->RSSetViewports(1, &Viewport);

    D3D12_RECT ScissorRect{};
    ScissorRect.right = static_cast<LONG>(ViewportWidth);
    ScissorRect.bottom = static_cast<LONG>(ViewportHeight);
    CommandList->RSSetScissorRects(1, &ScissorRect);

    if (Scene != nullptr)
    {
        FMeshBatchCollector Collector;
        GatherVisibleMeshBatches(*Scene, Collector);

        std::vector<FMeshDrawCommand> DrawCommands;
        DrawCommands.reserve(Collector.Num());
        const FMatrix ViewProjection = MakeFixedViewProjection(
            ViewportWidth,
            ViewportHeight);
        FOpaqueMeshPassProcessor OpaqueProcessor(
            OpaquePipelineState.Get(),
            FrameResource,
            ViewProjection);

        for (const FMeshBatch& MeshBatch : Collector.GetMeshBatches())
        {
            const FMeshPassMask PassMask = ComputeMeshPassMask(MeshBatch);
            if (PassMask.Contains(EMeshPass::Opaque))
            {
                OpaqueProcessor.AddMeshBatch(MeshBatch, DrawCommands);
            }
        }

        std::ranges::sort(
            DrawCommands,
            {},
            &FMeshDrawCommand::SortKey);

        CommandList->SetGraphicsRootSignature(OpaqueRootSignature.Get());
        ID3D12PipelineState* BoundPipelineState = nullptr;
        for (const FMeshDrawCommand& DrawCommand : DrawCommands)
        {
            if (BoundPipelineState != DrawCommand.PipelineState)
            {
                CommandList->SetPipelineState(DrawCommand.PipelineState);
                BoundPipelineState = DrawCommand.PipelineState;
            }

            CommandList->SetGraphicsRoot32BitConstants(
                0,
                sizeof(FMeshDrawConstants) / sizeof(std::uint32_t),
                &DrawCommand.Constants,
                0);
            CommandList->IASetVertexBuffers(0, 1, &DrawCommand.VertexBufferView);
            CommandList->IASetPrimitiveTopology(DrawCommand.PrimitiveTopology);
            CommandList->DrawInstanced(
                DrawCommand.VertexCount,
                1,
                DrawCommand.StartVertex,
                0);
        }
    }

    CommandContext.TransitionResource(
        BackBuffer,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    if (!CommandContext.ExecuteCommandList())
    {
        return;
    }

    const bool bPresented = SwapChain.Present();
    const bool bSignaled = CommandContext.Signal(FrameResource);
    if (!bPresented || !bSignaled)
    {
        return;
    }
}

bool FRenderer::Resize(std::uint32_t Width, std::uint32_t Height)
{
    if (!bInitialized || Width == 0 || Height == 0)
    {
        return false;
    }

    // ResizeBuffers 전에 BackBuffer를 참조하는 GPU 작업과 CPU 소유 참조를 모두 끝내야 한다.
    if (!CommandContext.WaitForGpu())
    {
        return false;
    }

    for (FFrameResource& FrameResource : FrameResources)
    {
        FrameResource.SetFenceValue(0);
    }

    if (!SwapChain.Resize(Device.GetDevice(), Width, Height))
    {
        return false;
    }

    ViewportWidth = Width;
    ViewportHeight = Height;
    return true;
}

void FRenderer::Shutdown()
{
    // GPU가 참조할 수 있는 자원을 해제하기 전에 제출된 작업을 모두 완료한다.
    CommandContext.WaitForGpu();

    ShutdownOpaquePass();
    SwapChain.Shutdown();
    CommandContext.Shutdown();
    for (FFrameResource& FrameResource : FrameResources)
    {
        FrameResource.Shutdown();
    }
    Device.Shutdown();

    ViewportWidth = 0;
    ViewportHeight = 0;
    bInitialized = false;
}

bool FRenderer::InitializeOpaquePass()
{
    if (OpaquePipelineState != nullptr)
    {
        return true;
    }

    ID3D12Device* D3DDevice = Device.GetDevice();
    if (D3DDevice == nullptr)
    {
        return false;
    }

    D3D12_ROOT_PARAMETER RootParameter{};
    RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    RootParameter.Constants.ShaderRegister = 0;
    RootParameter.Constants.RegisterSpace = 0;
    RootParameter.Constants.Num32BitValues =
        sizeof(FMeshDrawConstants) / sizeof(std::uint32_t);
    RootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC RootSignatureDescription{};
    RootSignatureDescription.NumParameters = 1;
    RootSignatureDescription.pParameters = &RootParameter;
    RootSignatureDescription.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> SerializedRootSignature;
    Microsoft::WRL::ComPtr<ID3DBlob> RootSignatureErrors;
    if (FAILED(D3D12SerializeRootSignature(
            &RootSignatureDescription,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &SerializedRootSignature,
            &RootSignatureErrors)))
    {
        if (RootSignatureErrors != nullptr)
        {
            OutputDebugStringA(static_cast<const char*>(
                RootSignatureErrors->GetBufferPointer()));
        }
        return false;
    }

    if (FAILED(D3DDevice->CreateRootSignature(
            0,
            SerializedRootSignature->GetBufferPointer(),
            SerializedRootSignature->GetBufferSize(),
            IID_PPV_ARGS(&OpaqueRootSignature))))
    {
        ShutdownOpaquePass();
        return false;
    }

    const std::filesystem::path ShaderPath =
        std::filesystem::path(JISOO_ENGINE_SHADER_DIRECTORY) /
        L"DefaultMesh.hlsl";
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader;
    if (!CompileShader(ShaderPath, "VSMain", "vs_5_1", VertexShader) ||
        !CompileShader(ShaderPath, "PSMain", "ps_5_1", PixelShader))
    {
        ShutdownOpaquePass();
        return false;
    }

    D3D12_INPUT_ELEMENT_DESC InputElements[] = {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0}};

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDescription{};
    PipelineDescription.pRootSignature = OpaqueRootSignature.Get();
    PipelineDescription.VS = {
        VertexShader->GetBufferPointer(),
        VertexShader->GetBufferSize()};
    PipelineDescription.PS = {
        PixelShader->GetBufferPointer(),
        PixelShader->GetBufferSize()};
    PipelineDescription.BlendState.AlphaToCoverageEnable = FALSE;
    PipelineDescription.BlendState.IndependentBlendEnable = FALSE;
    PipelineDescription.BlendState.RenderTarget[0].BlendEnable = FALSE;
    PipelineDescription.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
    PipelineDescription.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    PipelineDescription.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    PipelineDescription.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    PipelineDescription.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    PipelineDescription.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    PipelineDescription.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    PipelineDescription.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    PipelineDescription.BlendState.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    PipelineDescription.SampleMask = UINT_MAX;
    PipelineDescription.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    PipelineDescription.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    PipelineDescription.RasterizerState.FrontCounterClockwise = FALSE;
    PipelineDescription.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    PipelineDescription.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    PipelineDescription.RasterizerState.SlopeScaledDepthBias =
        D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    PipelineDescription.RasterizerState.DepthClipEnable = TRUE;
    PipelineDescription.RasterizerState.MultisampleEnable = FALSE;
    PipelineDescription.RasterizerState.AntialiasedLineEnable = FALSE;
    PipelineDescription.RasterizerState.ForcedSampleCount = 0;
    PipelineDescription.RasterizerState.ConservativeRaster =
        D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    PipelineDescription.DepthStencilState.DepthEnable = FALSE;
    PipelineDescription.DepthStencilState.DepthWriteMask =
        D3D12_DEPTH_WRITE_MASK_ZERO;
    PipelineDescription.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    PipelineDescription.DepthStencilState.StencilEnable = FALSE;
    PipelineDescription.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    PipelineDescription.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    PipelineDescription.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    PipelineDescription.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    PipelineDescription.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    PipelineDescription.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    PipelineDescription.DepthStencilState.BackFace =
        PipelineDescription.DepthStencilState.FrontFace;
    PipelineDescription.InputLayout = {InputElements, 1};
    PipelineDescription.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    PipelineDescription.NumRenderTargets = 1;
    PipelineDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    PipelineDescription.SampleDesc.Count = 1;

    if (FAILED(D3DDevice->CreateGraphicsPipelineState(
            &PipelineDescription,
            IID_PPV_ARGS(&OpaquePipelineState))))
    {
        ShutdownOpaquePass();
        return false;
    }
    return true;
}

void FRenderer::ShutdownOpaquePass()
{
    OpaquePipelineState.Reset();
    OpaqueRootSignature.Reset();
}
