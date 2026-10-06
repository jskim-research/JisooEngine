#pragma once

#include "Runtime/Render/Mesh/MeshPass.h"

/** Opaque MeshBatch를 현재 Frame의 D3D12 실행 상태로 변환한다. */
class FOpaqueMeshPassProcessor final : public FMeshPassProcessor
{
public:
    explicit FOpaqueMeshPassProcessor(ID3D12PipelineState* InPipelineState);

    /** 처리 가능한 Batch이면 정점을 FrameResource에 업로드하고 DrawCommand를 추가한다. */
    bool AddMeshBatch(
        const FMeshBatch& MeshBatch,
        const FMeshPassProcessorContext& Context,
        std::vector<FMeshDrawCommand>& DrawCommands) const override;

private:
    ID3D12PipelineState* PipelineState = nullptr;
};
