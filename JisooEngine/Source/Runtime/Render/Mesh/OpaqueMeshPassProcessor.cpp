#include "Runtime/Render/Mesh/OpaqueMeshPassProcessor.h"

#include "Runtime/Render/D3D12/FrameResource.h"
#include "Runtime/Render/Mesh/MeshBatch.h"

#include <cstring>
#include <limits>

FOpaqueMeshPassProcessor::FOpaqueMeshPassProcessor(
    ID3D12PipelineState* InPipelineState)
    : PipelineState(InPipelineState)
{
}

bool FOpaqueMeshPassProcessor::AddMeshBatch(
    const FMeshBatch& MeshBatch,
    const FMeshPassProcessorContext& Context,
    std::vector<FMeshDrawCommand>& DrawCommands) const
{
    if (PipelineState == nullptr || MeshBatch.Vertices.empty() ||
        MeshBatch.Material.BlendMode != EMeshBlendMode::Opaque ||
        MeshBatch.PrimitiveTopology != EMeshPrimitiveTopology::TriangleList ||
        MeshBatch.Vertices.size() > (std::numeric_limits<std::uint32_t>::max)())
    {
        return false;
    }

    const std::size_t VertexDataSize = MeshBatch.Vertices.size_bytes();
    const FD3D12UploadAllocation VertexAllocation =
        Context.FrameResource.GetDynamicUploadBuffer().Allocate(
            VertexDataSize,
            alignof(FMeshVertex));
    if (!VertexAllocation.IsValid() ||
        VertexDataSize > (std::numeric_limits<UINT>::max)())
    {
        return false;
    }

    std::memcpy(
        VertexAllocation.CpuAddress,
        MeshBatch.Vertices.data(),
        VertexDataSize);

    FMeshDrawCommand DrawCommand;
    DrawCommand.PipelineState = PipelineState;
    DrawCommand.VertexBufferView.BufferLocation = VertexAllocation.GpuAddress;
    DrawCommand.VertexBufferView.SizeInBytes = static_cast<UINT>(VertexDataSize);
    DrawCommand.VertexBufferView.StrideInBytes = sizeof(FMeshVertex);
    DrawCommand.PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    DrawCommand.Constants.WorldViewProjection =
        MeshBatch.LocalToWorld * Context.ViewProjection;
    DrawCommand.Constants.BaseColor = MeshBatch.Material.BaseColor;
    DrawCommand.VertexCount = static_cast<std::uint32_t>(MeshBatch.Vertices.size());
    DrawCommands.push_back(DrawCommand);
    return true;
}
