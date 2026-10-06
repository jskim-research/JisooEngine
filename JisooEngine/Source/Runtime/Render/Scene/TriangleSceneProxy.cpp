#include "Runtime/Render/Scene/TriangleSceneProxy.h"

FTriangleSceneProxy::FTriangleSceneProxy(const FPrimitiveSceneDescription& Description)
    : FPrimitiveSceneProxy(Description)
    , Vertices{
        FMeshVertex{{0.0f, -50.0f, -50.0f}},
        FMeshVertex{{0.0f, 50.0f, -50.0f}},
        FMeshVertex{{0.0f, 0.0f, 50.0f}}}
{
    Material.BlendMode = EMeshBlendMode::Opaque;
    Material.BaseColor = {0.95f, 0.25f, 0.12f, 1.0f};
}

void FTriangleSceneProxy::GatherMeshBatches(FMeshBatchCollector& Collector) const
{
    FMeshBatch MeshBatch;
    MeshBatch.Vertices = Vertices;
    MeshBatch.LocalToWorld = GetLocalToWorld();
    MeshBatch.Material = Material;
    MeshBatch.PrimitiveTopology = EMeshPrimitiveTopology::TriangleList;
    Collector.AddMesh(MeshBatch);
}
