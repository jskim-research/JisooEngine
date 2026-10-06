#pragma once

#include "Runtime/Input/InputTypes.h"

/** 플랫폼 입력 생산자가 수명 비소유로 연결하는 이벤트 수신 계약이다. */
class IInputEventSink
{
public:
    virtual ~IInputEventSink() = default;

    /** 현재 Thread에서 발생한 이벤트를 다음 Engine Tick의 입력 큐에 값으로 추가한다. */
    virtual void EnqueueInputEvent(const FInputEvent& Event) = 0;
};

/** Input Router가 선택한 프레임 입력을 소비하는 대상 계약이다. */
class IInputReceiver
{
public:
    virtual ~IInputReceiver() = default;

    /** 허용된 입력 채널만 남은 프레임 스냅숏을 현재 Tick 안에서 소비한다. */
    virtual void ProcessInput(const FInputFrame& InputFrame, float DeltaSeconds) = 0;
};
