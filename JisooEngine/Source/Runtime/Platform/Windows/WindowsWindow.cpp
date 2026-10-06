#include "Runtime/Platform/Windows/WindowsWindow.h"

#include "Runtime/Input/InputInterfaces.h"

#include <Windowsx.h>

namespace
{
constexpr wchar_t WindowClassName[] = L"JisooEngineWindowClass";

EInputKey TranslateVirtualKey(WPARAM VirtualKey)
{
    if (VirtualKey >= '0' && VirtualKey <= '9')
    {
        return static_cast<EInputKey>(
            static_cast<std::uint16_t>(EInputKey::Digit0) +
            static_cast<std::uint16_t>(VirtualKey - '0'));
    }

    if (VirtualKey >= 'A' && VirtualKey <= 'Z')
    {
        return static_cast<EInputKey>(
            static_cast<std::uint16_t>(EInputKey::A) +
            static_cast<std::uint16_t>(VirtualKey - 'A'));
    }

    if (VirtualKey >= VK_NUMPAD0 && VirtualKey <= VK_NUMPAD9)
    {
        return static_cast<EInputKey>(
            static_cast<std::uint16_t>(EInputKey::Numpad0) +
            static_cast<std::uint16_t>(VirtualKey - VK_NUMPAD0));
    }

    if (VirtualKey >= VK_F1 && VirtualKey <= VK_F12)
    {
        return static_cast<EInputKey>(
            static_cast<std::uint16_t>(EInputKey::F1) +
            static_cast<std::uint16_t>(VirtualKey - VK_F1));
    }

    switch (VirtualKey)
    {
    case VK_BACK: return EInputKey::Backspace;
    case VK_TAB: return EInputKey::Tab;
    case VK_RETURN: return EInputKey::Enter;
    case VK_SHIFT: return EInputKey::Shift;
    case VK_CONTROL: return EInputKey::Control;
    case VK_MENU: return EInputKey::Alt;
    case VK_PAUSE: return EInputKey::Pause;
    case VK_CAPITAL: return EInputKey::CapsLock;
    case VK_ESCAPE: return EInputKey::Escape;
    case VK_SPACE: return EInputKey::Space;
    case VK_PRIOR: return EInputKey::PageUp;
    case VK_NEXT: return EInputKey::PageDown;
    case VK_END: return EInputKey::End;
    case VK_HOME: return EInputKey::Home;
    case VK_LEFT: return EInputKey::Left;
    case VK_UP: return EInputKey::Up;
    case VK_RIGHT: return EInputKey::Right;
    case VK_DOWN: return EInputKey::Down;
    case VK_INSERT: return EInputKey::Insert;
    case VK_DELETE: return EInputKey::Delete;
    case VK_MULTIPLY: return EInputKey::Multiply;
    case VK_ADD: return EInputKey::Add;
    case VK_SUBTRACT: return EInputKey::Subtract;
    case VK_DECIMAL: return EInputKey::Decimal;
    case VK_DIVIDE: return EInputKey::Divide;
    case VK_OEM_1: return EInputKey::Semicolon;
    case VK_OEM_PLUS: return EInputKey::Equal;
    case VK_OEM_COMMA: return EInputKey::Comma;
    case VK_OEM_MINUS: return EInputKey::Minus;
    case VK_OEM_PERIOD: return EInputKey::Period;
    case VK_OEM_2: return EInputKey::Slash;
    case VK_OEM_3: return EInputKey::Backtick;
    case VK_OEM_4: return EInputKey::LeftBracket;
    case VK_OEM_5: return EInputKey::Backslash;
    case VK_OEM_6: return EInputKey::RightBracket;
    case VK_OEM_7: return EInputKey::Quote;
    default: return EInputKey::Unknown;
    }
}
}

FWindowsWindow::~FWindowsWindow()
{
    Shutdown();
}

