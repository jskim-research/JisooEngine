#pragma once

#include "Runtime/Input/InputInterfaces.h"

/** Editor나 Game이 현재 포커스·캡처 정책으로 선택한 입력 대상들이다. */
struct FInputRouteContext
{
    IInputReceiver* KeyboardTarget = nullptr;
    IInputReceiver* PointerTarget = nullptr;
};

/**
 * 입력 의미를 해석하지 않고 Keyboard와 Pointer 채널을 선택된 Receiver에 전달한다.
 * UI와 Viewport의 우선순위 판정은 Route Context를 만드는 구체 Engine의 책임이다.
 */
class FInputRouter
{
public:
    void Route(
        const FInputFrame& InputFrame,
        const FInputRouteContext& Context,
        float DeltaSeconds) const;
};
