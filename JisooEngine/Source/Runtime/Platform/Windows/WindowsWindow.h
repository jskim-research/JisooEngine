#pragma once

#include <Windows.h>

#include <cstdint>

class FWindowsWindow
{
public:
    FWindowsWindow() = default;
    ~FWindowsWindow();

    FWindowsWindow(const FWindowsWindow&) = delete;
    FWindowsWindow& operator=(const FWindowsWindow&) = delete;

    bool Initialize(const wchar_t* Title, std::uint32_t ClientWidth, std::uint32_t ClientHeight);
    bool ProcessMessages();
    void Shutdown();

    HWND GetNativeHandle() const noexcept;

private:
    static LRESULT CALLBACK WindowProcedure(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam);

    HINSTANCE InstanceHandle = nullptr;
    HWND WindowHandle = nullptr;
    ATOM WindowClassAtom = 0;
};
