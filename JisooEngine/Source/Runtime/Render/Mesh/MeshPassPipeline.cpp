#include "Runtime/Render/Mesh/MeshPassPipeline.h"

#include "Runtime/Render/Mesh/MeshBatch.h"
#include "Runtime/Render/Mesh/OpaqueMeshPass.h"

FMeshPassPipeline::FMeshPassPipeline()
{
    MeshPasses.push_back(std::make_unique<FOpaqueMeshPass>());
}

FMeshPassPipeline::~FMeshPassPipeline()
{
    Shutdown();
}

bool FMeshPassPipeline::Initialize(
    const FMeshPassInitializationContext& Context)
{
    if (bInitialized)
    {
        return true;
    }

    std::size_t InitializedPassCount = 0;
    for (const std::unique_ptr<FMeshPass>& MeshPass : MeshPasses)
    {
        if (!MeshPass->Initialize(Context))
        {
            while (InitializedPassCount > 0)
            {
                --InitializedPassCount;
                MeshPasses[InitializedPassCount]->Shutdown();
            }
            return false;
        }
        ++InitializedPassCount;
    }

    bInitialized = true;
    return true;
}

void FMeshPassPipeline::Shutdown()
{
    if (!bInitialized)
    {
        return;
    }

    for (auto Iterator = MeshPasses.rbegin(); Iterator != MeshPasses.rend(); ++Iterator)
    {
        (*Iterator)->Shutdown();
    }
    bInitialized = false;
}

FMeshPassMask FMeshPassPipeline::ComputePassMask(
    const FMeshBatch& MeshBatch) const
{
    FMeshPassMask PassMask;
    for (const std::unique_ptr<FMeshPass>& MeshPass : MeshPasses)
    {
        if (MeshPass->IsRelevant(MeshBatch))
        {
            PassMask.Add(MeshPass->GetType());
        }
    }
    return PassMask;
}

void FMeshPassPipeline::Execute(
    const FMeshBatchCollector& Collector,
    const FMeshPassPipelineExecutionContext& Context) const
{
    if (!bInitialized)
    {
        return;
    }

    std::vector<FCollectedMeshBatch> CollectedMeshes;
    CollectedMeshes.reserve(Collector.Num());
    for (const FMeshBatch& MeshBatch : Collector.GetMeshBatches())
    {
        const FMeshPassMask PassMask = ComputePassMask(MeshBatch);
        if (!PassMask.IsEmpty())
        {
            CollectedMeshes.push_back({MeshBatch, PassMask});
        }
    }

    const FMeshPassExecutionContext PassContext{
        Context.CommandList,
        Context.FrameResource,
        Context.RenderTargetView,
        Context.ViewportX,
        Context.ViewportY,
        Context.ViewportWidth,
        Context.ViewportHeight,
        Context.ViewProjection,
        CollectedMeshes};
    for (const std::unique_ptr<FMeshPass>& MeshPass : MeshPasses)
    {
        MeshPass->Execute(PassContext);
    }
}
