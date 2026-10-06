#pragma once

#include "Runtime/Render/Mesh/MeshBatch.h"
#include "Runtime/Render/Scene/PrimitiveSceneProxy.h"

#include <array>

/** CPU 정점 세 개와 단색 Material 입력을 보관하고 MeshBatch로 제출한다. */
class FTriangleSceneProxy final : public FPrimitiveSceneProxy
{
public:
    explicit FTriangleSceneProxy(const FPrimitiveSceneDescription& Description);

    void GatherMeshBatches(FMeshBatchCollector& Collector) const override;

private:
    std::array<FMeshVertex, 3> Vertices;
    FMeshMaterialData Material;
};
