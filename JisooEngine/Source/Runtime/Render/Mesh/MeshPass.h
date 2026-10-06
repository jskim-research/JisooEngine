#pragma once

#include "Runtime/Render/Mesh/MeshBatch.h"

#include <cstdint>

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

private:
    [[nodiscard]] static constexpr std::uint32_t GetBit(EMeshPass Pass)
    {
        return 1u << static_cast<std::uint32_t>(Pass);
    }

    std::uint32_t Bits = 0;
};

[[nodiscard]] inline FMeshPassMask ComputeMeshPassMask(const FMeshBatch& MeshBatch)
{
    FMeshPassMask Result;
    if (MeshBatch.Material.BlendMode == EMeshBlendMode::Opaque)
    {
        Result.Add(EMeshPass::Opaque);
    }
    return Result;
}
