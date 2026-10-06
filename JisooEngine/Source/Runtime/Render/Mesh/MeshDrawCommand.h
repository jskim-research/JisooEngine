#pragma once

#include "Runtime/Core/Math/MathTypes.h"

#include <d3d12.h>

#include <cstdint>

struct FMeshDrawConstants
{
    FMatrix WorldViewProjection = FMatrix::Identity();
    FLinearColor BaseColor{1.0f, 1.0f, 1.0f, 1.0f};
};

static_assert(sizeof(FMeshDrawConstants) == sizeof(float) * 20);

/** Pass Processor가 확정하고 Renderer가 CommandList에 기록할 단일 Mesh Draw 상태이다. */
struct FMeshDrawCommand
{
    ID3D12PipelineState* PipelineState = nullptr;
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView{};
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology =
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    FMeshDrawConstants Constants;
    std::uint32_t VertexCount = 0;
    std::uint32_t StartVertex = 0;
    std::uint64_t SortKey = 0;
};
