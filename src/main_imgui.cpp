#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include <string>
#include <memory>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "process_manager.h"
#include "hotkey_manager.h"
#include "resource.h"

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class TabGlitcherApp {
public:
    TabGlitcherApp();
    ~TabGlitcherApp();

    bool Initialize(HWND hwnd);
    void Render();
    void Shutdown();

private:
    void RenderMainInterface();
    void RenderHotkeySection();
    void RenderStatusSection();
    void RenderInstructions();
    
    void OnSetHotkeyButton();
    void OnStartButton();
    void OnStopButton();
    void OnHotkeyPressed(int key_code, bool is_pressed);

    ProcessManager process_manager_;
    std::unique_ptr<HotkeyManager> hotkey_manager_;
    
    bool is_running_;
    int current_hotkey_;
    std::wstring current_hotkey_name_;
    std::wstring status_text_;
    
    float window_alpha_;
    ImVec4 primary_color_;
    ImVec4 success_color_;
    ImVec4 error_color_;
    ImVec4 warning_color_;
};

TabGlitcherApp::TabGlitcherApp()
    : is_running_(false)
    , current_hotkey_(0)
    , current_hotkey_name_(L"Not Set")
    , status_text_(L"Inactive")
    , window_alpha_(1.0f)
    , primary_color_(ImVec4(0.13f, 0.59f, 0.95f, 1.0f))
    , success_color_(ImVec4(0.13f, 0.75f, 0.38f, 1.0f))
    , error_color_(ImVec4(0.91f, 0.30f, 0.24f, 1.0f))
    , warning_color_(ImVec4(0.95f, 0.61f, 0.07f, 1.0f))
{
    hotkey_manager_ = std::make_unique<HotkeyManager>();
}

TabGlitcherApp::~TabGlitcherApp() {
    if (is_running_) {
        OnStopButton();
    }
}

bool TabGlitcherApp::Initialize(HWND hwnd) {

    if (!hotkey_manager_->Initialize(
        [this](int key_code, bool is_pressed) { OnHotkeyPressed(key_code, is_pressed); }
    )) {
        return false;
    }

    return true;
}

void TabGlitcherApp::Render() {

    if (!hotkey_manager_->IsSettingHotkey() && current_hotkey_ == 0) {
        int captured = hotkey_manager_->GetCapturedHotkey();
        if (captured != 0) {
            current_hotkey_ = captured;
            current_hotkey_name_ = hotkey_manager_->GetHotkeyName();
            status_text_ = L"Hotkey set successfully! Click Start to begin.";
        }
    }
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;
    
    style.WindowPadding = ImVec2(20, 20);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    
    colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.13f, 0.13f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.13f, 0.59f, 0.95f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.59f, 0.95f, 0.67f);
    
    colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.59f, 0.95f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.59f, 0.95f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.59f, 0.95f, 0.50f);
    
    colors[ImGuiCol_Button] = primary_color_;
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.69f, 1.0f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.49f, 0.85f, 1.0f);
    
    colors[ImGuiCol_Header] = ImVec4(0.13f, 0.59f, 0.95f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.59f, 0.95f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.59f, 0.95f, 1.0f);
    
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    RenderMainInterface();
}

void TabGlitcherApp::RenderMainInterface() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Tab Glitcher", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    ImGui::TextColored(primary_color_, "Tab Glitcher");
    ImGui::PopFont();
    ImGui::Text("Roblox Process Suspend Tool");
    ImGui::Separator();
    
    RenderHotkeySection();
    ImGui::Spacing();
    RenderStatusSection();
    ImGui::Spacing();
    RenderInstructions();
    
    ImGui::End();
}

void TabGlitcherApp::RenderHotkeySection() {
    ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "Hotkey Configuration");
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
    
    ImGui::Text("Current Hotkey:");
    ImGui::SameLine();
    
    std::string hotkey_str;
    for (wchar_t wc : current_hotkey_name_) {
        if (wc == 0 || wc > 127) break;
        hotkey_str += (char)wc;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.13f, 0.75f, 0.38f, 1.0f));
    ImGui::Text("%s", hotkey_str.c_str());
    ImGui::PopStyleColor();
    
    if (ImGui::Button("Set Hotkey", ImVec2(120, 40))) {
        OnSetHotkeyButton();
    }
    
    ImGui::SameLine();
    
    if (is_running_) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        ImGui::Button("Start", ImVec2(100, 40));
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    } else {
        if (current_hotkey_ == 0) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
            ImGui::Button("Start", ImVec2(100, 40));
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        } else {
            if (ImGui::Button("Start", ImVec2(100, 40))) {
                OnStartButton();
            }
        }
    }
    
    ImGui::SameLine();
    
    if (!is_running_) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        ImGui::Button("Stop", ImVec2(100, 40));
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    } else {
        if (ImGui::Button("Stop", ImVec2(100, 40))) {
            OnStopButton();
        }
    }
    
    ImGui::PopStyleVar();
}

