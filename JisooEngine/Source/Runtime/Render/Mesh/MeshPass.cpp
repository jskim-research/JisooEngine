#include "Runtime/Render/Mesh/MeshPass.h"

#include <algorithm>

void FMeshPass::Execute(const FMeshPassExecutionContext& Context) const
{
    const FMeshPassProcessor* Processor = GetProcessor();
    if (Processor == nullptr)
    {
        return;
    }

    std::vector<FMeshDrawCommand> DrawCommands;
    DrawCommands.reserve(Context.MeshBatches.size());

    const FMeshPassProcessorContext ProcessorContext{
        Context.FrameResource,
        Context.ViewProjection};
    for (const FCollectedMeshBatch& CollectedMesh : Context.MeshBatches)
    {
        if (CollectedMesh.PassMask.Contains(GetType()))
        {
            Processor->AddMeshBatch(
                CollectedMesh.MeshBatch,
                ProcessorContext,
                DrawCommands);
        }
    }

    std::ranges::sort(
        DrawCommands,
        {},
        &FMeshDrawCommand::SortKey);

    BindPassState(Context);

    // Pass 경계마다 바인딩 캐시를 비워 이전 Pass의 PSO 상태를 재사용하지 않는다.
    ID3D12PipelineState* BoundPipelineState = nullptr;
    for (const FMeshDrawCommand& DrawCommand : DrawCommands)
    {
        if (BoundPipelineState != DrawCommand.PipelineState)
        {
            Context.CommandList.SetPipelineState(DrawCommand.PipelineState);
            BoundPipelineState = DrawCommand.PipelineState;
        }

        Context.CommandList.SetGraphicsRoot32BitConstants(
            0,
            sizeof(FMeshDrawConstants) / sizeof(std::uint32_t),
            &DrawCommand.Constants,
            0);
        Context.CommandList.IASetVertexBuffers(
            0,
            1,
            &DrawCommand.VertexBufferView);
        Context.CommandList.IASetPrimitiveTopology(
            DrawCommand.PrimitiveTopology);
        Context.CommandList.DrawInstanced(
            DrawCommand.VertexCount,
            1,
            DrawCommand.StartVertex,
            0);
    }
}
