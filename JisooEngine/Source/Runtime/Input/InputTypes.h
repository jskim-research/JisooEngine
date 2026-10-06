#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

enum class EInputKey : std::uint16_t
{
    Unknown,

    MouseLeft,
    MouseRight,
    MouseMiddle,
    MouseX1,
    MouseX2,

    Backspace,
    Tab,
    Enter,
    Shift,
    Control,
    Alt,
    Pause,
    CapsLock,
    Escape,
    Space,
    PageUp,
    PageDown,
    End,
    Home,
    Left,
    Up,
    Right,
    Down,
    Insert,
    Delete,

    Digit0,
    Digit1,
    Digit2,
    Digit3,
    Digit4,
    Digit5,
    Digit6,
    Digit7,
    Digit8,
    Digit9,

    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    Numpad0,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad4,
    Numpad5,
    Numpad6,
    Numpad7,
    Numpad8,
    Numpad9,
    Multiply,
    Add,
    Subtract,
    Decimal,
    Divide,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    Semicolon,
    Equal,
    Comma,
    Minus,
    Period,
    Slash,
    Backtick,
    LeftBracket,
    Backslash,
    RightBracket,
    Quote,

    Count
};

inline constexpr std::size_t InputKeyCount = static_cast<std::size_t>(EInputKey::Count);

[[nodiscard]] constexpr bool IsPointerInputKey(EInputKey Key) noexcept
{
    return Key >= EInputKey::MouseLeft && Key <= EInputKey::MouseX2;
}

enum class EInputEventType : std::uint8_t
{
    KeyDown,
    KeyUp,
    PointerMove,
    PointerWheel,
    TextInput,
    FocusGained,
    FocusLost
};

/** 플랫폼 메시지에서 변환된 입력 한 건이다. Engine Tick 전까지 FInputSystem이 값으로 보관한다. */
struct FInputEvent
{
    EInputEventType Type = EInputEventType::FocusLost;
    EInputKey Key = EInputKey::Unknown;
    std::int32_t PositionX = 0;
    std::int32_t PositionY = 0;
    std::int32_t DeltaX = 0;
    std::int32_t DeltaY = 0;
    float WheelDelta = 0.0f;
    char16_t Character = u'\0';
};

/**
 * 한 Engine Tick에서 소비할 입력 상태의 불변 스냅숏이다.
 * Pressed와 Released는 해당 프레임에만 유지되며 Down은 다음 프레임으로 이어진다.
 */
class FInputFrame
{
public:
    [[nodiscard]] bool IsDown(EInputKey Key) const noexcept;
    [[nodiscard]] bool WasPressed(EInputKey Key) const noexcept;
    [[nodiscard]] bool WasReleased(EInputKey Key) const noexcept;

    [[nodiscard]] std::int32_t GetPointerX() const noexcept;
    [[nodiscard]] std::int32_t GetPointerY() const noexcept;
    [[nodiscard]] std::int32_t GetPointerDeltaX() const noexcept;
    [[nodiscard]] std::int32_t GetPointerDeltaY() const noexcept;
    [[nodiscard]] float GetWheelDelta() const noexcept;
    [[nodiscard]] std::span<const char16_t> GetTextInput() const noexcept;
    [[nodiscard]] bool HasPointerPosition() const noexcept;
    [[nodiscard]] bool IsWindowFocused() const noexcept;

private:
    friend class FInputRouter;
    friend class FInputSystem;

    [[nodiscard]] static std::size_t ToIndex(EInputKey Key) noexcept;
    void ClearKeyboardInput() noexcept;
    void ClearPointerInput() noexcept;

    std::array<bool, InputKeyCount> DownKeys{};
    std::array<bool, InputKeyCount> PressedKeys{};
    std::array<bool, InputKeyCount> ReleasedKeys{};
    std::int32_t PointerX = 0;
    std::int32_t PointerY = 0;
    std::int32_t PointerDeltaX = 0;
    std::int32_t PointerDeltaY = 0;
    float WheelDelta = 0.0f;
    std::vector<char16_t> TextInput;
    bool bHasPointerPosition = false;
    bool bWindowFocused = false;
};
