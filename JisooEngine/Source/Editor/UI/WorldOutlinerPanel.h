#pragma once

#include "Runtime/CoreUObject/ObjectHandle.h"

#include <array>

class UWorld;

/** World의 현재 Actor 목록을 검색·선택할 수 있는 최소 Editor Panel이다. */
class FWorldOutlinerPanel
{
public:
    void Draw(const UWorld& World);

    void SetOpen(bool bInOpen);
    [[nodiscard]] bool IsOpen() const noexcept;

private:
    void ResetState();

    std::array<char, 128> SearchText{};
    FObjectHandle SelectedActor;
    bool bOpen = true;
};
