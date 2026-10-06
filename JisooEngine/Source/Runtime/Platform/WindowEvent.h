#pragma once

#include <cstdint>

/** 플랫폼 Window가 보고한 최신 Client Area 크기 변경 값이다. */
struct FWindowResizeEvent
{
    std::uint32_t Width = 0;
    std::uint32_t Height = 0;
    bool bMinimized = false;
};
