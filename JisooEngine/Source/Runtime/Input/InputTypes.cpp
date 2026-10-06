#include "Runtime/Input/InputTypes.h"

bool FInputFrame::IsDown(EInputKey Key) const noexcept
{
    return DownKeys[ToIndex(Key)];
}

bool FInputFrame::WasPressed(EInputKey Key) const noexcept
{
    return PressedKeys[ToIndex(Key)];
}

bool FInputFrame::WasReleased(EInputKey Key) const noexcept
{
    return ReleasedKeys[ToIndex(Key)];
}

std::int32_t FInputFrame::GetPointerX() const noexcept
{
    return PointerX;
}

std::int32_t FInputFrame::GetPointerY() const noexcept
{
    return PointerY;
}

std::int32_t FInputFrame::GetPointerDeltaX() const noexcept
{
    return PointerDeltaX;
}

std::int32_t FInputFrame::GetPointerDeltaY() const noexcept
{
    return PointerDeltaY;
}

float FInputFrame::GetWheelDelta() const noexcept
{
    return WheelDelta;
}

bool FInputFrame::HasPointerPosition() const noexcept
{
    return bHasPointerPosition;
}

bool FInputFrame::IsWindowFocused() const noexcept
{
    return bWindowFocused;
}

std::size_t FInputFrame::ToIndex(EInputKey Key) noexcept
{
    const std::size_t Index = static_cast<std::size_t>(Key);
    return Index < InputKeyCount ? Index : 0;
}

void FInputFrame::ClearKeyboardInput() noexcept
{
    for (std::size_t Index = 0; Index < InputKeyCount; ++Index)
    {
        const EInputKey Key = static_cast<EInputKey>(Index);
        if (!IsPointerInputKey(Key))
        {
            DownKeys[Index] = false;
            PressedKeys[Index] = false;
            ReleasedKeys[Index] = false;
        }
    }
}

void FInputFrame::ClearPointerInput() noexcept
{
    for (std::size_t Index = 0; Index < InputKeyCount; ++Index)
    {
        const EInputKey Key = static_cast<EInputKey>(Index);
        if (IsPointerInputKey(Key))
        {
            DownKeys[Index] = false;
            PressedKeys[Index] = false;
            ReleasedKeys[Index] = false;
        }
    }

    PointerX = 0;
    PointerY = 0;
    PointerDeltaX = 0;
    PointerDeltaY = 0;
    WheelDelta = 0.0f;
    bHasPointerPosition = false;
}
