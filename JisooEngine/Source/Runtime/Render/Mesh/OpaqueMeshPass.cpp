#include "Runtime/Render/Mesh/OpaqueMeshPass.h"

#include "Runtime/Render/Mesh/OpaqueMeshPassProcessor.h"

#include <d3dcompiler.h>

#include <filesystem>

namespace
{
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

FOpaqueMeshPass::FOpaqueMeshPass() = default;

FOpaqueMeshPass::~FOpaqueMeshPass()
{
    Shutdown();
}

bool FOpaqueMeshPass::Initialize(
    const FMeshPassInitializationContext& Context)
{
    if (PipelineState != nullptr && Processor != nullptr)
    {
        return true;
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

    if (FAILED(Context.Device.CreateRootSignature(
            0,
            SerializedRootSignature->GetBufferPointer(),
            SerializedRootSignature->GetBufferSize(),
            IID_PPV_ARGS(&RootSignature))))
    {
        Shutdown();
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
        Shutdown();
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
    PipelineDescription.pRootSignature = RootSignature.Get();
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
    PipelineDescription.RasterizerState.DepthBiasClamp =
        D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
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
    PipelineDescription.DepthStencilState.StencilReadMask =
        D3D12_DEFAULT_STENCIL_READ_MASK;
    PipelineDescription.DepthStencilState.StencilWriteMask =
        D3D12_DEFAULT_STENCIL_WRITE_MASK;
    PipelineDescription.DepthStencilState.FrontFace.StencilFailOp =
        D3D12_STENCIL_OP_KEEP;
    PipelineDescription.DepthStencilState.FrontFace.StencilDepthFailOp =
        D3D12_STENCIL_OP_KEEP;
    PipelineDescription.DepthStencilState.FrontFace.StencilPassOp =
        D3D12_STENCIL_OP_KEEP;
    PipelineDescription.DepthStencilState.FrontFace.StencilFunc =
        D3D12_COMPARISON_FUNC_ALWAYS;
    PipelineDescription.DepthStencilState.BackFace =
        PipelineDescription.DepthStencilState.FrontFace;
    PipelineDescription.InputLayout = {InputElements, 1};
    PipelineDescription.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    PipelineDescription.NumRenderTargets = 1;
    PipelineDescription.RTVFormats[0] = Context.RenderTargetFormat;
    PipelineDescription.SampleDesc.Count = 1;

    if (FAILED(Context.Device.CreateGraphicsPipelineState(
            &PipelineDescription,
            IID_PPV_ARGS(&PipelineState))))
    {
        Shutdown();
        return false;
    }

    Processor = std::make_unique<FOpaqueMeshPassProcessor>(PipelineState.Get());
    return true;
}

void FOpaqueMeshPass::Shutdown()
{
    Processor.reset();
    PipelineState.Reset();
    RootSignature.Reset();
}

bool FOpaqueMeshPass::IsRelevant(const FMeshBatch& MeshBatch) const
{
    return MeshBatch.Material.BlendMode == EMeshBlendMode::Opaque;
}

const FMeshPassProcessor* FOpaqueMeshPass::GetProcessor() const
{
    return Processor.get();
}

void FOpaqueMeshPass::BindPassState(
    const FMeshPassExecutionContext& Context) const
{
    Context.CommandList.OMSetRenderTargets(
        1,
        &Context.RenderTargetView,
        FALSE,
        nullptr);

    D3D12_VIEWPORT Viewport{};
    Viewport.Width = static_cast<float>(Context.ViewportWidth);
    Viewport.Height = static_cast<float>(Context.ViewportHeight);
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    Context.CommandList.RSSetViewports(1, &Viewport);

    D3D12_RECT ScissorRect{};
    ScissorRect.right = static_cast<LONG>(Context.ViewportWidth);
    ScissorRect.bottom = static_cast<LONG>(Context.ViewportHeight);
    Context.CommandList.RSSetScissorRects(1, &ScissorRect);
    Context.CommandList.SetGraphicsRootSignature(RootSignature.Get());
}
