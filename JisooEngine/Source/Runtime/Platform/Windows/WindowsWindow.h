#pragma once

#include <Windows.h>

#include <cstdint>

/**
 * Win32 Window Class와 Window Handle을 소유하고 현재 Thread의 Message를 처리한다.
 * 생성한 Window가 파괴되면 Native Handle을 무효화한다.
 */
class FWindowsWindow
{
public:
    FWindowsWindow() = default;
    ~FWindowsWindow();

    FWindowsWindow(const FWindowsWindow&) = delete;
    FWindowsWindow& operator=(const FWindowsWindow&) = delete;

    /**
     * 요청한 Client Area 크기로 Win32 Window를 생성하고 화면에 표시한다.
     *
     * @param Title Window Title로 사용할 null-terminated 문자열
     * @param ClientWidth Client Area의 pixel 너비
     * @param ClientHeight Client Area의 pixel 높이
     * @return Window 생성에 성공하면 true, 실패하면 false
     */
    bool Initialize(const wchar_t* Title, std::uint32_t ClientWidth, std::uint32_t ClientHeight);

    /**
     * 현재 Thread의 대기 중인 Window Message를 모두 처리한다.
     *
     * @return 실행을 계속할 수 있으면 true, WM_QUIT을 받았거나 Window가 파괴되었으면 false
     */
    bool ProcessMessages();
    void Shutdown();

    HWND GetNativeHandle() const noexcept;

private:
    static LRESULT CALLBACK WindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

    HINSTANCE InstanceHandle = nullptr;
    HWND WindowHandle = nullptr;
    ATOM WindowClassAtom = 0;
};
