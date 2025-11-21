#include "hotkey_manager.h"
#include <windowsx.h>

HotkeyManager* HotkeyManager::instance_ = nullptr;

HotkeyManager::HotkeyManager()
    : keyboard_hook_(nullptr)
    , mouse_hook_(nullptr)
    , is_setting_hotkey_(false)
    , captured_hotkey_(0)
    , callback_(nullptr) {
    instance_ = this;
    memset(hotkey_name_, 0, sizeof(hotkey_name_));
}

HotkeyManager::~HotkeyManager() {
    Shutdown();
}

bool HotkeyManager::Initialize(HotkeyCallback callback) {
    callback_ = callback;
    return true;
}

void HotkeyManager::Shutdown() {
    if (keyboard_hook_) {
        UnhookWindowsHookEx(keyboard_hook_);
        keyboard_hook_ = nullptr;
    }
    
    if (mouse_hook_) {
        UnhookWindowsHookEx(mouse_hook_);
        mouse_hook_ = nullptr;
    }
}

bool HotkeyManager::SetHotkeyMode(bool enabled) {
    is_setting_hotkey_ = enabled;
    
    if (enabled) {
        // Hook into the system to catch keys
        HINSTANCE hInstance = GetModuleHandleW(nullptr);
        
        keyboard_hook_ = SetWindowsHookExW(
            WH_KEYBOARD_LL,
            LowLevelKeyboardProc,
            hInstance,
            0
        );
        
        if (!keyboard_hook_) {
            return false;
        }
        
        mouse_hook_ = SetWindowsHookExW(
            WH_MOUSE_LL,
            LowLevelMouseProc,
            hInstance,
            0
        );
        
        if (!mouse_hook_) {
            UnhookWindowsHookEx(keyboard_hook_);
            keyboard_hook_ = nullptr;
            return false;
        }
        
        wcscpy_s(hotkey_name_, L"Press any key...");
    } else {
        // Remove the hooks, we're done
        if (keyboard_hook_) {
            UnhookWindowsHookEx(keyboard_hook_);
            keyboard_hook_ = nullptr;
        }
        
        if (mouse_hook_) {
            UnhookWindowsHookEx(mouse_hook_);
            mouse_hook_ = nullptr;
        }
    }
    
    return true;
}

LRESULT CALLBACK HotkeyManager::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance_) {
        instance_->HandleKeyboardEvent(wParam, lParam);
    }
    
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK HotkeyManager::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance_) {
        instance_->HandleMouseEvent(wParam, lParam);
    }
    
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void HotkeyManager::HandleKeyboardEvent(WPARAM wParam, LPARAM lParam) {
    if (!is_setting_hotkey_) {
        // Just normal key processing
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
            int key_code = kbd->vkCode;
            
            if (callback_) {
                callback_(key_code, true);
            }
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
            int key_code = kbd->vkCode;
            
            if (callback_) {
                callback_(key_code, false);
            }
        }
        return;
    }
    
    // We're in capture mode, waiting for a key
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
        captured_hotkey_ = kbd->vkCode;
        SetKeyName(captured_hotkey_);
        
        // Got it! Back to normal mode but keep listening
        is_setting_hotkey_ = false;
    }
}

