#include "Runtime/Platform/Windows/WindowsWindow.h"

namespace
{
constexpr wchar_t WindowClassName[] = L"JisooEngineWindowClass";
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
        }
        break;

    default:
        break;
    }

    return DefWindowProcW(InWindowHandle, Message, WParam, LParam);
}
