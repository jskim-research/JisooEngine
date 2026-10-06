#pragma once

#include "Runtime/Input/InputInterfaces.h"

#include <vector>

/**
 * 플랫폼 이벤트 큐와 현재 키 상태를 소유하고 Tick마다 FInputFrame을 확정한다.
 * 이벤트 생산과 AdvanceFrame은 같은 Thread에서 순서대로 호출해야 한다.
 */
class FInputSystem final : public IInputEventSink
{
public:
    void EnqueueInputEvent(const FInputEvent& Event) override;

    /** 대기 중인 이벤트를 순서대로 소비해 이번 Engine Tick의 입력 상태를 확정한다. */
    void AdvanceFrame();
    void Reset();

    [[nodiscard]] const FInputFrame& GetCurrentFrame() const noexcept;

private:
    void ApplyEvent(FInputFrame& Frame, const FInputEvent& Event);

    std::vector<FInputEvent> PendingEvents;
    FInputFrame CurrentFrame;
};