void TabGlitcherApp::RenderStatusSection() {
    ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "Status");
    
    ImVec4 status_color = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    if (status_text_.find(L"Suspended") != std::wstring::npos) {
        status_color = warning_color_;
    } else if (status_text_.find(L"Resumed") != std::wstring::npos) {
        status_color = success_color_;
    } else if (status_text_.find(L"Monitoring") != std::wstring::npos) {
        status_color = primary_color_;
    } else if (status_text_.find(L"Error") != std::wstring::npos || status_text_.find(L"Cannot") != std::wstring::npos) {
        status_color = error_color_;
    }
    
    std::string status_str;
    for (wchar_t wc : status_text_) {
        if (wc == 0 || wc > 127) break;
        status_str += (char)wc;
    }
    
    ImGui::TextColored(status_color, "%s", status_str.c_str());
    
    if (is_running_ && process_manager_.IsProcessOpen()) {
        ImGui::Spacing();
        ImGui::Text("Process ID: %lu", process_manager_.GetProcessId());
        ImGui::Text("Process: RobloxPlayerBeta.exe");
    }
    
    if (hotkey_manager_->IsSettingHotkey()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "DEBUG: Waiting for hotkey input...");
        ImGui::Text("Current hotkey code: %d", current_hotkey_);
    }
}

void TabGlitcherApp::RenderInstructions() {
    ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "Instructions");
    
    ImGui::Text("1. Click 'Set Hotkey' and press any key or mouse button");
    ImGui::Text("2. Click 'Start' to begin monitoring Roblox process");
    ImGui::Text("3. Hold your hotkey to suspend Roblox (tab glitch effect)");
    ImGui::Text("4. Release hotkey to resume Roblox process");
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.95f, 0.61f, 0.07f, 1.0f), "[INFO] Make sure Roblox is running before starting");
}

void TabGlitcherApp::OnSetHotkeyButton() {
    if (is_running_) {
        status_text_ = L"Cannot change hotkey while monitoring is active!";
        return;
    }

    hotkey_manager_->SetHotkeyMode(true);
    status_text_ = L"Press any key or mouse button to set hotkey...";
    current_hotkey_ = 0;
    current_hotkey_name_ = L"Waiting...";
}

void TabGlitcherApp::OnStartButton() {
    if (is_running_) {
        return;
    }

    current_hotkey_ = hotkey_manager_->GetCapturedHotkey();
    if (current_hotkey_ == 0) {
        status_text_ = L"Please set a hotkey first!";
        return;
    }

    if (!process_manager_.FindRobloxProcess()) {
        status_text_ = L"Error - Roblox not found";
        return;
    }

    is_running_ = true;
    current_hotkey_name_ = hotkey_manager_->GetHotkeyName();
    status_text_ = L"Monitoring Roblox...";
}

void TabGlitcherApp::OnStopButton() {
    if (!is_running_) {
        return;
    }

    is_running_ = false;
    process_manager_.CloseProcess();
    hotkey_manager_->SetHotkeyMode(false);
    status_text_ = L"Inactive";
}

void TabGlitcherApp::OnHotkeyPressed(int key_code, bool is_pressed) {
    if (!is_running_ || key_code != current_hotkey_) {
        return;
    }

    if (is_pressed) {
        if (process_manager_.SuspendProcess()) {
            status_text_ = L"Process Suspended";
        }
    } else {
        if (process_manager_.ResumeProcess()) {
            status_text_ = L"Process Resumed";
        }
    }
}

void TabGlitcherApp::Shutdown() {
    if (is_running_) {
        OnStopButton();
    }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static std::unique_ptr<TabGlitcherApp> g_app;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {

    HICON hIcon = static_cast<HICON>(LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_APPICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = hIcon;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"Tab Glitcher";
    wc.hIconSm = hIcon;
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Tab Glitcher - Roblox Process Suspend Tool", WS_OVERLAPPEDWINDOW, 100, 100, 600, 450, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImFont* font_default = io.Fonts->AddFontDefault();
    ImFont* font_large = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    ImFont* font_title = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 24.0f);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    g_app = std::make_unique<TabGlitcherApp>();
    if (!g_app->Initialize(hwnd)) {
        return 1;
    }

    bool done = false;
    while (!done) {

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        g_app->Render();

        ImGui::Render();
        const float clear_color[4] = { 0.06f, 0.06f, 0.06f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    g_app->Shutdown();
    g_app.reset();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
