#pragma once

#include "Runtime/Render/Mesh/MeshPass.h"

#include <wrl/client.h>

#include <memory>

class FOpaqueMeshPassProcessor;

/** Opaque Raster Pass의 영구 GPU 자원, relevance와 Pass 시작 상태를 소유한다. */
class FOpaqueMeshPass final : public FMeshPass
{
public:
    FOpaqueMeshPass();
    ~FOpaqueMeshPass() override;

    EMeshPass GetType() const override
    {
        return EMeshPass::Opaque;
    }

    bool Initialize(const FMeshPassInitializationContext& Context) override;
    void Shutdown() override;
    bool IsRelevant(const FMeshBatch& MeshBatch) const override;

protected:
    const FMeshPassProcessor* GetProcessor() const override;
    void BindPassState(const FMeshPassExecutionContext& Context) const override;

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;
    std::unique_ptr<FOpaqueMeshPassProcessor> Processor;
};
