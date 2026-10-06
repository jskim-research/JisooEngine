#pragma once

#include "Runtime/Core/Math/MathTypes.h"
#include "Runtime/Render/Mesh/MeshBatch.h"
#include "Runtime/Render/Mesh/MeshDrawCommand.h"

#include <d3d12.h>

#include <cstdint>
#include <span>
#include <vector>

class FFrameResource;

enum class EMeshPass : std::uint8_t
{
    Opaque,
    Count
};

class FMeshPassMask
{
public:
    void Add(EMeshPass Pass)
    {
        Bits |= GetBit(Pass);
    }

    [[nodiscard]] bool Contains(EMeshPass Pass) const
    {
        return (Bits & GetBit(Pass)) != 0;
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return Bits == 0;
    }

private:
    [[nodiscard]] static constexpr std::uint32_t GetBit(EMeshPass Pass)
    {
        return 1u << static_cast<std::uint32_t>(Pass);
    }

    std::uint32_t Bits = 0;
};

struct FCollectedMeshBatch
{
    FMeshBatch MeshBatch;
    FMeshPassMask PassMask;
};

struct FMeshPassInitializationContext
{
    ID3D12Device& Device;
    DXGI_FORMAT RenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
};

struct FMeshPassProcessorContext
{
    FFrameResource& FrameResource;
    FMatrix ViewProjection = FMatrix::Identity();
};

struct FMeshPassExecutionContext
{
    ID3D12GraphicsCommandList& CommandList;
    FFrameResource& FrameResource;
    D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView{};
    std::uint32_t ViewportX = 0;
    std::uint32_t ViewportY = 0;
    std::uint32_t ViewportWidth = 0;
    std::uint32_t ViewportHeight = 0;
    FMatrix ViewProjection = FMatrix::Identity();
    std::span<const FCollectedMeshBatch> MeshBatches;
};

/** Pass별 정책으로 MeshBatch를 GPU 실행이 가능한 DrawCommand로 변환한다. */
class FMeshPassProcessor
{
public:
    virtual ~FMeshPassProcessor() = default;

    /** 현재 Pass에서 처리할 수 있으면 DrawCommand를 추가하고 true를 반환한다. */
    virtual bool AddMeshBatch(
        const FMeshBatch& MeshBatch,
        const FMeshPassProcessorContext& Context,
        std::vector<FMeshDrawCommand>& DrawCommands) const = 0;
};

/**
 * 하나의 Raster Mesh Pass가 소유하는 영구 자원과 프레임 실행 계약이다.
 * Execute는 이전 Pass의 바인딩을 신뢰하지 않고 Pass 상태와 Draw 상태를 다시 설정한다.
 */
class FMeshPass
{
public:
    virtual ~FMeshPass() = default;

    virtual EMeshPass GetType() const = 0;

    /** Device에 종속된 Pass 영구 자원을 생성하며, 실패하면 소유한 부분 자원을 정리한다. */
    virtual bool Initialize(const FMeshPassInitializationContext& Context) = 0;

    /** GPU 사용이 끝난 뒤 Pass가 소유한 영구 자원을 해제한다. */
    virtual void Shutdown() = 0;

    /** Batch가 이 Pass의 Draw 후보인지 빠르게 판정한다. */
    virtual bool IsRelevant(const FMeshBatch& MeshBatch) const = 0;

    /**
     * 관련 Batch를 DrawCommand로 변환하고 정렬한 뒤 CommandList에 기록한다.
     * 호출 전 Pass가 초기화되어 있어야 하며, CommandList의 이전 Pass 바인딩에는 의존하지 않는다.
     */
    void Execute(const FMeshPassExecutionContext& Context) const;

protected:
    virtual const FMeshPassProcessor* GetProcessor() const = 0;

    /** 현재 Pass에 필요한 Pass 단위 GPU 상태를 빠짐없이 설정한다. */
    virtual void BindPassState(const FMeshPassExecutionContext& Context) const = 0;
};
