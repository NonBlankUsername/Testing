#include "Interface.hpp"
#include "Visuals/Visuals.hpp"
#include "../keybinds.hpp"
#include "../Globals.hpp"


namespace Interface {
    std::unique_ptr<CRenderer> Renderer = std::make_unique<CRenderer>();

    CRenderer::CRenderer() noexcept = default;
    CRenderer::~CRenderer() noexcept {
        Shutdown();
    }

    void CRenderer::Initialize() noexcept {
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

        ImGui_ImplWin32_Init(Window);
        ImGui_ImplDX11_Init(Device.Get(), Context.Get());

        SetStyle();
    }

    LRESULT CRenderer::HandleWindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) noexcept {
        if (Initialized && ImGui_ImplWin32_WndProcHandler(Hwnd, Msg, WParam, LParam))
            return true;

        if (GetAsyncKeyState(VK_INSERT) & 1)
            Globals::Interface::Open = !Globals::Interface::Open;

        return CallWindowProc(OriginalWndProc, Hwnd, Msg, WParam, LParam);
    }

    HRESULT CRenderer::HandlePresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) noexcept {
        if (!Initialized) {
            if (SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(Device.GetAddressOf())))) {
                Device->GetImmediateContext(Context.GetAddressOf());

                DXGI_SWAP_CHAIN_DESC Sd;
                SwapChain->GetDesc(&Sd);
                Window = Sd.OutputWindow;

                Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBuffer;

                SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(BackBuffer.GetAddressOf()));
                Device->CreateRenderTargetView(BackBuffer.Get(), nullptr, RenderView.GetAddressOf());

                OriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(static_cast<WNDPROC>([](HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) -> LRESULT { return Renderer->HandleWindowProc(Hwnd, Msg, WParam, LParam); }))));

                Initialize();
                Initialized = true;
            }
            else return OriginalPresent(SwapChain, SyncInterval, Flags);
        }

        if (GetAsyncKeyState(VK_END) & 1) {
            Shutdown();
            return OriginalPresent(SwapChain, SyncInterval, Flags);
        }

        {
            std::lock_guard<std::mutex> lock(RenderMutex);
            RenderFrame();
        }

        return OriginalPresent(SwapChain, SyncInterval, Flags);
    }

    void CRenderer::SetStyle() noexcept {
        ImGuiStyle& Style = ImGui::GetStyle();


        ImVec4* Colors = Style.Colors;

    }

    void CRenderer::RenderFrame() noexcept {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        if (Globals::ESP::Enabled)
            Visuals::Players();

        if (Globals::Interface::Open)
            RenderMenu();

        ImGui::EndFrame();
        ImGui::Render();
        Context->OMSetRenderTargets(1, RenderView.GetAddressOf(), nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }


    void CRenderer::RenderMenu() noexcept {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

        ImGui::Begin("Desire", &Globals::Interface::Open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        ImVec2 window_pos = ImGui::GetWindowPos();

  
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, 120);

        static int CurrentTab = 0;
        const char* Tabs[] = { "Legit", "Visuals", "Movement", "Misc" };

        ImGui::BeginChild("TabSection", ImVec2(120, 0), true);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

        for (int i = 0; i < IM_ARRAYSIZE(Tabs); ++i) {
            ImGui::SetCursorPosX(10);
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            if (ImGui::Button(Tabs[i], ImVec2(100, 30))) {
                CurrentTab = i;
            }
            ImGui::PopFont();
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::EndChild();

        ImGui::NextColumn();

        ImGui::BeginChild("MainSection", ImVec2(0, 0), true);

        if (CurrentTab == 0) {
            ImGui::Text("Aimbot Settings");
            ImGui::Separator();
            ImGui::Text("Soon");
        }
        else if (CurrentTab == 1) {
            ImGui::Text("ESP Settings");
            ImGui::Separator();
            ImGui::Checkbox("Enabled", &Globals::ESP::Enabled);
            ImGui::Separator();
            ImGui::Checkbox("Box", &Globals::ESP::Box);
            ImGui::Checkbox("Box Fill", &Globals::ESP::FilledBox);
            ImGui::Checkbox("Distance ESP", &Globals::ESP::Distance);
            ImGui::Checkbox("Name ESP", &Globals::ESP::Name);
        }
        else if (CurrentTab == 2) {
            ImGui::Text("Movement Settings");
            ImGui::Separator();
            ImGui::Text("Coming Soon");
        }
        else if (CurrentTab == 3) {
            ImGui::Text("Misc Settings");
            ImGui::Separator();
            ImGui::Checkbox("Team check", &Globals::Misc::TeamCheck);
            ImGui::Separator();

            ImGui::Text("Colors");
            ImGui::Separator();
            ImGui::Text("Box"); ImGui::SameLine(150); ImGui::ColorEdit3("##BoxColor", (float*)&Globals::ESP::BoxColor, ImGuiColorEditFlags_NoInputs);
            ImGui::Text("Name"); ImGui::SameLine(150); ImGui::ColorEdit3("##NameColor", (float*)&Globals::ESP::NameColor, ImGuiColorEditFlags_NoInputs);
            ImGui::Text("Health Text"); ImGui::SameLine(150); ImGui::ColorEdit3("##HealthTextColor", (float*)&Globals::ESP::HealthTextColor, ImGuiColorEditFlags_NoInputs);
        }

        ImGui::EndChild();
        ImGui::Columns(1);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
        ImGui::End();
    }





    void CRenderer::Shutdown() noexcept {
        if (!Initialized) return;

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (Window)
            SetWindowLongPtr(Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(OriginalWndProc));

        RenderView.Reset();
        Context.Reset();
        Device.Reset();

        Initialized = false;
    }
}