bool FWindowsWindow::Initialize(const wchar_t* Title, std::uint32_t ClientWidth, std::uint32_t ClientHeight)
{
    if (WindowHandle != nullptr)
    {
        return true;
    }

    InstanceHandle = GetModuleHandleW(nullptr);
    if (InstanceHandle == nullptr)
    {
        return false;
    }

    WNDCLASSEXW WindowClass{};
    WindowClass.cbSize = sizeof(WindowClass);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = &FWindowsWindow::WindowProcedure;
    WindowClass.hInstance = InstanceHandle;
    WindowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    WindowClass.lpszClassName = WindowClassName;

    WindowClassAtom = RegisterClassExW(&WindowClass);
    if (WindowClassAtom == 0)
    {
        return false;
    }

    RECT WindowRectangle{
        0,
        0,
        static_cast<LONG>(ClientWidth),
        static_cast<LONG>(ClientHeight)
    };

    constexpr DWORD WindowStyle = WS_OVERLAPPEDWINDOW;

    // 요청한 크기는 전체 Window가 아니라 Client Area에 적용되어야 한다.
    if (AdjustWindowRectEx(&WindowRectangle, WindowStyle, FALSE, 0) == FALSE)
    {
        Shutdown();
        return false;
    }

    WindowHandle = CreateWindowExW(
        0,
        WindowClassName,
        Title,
        WindowStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WindowRectangle.right - WindowRectangle.left,
        WindowRectangle.bottom - WindowRectangle.top,
        nullptr,
        nullptr,
        InstanceHandle,
        this);

    if (WindowHandle == nullptr)
    {
        Shutdown();
        return false;
    }

    ShowWindow(WindowHandle, SW_SHOWDEFAULT);
    UpdateWindow(WindowHandle);
    return true;
}

bool FWindowsWindow::ProcessMessages()
{
    MSG Message{};
    while (PeekMessageW(&Message, nullptr, 0, 0, PM_REMOVE) != FALSE)
    {
        if (Message.message == WM_QUIT)
        {
            return false;
        }

        TranslateMessage(&Message);
        DispatchMessageW(&Message);
    }

    return WindowHandle != nullptr;
}

void FWindowsWindow::Shutdown()
{
    if (WindowHandle != nullptr)
    {
        DestroyWindow(WindowHandle);
        WindowHandle = nullptr;
    }

    if (WindowClassAtom != 0 && InstanceHandle != nullptr)
    {
        UnregisterClassW(WindowClassName, InstanceHandle);
        WindowClassAtom = 0;
    }

    InstanceHandle = nullptr;
    bHasPointerPosition = false;
}

void FWindowsWindow::SetInputEventSink(IInputEventSink* InInputEventSink) noexcept
{
    InputEventSink = InInputEventSink;
    if (InputEventSink != nullptr && WindowHandle != nullptr)
    {
        FInputEvent FocusEvent;
        FocusEvent.Type = GetFocus() == WindowHandle
            ? EInputEventType::FocusGained
            : EInputEventType::FocusLost;
        InputEventSink->EnqueueInputEvent(FocusEvent);
    }
}

HWND FWindowsWindow::GetNativeHandle() const noexcept
{
    return WindowHandle;
}