void HotkeyManager::HandleMouseEvent(WPARAM wParam, LPARAM lParam) {
    if (!is_setting_hotkey_) {
        // Normal mouse clicks
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || 
            wParam == WM_MBUTTONDOWN || wParam == WM_XBUTTONDOWN) {
            
            MSLLHOOKSTRUCT* mouse = (MSLLHOOKSTRUCT*)lParam;
            int key_code = 0;
            
            switch (wParam) {
            case WM_LBUTTONDOWN: key_code = VK_LBUTTON; break;
            case WM_RBUTTONDOWN: key_code = VK_RBUTTON; break;
            case WM_MBUTTONDOWN: key_code = VK_MBUTTON; break;
            case WM_XBUTTONDOWN:
                key_code = GET_XBUTTON_WPARAM(mouse->mouseData) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
                break;
            }
            
            if (callback_) {
                callback_(key_code, true);
            }
        } else if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || 
                   wParam == WM_MBUTTONUP || wParam == WM_XBUTTONUP) {
            
            MSLLHOOKSTRUCT* mouse = (MSLLHOOKSTRUCT*)lParam;
            int key_code = 0;
            
            switch (wParam) {
            case WM_LBUTTONUP: key_code = VK_LBUTTON; break;
            case WM_RBUTTONUP: key_code = VK_RBUTTON; break;
            case WM_MBUTTONUP: key_code = VK_MBUTTON; break;
            case WM_XBUTTONUP:
                key_code = GET_XBUTTON_WPARAM(mouse->mouseData) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
                break;
            }
            
            if (callback_) {
                callback_(key_code, false);
            }
        }
        return;
    }
    
    // Waiting for a mouse click to set as hotkey
    if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || 
        wParam == WM_MBUTTONDOWN || wParam == WM_XBUTTONDOWN) {
        
        MSLLHOOKSTRUCT* mouse = (MSLLHOOKSTRUCT*)lParam;
        
        switch (wParam) {
        case WM_LBUTTONDOWN: captured_hotkey_ = VK_LBUTTON; wcscpy_s(hotkey_name_, L"Left Mouse"); break;
        case WM_RBUTTONDOWN: captured_hotkey_ = VK_RBUTTON; wcscpy_s(hotkey_name_, L"Right Mouse"); break;
        case WM_MBUTTONDOWN: captured_hotkey_ = VK_MBUTTON; wcscpy_s(hotkey_name_, L"Middle Mouse"); break;
        case WM_XBUTTONDOWN:
            if (GET_XBUTTON_WPARAM(mouse->mouseData) == XBUTTON1) {
                captured_hotkey_ = VK_XBUTTON1;
                wcscpy_s(hotkey_name_, L"Mouse X1");
            } else {
                captured_hotkey_ = VK_XBUTTON2;
                wcscpy_s(hotkey_name_, L"Mouse X2");
            }
            break;
        }
        
        // Got it! Back to normal mode but keep listening
        is_setting_hotkey_ = false;
    }
}

