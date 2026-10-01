#pragma once

#include <cstdint>
#include <limits>

/** FScene 슬롯과 세대를 함께 식별해 제거되거나 재사용된 Proxy Handle을 거부한다. */
struct FPrimitiveSceneHandle
{
    static constexpr std::uint32_t InvalidIndex = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t Index = InvalidIndex;
    std::uint32_t Generation = 0;

    [[nodiscard]] bool IsSet() const
    {
        return Index != InvalidIndex && Generation != 0;
    }

    friend bool operator==(const FPrimitiveSceneHandle&, const FPrimitiveSceneHandle&) = default;
};
