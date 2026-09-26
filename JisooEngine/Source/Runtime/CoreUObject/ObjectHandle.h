#pragma once

#include <cstdint>
#include <limits>

/** GUObjectArray 슬롯과 세대를 함께 식별해 슬롯 재사용 후의 오래된 참조를 거부한다. */
struct FObjectHandle
{
    static constexpr std::uint32_t InvalidIndex = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t Index = InvalidIndex;
    std::uint32_t SerialNumber = 0;

    [[nodiscard]] bool IsSet() const
    {
        return Index != InvalidIndex && SerialNumber != 0;
    }

    friend bool operator==(const FObjectHandle&, const FObjectHandle&) = default;
};
