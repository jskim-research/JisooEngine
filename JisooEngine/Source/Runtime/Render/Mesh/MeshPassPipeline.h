#pragma once

#include "Runtime/Core/Math/MathTypes.h"
#include "Runtime/Render/Mesh/MeshPass.h"

#include <d3d12.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class FFrameResource;
class FMeshBatchCollector;

struct FMeshPassPipelineExecutionContext
{
    ID3D12GraphicsCommandList& CommandList;
    FFrameResource& FrameResource;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView{};
    std::uint32_t ViewportWidth = 0;
    std::uint32_t ViewportHeight = 0;
    FMatrix ViewProjection = FMatrix::Identity();
};

/**
 * Raster Mesh Pass의 등록 순서, 생명주기와 프레임 실행을 관리한다.
 * 등록 순서가 실행 순서이며 Renderer는 구체 Pass 타입을 알지 않는다.
 */
class FMeshPassPipeline
{
public:
    FMeshPassPipeline();
    ~FMeshPassPipeline();

    FMeshPassPipeline(const FMeshPassPipeline&) = delete;
    FMeshPassPipeline& operator=(const FMeshPassPipeline&) = delete;

    /** 등록된 Pass를 실행 순서대로 초기화하며, 실패하면 앞서 초기화한 Pass도 정리한다. */
    bool Initialize(const FMeshPassInitializationContext& Context);

    /** 등록된 Pass를 초기화의 역순으로 종료한다. */
    void Shutdown();

    /** 각 Pass에 relevance를 질의해 하나의 Batch가 참여할 PassMask를 계산한다. */
    [[nodiscard]] FMeshPassMask ComputePassMask(const FMeshBatch& MeshBatch) const;

    /** Visible Batch를 Pass별 후보로 분류한 뒤 등록 순서대로 Pass를 실행한다. */
    void Execute(
        const FMeshBatchCollector& Collector,
        const FMeshPassPipelineExecutionContext& Context) const;

    [[nodiscard]] std::size_t NumPasses() const
    {
        return MeshPasses.size();
    }

    [[nodiscard]] EMeshPass GetPassType(std::size_t Index) const
    {
        return MeshPasses[Index]->GetType();
    }

private:
    std::vector<std::unique_ptr<FMeshPass>> MeshPasses;
    bool bInitialized = false;
};
