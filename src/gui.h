#pragma once
#include <windows.h>
#include <string>
#include <commctrl.h>

class GUI {
public:
    GUI();
    ~GUI();

    bool Initialize(HINSTANCE hInstance);
    void Run();
    void Shutdown();

    void UpdateHotkeyDisplay(const std::wstring& hotkey);
    void UpdateStatusDisplay(const std::wstring& status);
    void ShowError(const std::wstring& message);

    // Getters for controls
    HWND GetHotkeyButton() const { return hotkey_button_; }
    HWND GetStartButton() const { return start_button_; }
    HWND GetStopButton() const { return stop_button_; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void CreateControls();
    void ApplyModernStyling();
    void CenterWindow();
    
    HWND hwnd_;
    HWND hotkey_button_;
    HWND start_button_;
    HWND stop_button_;
    HWND hotkey_label_;
    HWND status_label_;
    HWND title_label_;
    
    HFONT title_font_;
    HFONT body_font_;
    
    static GUI* instance_;
};
