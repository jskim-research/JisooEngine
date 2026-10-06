#pragma once

#include <cstdint>
#include <limits>

/** Renderer의 RenderTarget 슬롯과 세대를 식별해 만료된 출력 참조를 거부한다. */
struct FRenderTargetHandle
{
    static constexpr std::uint32_t InvalidIndex = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t Index = InvalidIndex;
    std::uint32_t Generation = 0;

    [[nodiscard]] bool IsSet() const
    {
        return Index != InvalidIndex && Generation != 0;
    }

    friend bool operator==(const FRenderTargetHandle&, const FRenderTargetHandle&) = default;
};
