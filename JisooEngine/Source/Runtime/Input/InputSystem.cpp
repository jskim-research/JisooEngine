#include "Runtime/Input/InputSystem.h"

#include <utility>

void FInputSystem::EnqueueInputEvent(const FInputEvent& Event)
{
    PendingEvents.push_back(Event);
}

void FInputSystem::AdvanceFrame()
{
    FInputFrame NextFrame;
    NextFrame.DownKeys = CurrentFrame.DownKeys;
    NextFrame.PointerX = CurrentFrame.PointerX;
    NextFrame.PointerY = CurrentFrame.PointerY;
    NextFrame.bHasPointerPosition = CurrentFrame.bHasPointerPosition;
    NextFrame.bWindowFocused = CurrentFrame.bWindowFocused;

    std::vector<FInputEvent> Events = std::move(PendingEvents);
    PendingEvents.clear();
    for (const FInputEvent& Event : Events)
    {
        ApplyEvent(NextFrame, Event);
    }

    CurrentFrame = NextFrame;
}

void FInputSystem::Reset()
{
    PendingEvents.clear();
    CurrentFrame = {};
}

const FInputFrame& FInputSystem::GetCurrentFrame() const noexcept
{
    return CurrentFrame;
}

void FInputSystem::ApplyEvent(FInputFrame& Frame, const FInputEvent& Event)
{
    const std::size_t KeyIndex = FInputFrame::ToIndex(Event.Key);

    switch (Event.Type)
    {
    case EInputEventType::KeyDown:
        if (Event.Key != EInputKey::Unknown && Frame.bWindowFocused && !Frame.DownKeys[KeyIndex])
        {
            Frame.DownKeys[KeyIndex] = true;
            Frame.PressedKeys[KeyIndex] = true;
        }
        break;

    case EInputEventType::KeyUp:
        if (Event.Key != EInputKey::Unknown && Frame.DownKeys[KeyIndex])
        {
            Frame.DownKeys[KeyIndex] = false;
            Frame.ReleasedKeys[KeyIndex] = true;
        }
        break;

    case EInputEventType::PointerMove:
        if (Frame.bWindowFocused)
        {
            Frame.PointerX = Event.PositionX;
            Frame.PointerY = Event.PositionY;
            Frame.PointerDeltaX += Event.DeltaX;
            Frame.PointerDeltaY += Event.DeltaY;
            Frame.bHasPointerPosition = true;
        }
        break;

    case EInputEventType::PointerWheel:
        if (Frame.bWindowFocused)
        {
            Frame.WheelDelta += Event.WheelDelta;
        }
        break;

    case EInputEventType::TextInput:
        if (Frame.bWindowFocused && Event.Character != u'\0')
        {
            Frame.TextInput.push_back(Event.Character);
        }
        break;

    case EInputEventType::FocusGained:
        Frame.bWindowFocused = true;
        break;

    case EInputEventType::FocusLost:
        Frame.bWindowFocused = false;
        Frame.PointerDeltaX = 0;
        Frame.PointerDeltaY = 0;
        Frame.WheelDelta = 0.0f;
        Frame.TextInput.clear();
        Frame.bHasPointerPosition = false;
        for (std::size_t Index = 0; Index < InputKeyCount; ++Index)
        {
            if (Frame.DownKeys[Index])
            {
                Frame.DownKeys[Index] = false;
                Frame.ReleasedKeys[Index] = true;
            }
        }
        break;
    }
}