void HotkeyManager::SetKeyName(int key_code) {
    switch (key_code) {
    case VK_LBUTTON: wcscpy_s(hotkey_name_, L"Left Mouse"); break;
    case VK_RBUTTON: wcscpy_s(hotkey_name_, L"Right Mouse"); break;
    case VK_MBUTTON: wcscpy_s(hotkey_name_, L"Middle Mouse"); break;
    case VK_XBUTTON1: wcscpy_s(hotkey_name_, L"Mouse X1"); break;
    case VK_XBUTTON2: wcscpy_s(hotkey_name_, L"Mouse X2"); break;
    case VK_BACK: wcscpy_s(hotkey_name_, L"Backspace"); break;
    case VK_TAB: wcscpy_s(hotkey_name_, L"Tab"); break;
    case VK_RETURN: wcscpy_s(hotkey_name_, L"Enter"); break;
    case VK_SHIFT: wcscpy_s(hotkey_name_, L"Shift"); break;
    case VK_CONTROL: wcscpy_s(hotkey_name_, L"Ctrl"); break;
    case VK_MENU: wcscpy_s(hotkey_name_, L"Alt"); break;
    case VK_PAUSE: wcscpy_s(hotkey_name_, L"Pause"); break;
    case VK_CAPITAL: wcscpy_s(hotkey_name_, L"Caps Lock"); break;
    case VK_ESCAPE: wcscpy_s(hotkey_name_, L"Escape"); break;
    case VK_SPACE: wcscpy_s(hotkey_name_, L"Space"); break;
    case VK_PRIOR: wcscpy_s(hotkey_name_, L"Page Up"); break;
    case VK_NEXT: wcscpy_s(hotkey_name_, L"Page Down"); break;
    case VK_END: wcscpy_s(hotkey_name_, L"End"); break;
    case VK_HOME: wcscpy_s(hotkey_name_, L"Home"); break;
    case VK_LEFT: wcscpy_s(hotkey_name_, L"Left Arrow"); break;
    case VK_UP: wcscpy_s(hotkey_name_, L"Up Arrow"); break;
    case VK_RIGHT: wcscpy_s(hotkey_name_, L"Right Arrow"); break;
    case VK_DOWN: wcscpy_s(hotkey_name_, L"Down Arrow"); break;
    case VK_SNAPSHOT: wcscpy_s(hotkey_name_, L"Print Screen"); break;
    case VK_INSERT: wcscpy_s(hotkey_name_, L"Insert"); break;
    case VK_DELETE: wcscpy_s(hotkey_name_, L"Delete"); break;
    case VK_LWIN: wcscpy_s(hotkey_name_, L"Left Win"); break;
    case VK_RWIN: wcscpy_s(hotkey_name_, L"Right Win"); break;
    case VK_APPS: wcscpy_s(hotkey_name_, L"Menu"); break;
    case VK_NUMPAD0: wcscpy_s(hotkey_name_, L"Num 0"); break;
    case VK_NUMPAD1: wcscpy_s(hotkey_name_, L"Num 1"); break;
    case VK_NUMPAD2: wcscpy_s(hotkey_name_, L"Num 2"); break;
    case VK_NUMPAD3: wcscpy_s(hotkey_name_, L"Num 3"); break;
    case VK_NUMPAD4: wcscpy_s(hotkey_name_, L"Num 4"); break;
    case VK_NUMPAD5: wcscpy_s(hotkey_name_, L"Num 5"); break;
    case VK_NUMPAD6: wcscpy_s(hotkey_name_, L"Num 6"); break;
    case VK_NUMPAD7: wcscpy_s(hotkey_name_, L"Num 7"); break;
    case VK_NUMPAD8: wcscpy_s(hotkey_name_, L"Num 8"); break;
    case VK_NUMPAD9: wcscpy_s(hotkey_name_, L"Num 9"); break;
    case VK_MULTIPLY: wcscpy_s(hotkey_name_, L"Num *"); break;
    case VK_ADD: wcscpy_s(hotkey_name_, L"Num +"); break;
    case VK_SUBTRACT: wcscpy_s(hotkey_name_, L"Num -"); break;
    case VK_DECIMAL: wcscpy_s(hotkey_name_, L"Num ."); break;
    case VK_DIVIDE: wcscpy_s(hotkey_name_, L"Num /"); break;
    case VK_F1: wcscpy_s(hotkey_name_, L"F1"); break;
    case VK_F2: wcscpy_s(hotkey_name_, L"F2"); break;
    case VK_F3: wcscpy_s(hotkey_name_, L"F3"); break;
    case VK_F4: wcscpy_s(hotkey_name_, L"F4"); break;
    case VK_F5: wcscpy_s(hotkey_name_, L"F5"); break;
    case VK_F6: wcscpy_s(hotkey_name_, L"F6"); break;
    case VK_F7: wcscpy_s(hotkey_name_, L"F7"); break;
    case VK_F8: wcscpy_s(hotkey_name_, L"F8"); break;
    case VK_F9: wcscpy_s(hotkey_name_, L"F9"); break;
    case VK_F10: wcscpy_s(hotkey_name_, L"F10"); break;
    case VK_F11: wcscpy_s(hotkey_name_, L"F11"); break;
    case VK_F12: wcscpy_s(hotkey_name_, L"F12"); break;
    default:
        if (key_code >= '0' && key_code <= '9') {
            swprintf_s(hotkey_name_, L"%c", key_code);
        } else if (key_code >= 'A' && key_code <= 'Z') {
            swprintf_s(hotkey_name_, L"%c", key_code);
        } else {
            swprintf_s(hotkey_name_, L"Key %d", key_code);
        }
        break;
    }
}

std::wstring HotkeyManager::GetHotkeyName() const {
    return std::wstring(hotkey_name_);
}