LRESULT CALLBACK FWindowsWindow::WindowProcedure(
    HWND InWindowHandle,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam)
{
    FWindowsWindow* Window = reinterpret_cast<FWindowsWindow*>(
        GetWindowLongPtrW(InWindowHandle, GWLP_USERDATA));

    // CreateWindowExW로 전달한 this를 HWND에 연결해 이후 Message를 객체 상태로 처리한다.
    if (Message == WM_NCCREATE)
    {
        const auto* CreateStructure = reinterpret_cast<const CREATESTRUCTW*>(LParam);
        Window = static_cast<FWindowsWindow*>(CreateStructure->lpCreateParams);
        Window->WindowHandle = InWindowHandle;
        SetWindowLongPtrW(InWindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Window));
    }

    switch (Message)
    {
    case WM_SETFOCUS:
        if (Window != nullptr && Window->InputEventSink != nullptr)
        {
            FInputEvent Event;
            Event.Type = EInputEventType::FocusGained;
            Window->InputEventSink->EnqueueInputEvent(Event);
        }
        return 0;

    case WM_KILLFOCUS:
        if (Window != nullptr)
        {
            Window->bHasPointerPosition = false;
            if (Window->InputEventSink != nullptr)
            {
                FInputEvent Event;
                Event.Type = EInputEventType::FocusLost;
                Window->InputEventSink->EnqueueInputEvent(Event);
            }
        }
        return 0;

    case WM_KEYDOWN:
        if (Window != nullptr && Window->InputEventSink != nullptr)
        {
            FInputEvent Event;
            Event.Type = EInputEventType::KeyDown;
            Event.Key = TranslateVirtualKey(WParam);
            Window->InputEventSink->EnqueueInputEvent(Event);
        }
        return 0;

    case WM_KEYUP:
        if (Window != nullptr && Window->InputEventSink != nullptr)
        {
            FInputEvent Event;
            Event.Type = EInputEventType::KeyUp;
            Event.Key = TranslateVirtualKey(WParam);
            Window->InputEventSink->EnqueueInputEvent(Event);
        }
        return 0;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        if (Window != nullptr && Window->InputEventSink != nullptr)
        {
            FInputEvent Event;
            Event.Type = Message == WM_SYSKEYDOWN
                ? EInputEventType::KeyDown
                : EInputEventType::KeyUp;
            Event.Key = TranslateVirtualKey(WParam);
            Window->InputEventSink->EnqueueInputEvent(Event);
        }
        break;

    case WM_CHAR:
        if (Window != nullptr && Window->InputEventSink != nullptr)
        {
            FInputEvent Event;
            Event.Type = EInputEventType::TextInput;
            Event.Character = static_cast<char16_t>(WParam);
            Window->InputEventSink->EnqueueInputEvent(Event);
        }
        return 0;

    case WM_MOUSEMOVE:
        if (Window != nullptr && Window->InputEventSink != nullptr)
        {
            const std::int32_t PointerX = GET_X_LPARAM(LParam);
            const std::int32_t PointerY = GET_Y_LPARAM(LParam);

            FInputEvent Event;
            Event.Type = EInputEventType::PointerMove;
            Event.PositionX = PointerX;
            Event.PositionY = PointerY;
            if (Window->bHasPointerPosition)
            {
                Event.DeltaX = PointerX - Window->LastPointerX;
                Event.DeltaY = PointerY - Window->LastPointerY;
            }

            Window->LastPointerX = PointerX;
            Window->LastPointerY = PointerY;
            Window->bHasPointerPosition = true;
            Window->InputEventSink->EnqueueInputEvent(Event);
        }
        return 0;

    case WM_MOUSEWHEEL:
        if (Window != nullptr && Window->InputEventSink != nullptr)
        {
            FInputEvent Event;
            Event.Type = EInputEventType::PointerWheel;
            Event.WheelDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(WParam)) /
                static_cast<float>(WHEEL_DELTA);
            Window->InputEventSink->EnqueueInputEvent(Event);
        }
        return 0;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        if (Window != nullptr && Window->InputEventSink != nullptr)
        {
            FInputEvent Event;
            Event.Type = Message == WM_LBUTTONDOWN ||
                Message == WM_RBUTTONDOWN ||
                Message == WM_MBUTTONDOWN ||
                Message == WM_XBUTTONDOWN
                ? EInputEventType::KeyDown
                : EInputEventType::KeyUp;

            if (Message == WM_LBUTTONDOWN || Message == WM_LBUTTONUP)
            {
                Event.Key = EInputKey::MouseLeft;
            }
            else if (Message == WM_RBUTTONDOWN || Message == WM_RBUTTONUP)
            {
                Event.Key = EInputKey::MouseRight;
            }
            else if (Message == WM_MBUTTONDOWN || Message == WM_MBUTTONUP)
            {
                Event.Key = EInputKey::MouseMiddle;
            }
            else
            {
                Event.Key = GET_XBUTTON_WPARAM(WParam) == XBUTTON1
                    ? EInputKey::MouseX1
                    : EInputKey::MouseX2;
            }
            Window->InputEventSink->EnqueueInputEvent(Event);
        }
        return 0;

    case WM_CLOSE:
        DestroyWindow(InWindowHandle);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_NCDESTROY:
        SetWindowLongPtrW(InWindowHandle, GWLP_USERDATA, 0);
        if (Window != nullptr)
        {
            Window->WindowHandle = nullptr;
            Window->bHasPointerPosition = false;
        }
        break;

    default:
        break;
    }

    return DefWindowProcW(InWindowHandle, Message, WParam, LParam);
}
