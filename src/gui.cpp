#include "gui.h"
#include <dwmapi.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

GUI* GUI::instance_ = nullptr;

GUI::GUI() 
    : hwnd_(nullptr)
    , hotkey_button_(nullptr)
    , start_button_(nullptr)
    , stop_button_(nullptr)
    , hotkey_label_(nullptr)
    , status_label_(nullptr)
    , title_label_(nullptr)
    , title_font_(nullptr)
    , body_font_(nullptr) {
    instance_ = this;
}

GUI::~GUI() {
    Shutdown();
}

bool GUI::Initialize(HINSTANCE hInstance) {
    // Set up the window class (make it look modern)
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(248, 249, 250)); // Light gray background
    wc.lpszClassName = L"TabGlitcherWindow";
    
    if (!RegisterClassExW(&wc)) {
        return false;
    }

    // Build the main window
    hwnd_ = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW,
        L"TabGlitcherWindow",
        L"🎯 Tab Glitcher - Roblox Process Suspend Tool",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        520, 420,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd_) {
        return false;
    }

    // Turn on the fancy modern look
    ApplyModernStyling();
    
    // Add all the buttons and labels
    CreateControls();
    
    CenterWindow();
    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);

    return true;
}

void GUI::Run() {
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void GUI::Shutdown() {
    if (title_font_) {
        DeleteObject(title_font_);
        title_font_ = nullptr;
    }
    if (body_font_) {
        DeleteObject(body_font_);
        body_font_ = nullptr;
    }
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void GUI::ApplyModernStyling() {
    // Use DWM for that sleek dark mode
    if (HWND hwnd = hwnd_) {
        DWMWINDOWATTRIBUTE dwm_attribute = DWMWA_USE_IMMERSIVE_DARK_MODE;
        BOOL value = TRUE;
        DwmSetWindowAttribute(hwnd, dwm_attribute, &value, sizeof(value));
    }
}

void GUI::CreateControls() {
    // Load up some nice fonts
    title_font_ = CreateFontW(
        -20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    body_font_ = CreateFontW(
        -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    // The big title at the top
    title_label_ = CreateWindowExW(
        0, L"STATIC", L"🎯 Tab Glitcher",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        20, 20, 460, 30,
        hwnd_, nullptr, nullptr, nullptr
    );
    SendMessageW(title_label_, WM_SETFONT, (WPARAM)title_font_, TRUE);

    // Button to set the hotkey
    hotkey_button_ = CreateWindowExW(
        0, L"BUTTON", L"🎯 Set Hotkey",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
        30, 70, 140, 45,
        hwnd_, (HMENU)1, nullptr, nullptr
    );
    SendMessageW(hotkey_button_, WM_SETFONT, (WPARAM)body_font_, TRUE);

    // The Go button
    start_button_ = CreateWindowExW(
        0, L"BUTTON", L"▶ Start",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON,
        190, 70, 120, 45,
        hwnd_, (HMENU)2, nullptr, nullptr
    );
    SendMessageW(start_button_, WM_SETFONT, (WPARAM)body_font_, TRUE);

    // The Stop button
    stop_button_ = CreateWindowExW(
        0, L"BUTTON", L"⏹ Stop",
        WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
        330, 70, 120, 45,
        hwnd_, (HMENU)3, nullptr, nullptr
    );
    SendMessageW(stop_button_, WM_SETFONT, (WPARAM)body_font_, TRUE);

    // Label for the hotkey
    CreateWindowExW(
        0, L"STATIC", L"🔧 Current Hotkey:",
        WS_VISIBLE | WS_CHILD,
        30, 130, 150, 20,
        hwnd_, nullptr, nullptr, nullptr
    );
    SendMessageW(GetDlgItem(hwnd_, 4), WM_SETFONT, (WPARAM)body_font_, TRUE);

    hotkey_label_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"STATIC", L"Not Set",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        190, 130, 260, 22,
        hwnd_, (HMENU)4, nullptr, nullptr
    );
    SendMessageW(hotkey_label_, WM_SETFONT, (WPARAM)body_font_, TRUE);

    // Label for status
    CreateWindowExW(
        0, L"STATIC", L"📊 Status:",
        WS_VISIBLE | WS_CHILD,
        30, 165, 150, 20,
        hwnd_, nullptr, nullptr, nullptr
    );
    SendMessageW(GetDlgItem(hwnd_, 5), WM_SETFONT, (WPARAM)body_font_, TRUE);

    status_label_ = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"STATIC", L"⚪ Inactive",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        190, 165, 260, 22,
        hwnd_, (HMENU)5, nullptr, nullptr
    );
    SendMessageW(status_label_, WM_SETFONT, (WPARAM)body_font_, TRUE);

    // How to use this thing
    CreateWindowExW(
        0, L"STATIC", 
        L"📋 Instructions:\n"
        L"1. Click 'Set Hotkey' and press any key\n"
        L"2. Click 'Start' to begin monitoring\n"
        L"3. Hold hotkey to suspend Roblox\n"
        L"4. Release hotkey to resume Roblox",
        WS_VISIBLE | WS_CHILD,
        30, 200, 440, 80,
        hwnd_, nullptr, nullptr, nullptr
    );
    SendMessageW(GetDlgItem(hwnd_, 6), WM_SETFONT, (WPARAM)body_font_, TRUE);

    // Helpful tip
    CreateWindowExW(
        0, L"STATIC", L"ℹ️ Make sure Roblox is running before starting",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        30, 300, 440, 20,
        hwnd_, nullptr, nullptr, nullptr
    );
    SendMessageW(GetDlgItem(hwnd_, 7), WM_SETFONT, (WPARAM)body_font_, TRUE);
}

void GUI::CenterWindow() {
    RECT rect = {};
    GetWindowRect(hwnd_, &rect);
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    
    int x = (screen_width - width) / 2;
    int y = (screen_height - height) / 2;
    
    SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void GUI::UpdateHotkeyDisplay(const std::wstring& hotkey) {
    if (hotkey_label_) {
        SetWindowTextW(hotkey_label_, hotkey.c_str());
    }
}

void GUI::UpdateStatusDisplay(const std::wstring& status) {
    if (status_label_) {
        SetWindowTextW(status_label_, status.c_str());
    }
}

void GUI::ShowError(const std::wstring& message) {
    MessageBoxW(hwnd_, message.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

LRESULT CALLBACK GUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (instance_) {
        return instance_->HandleMessage(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT GUI::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId) {
            case 1: // Hotkey button
                // Will be handled by main application
                break;
            case 2: // Start button
                // Will be handled by main application
                break;
            case 3: // Stop button
                // Will be handled by main application
                break;
            }
        }
        break;
        
    case WM_CTLCOLORSTATIC:
        {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(33, 37, 41)); // Dark text
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
    }
    
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
