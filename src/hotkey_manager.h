#pragma once
#include <windows.h>
#include <string>
#include <functional>

class HotkeyManager {
public:
    using HotkeyCallback = std::function<void(int, bool)>; // key_code, is_pressed

    HotkeyManager();
    ~HotkeyManager();

    bool Initialize(HotkeyCallback callback);
    void Shutdown();
    
    bool SetHotkeyMode(bool enabled);
    bool IsSettingHotkey() const { return is_setting_hotkey_; }
    
    int GetCapturedHotkey() const { return captured_hotkey_; }
    std::wstring GetHotkeyName() const;

private:
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    void HandleKeyboardEvent(WPARAM wParam, LPARAM lParam);
    void HandleMouseEvent(WPARAM wParam, LPARAM lParam);
    void SetKeyName(int key_code);
    
    HHOOK keyboard_hook_;
    HHOOK mouse_hook_;
    bool is_setting_hotkey_;
    int captured_hotkey_;
    wchar_t hotkey_name_[64];
    HotkeyCallback callback_;
    
    static HotkeyManager* instance_;
};
