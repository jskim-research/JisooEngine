#pragma once

#include "Runtime/Core/Math/MathTypes.h"

#include <cstddef>
#include <span>
#include <vector>

enum class EMeshBlendMode
{
    Opaque
};

enum class EMeshPrimitiveTopology
{
    TriangleList
};

struct FMeshVertex
{
    FVector Position;
};

struct FMeshMaterialData
{
    EMeshBlendMode BlendMode = EMeshBlendMode::Opaque;
    FLinearColor BaseColor{1.0f, 1.0f, 1.0f, 1.0f};
};

/**
 * Proxy가 한 프레임 동안 제출하는 Pass 독립적인 Mesh Draw 후보이다.
 * Vertices는 원본 Proxy가 소유하며 Collector와 Processor 사용이 끝날 때까지 유효해야 한다.
 */
struct FMeshBatch
{
    std::span<const FMeshVertex> Vertices;
    FMatrix LocalToWorld = FMatrix::Identity();
    FMeshMaterialData Material;
    EMeshPrimitiveTopology PrimitiveTopology = EMeshPrimitiveTopology::TriangleList;
};

/** Proxy가 제출한 MeshBatch를 현재 RenderFrame 동안만 보관한다. */
class FMeshBatchCollector
{
public:
    void AddMesh(FMeshBatch Batch)
    {
        MeshBatches.push_back(Batch);
    }

    [[nodiscard]] std::span<const FMeshBatch> GetMeshBatches() const
    {
        return MeshBatches;
    }

    [[nodiscard]] std::size_t Num() const
    {
        return MeshBatches.size();
    }

private:
    std::vector<FMeshBatch> MeshBatches;
};
