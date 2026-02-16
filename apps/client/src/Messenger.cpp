#include "Pch.h"
#include "Messenger.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include "Protocol.h"

bool Messenger::Initialize()
{
   if (!glfwInit())
        return false;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    _window = glfwCreateWindow(1280, 720, "Messenger", nullptr, nullptr);
    if (_window == nullptr)
        return false;

    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Init 단계에서, NewFrame 전에

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void Messenger::Run()
{
    ApplyStyle();

    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 전체 도킹
        ImGui::DockSpaceOverViewport();

        // 왼쪽/상단 같은 느낌의 단일 창
        ImGui::Begin("Discord", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

        RenderAuthPanel();

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(_window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(_window);
    glfwTerminate();

    _window = nullptr;
}

void Messenger::ApplyStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.ItemSpacing = ImVec2(10, 8);
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(10, 6);

    auto& color = style.Colors;
    color[ImGuiCol_WindowBg]        = ImVec4(0.12f, 0.13f, 0.16f, 1.00f); // dark
    color[ImGuiCol_ChildBg]         = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    color[ImGuiCol_PopupBg]         = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    color[ImGuiCol_Border]          = ImVec4(0.20f, 0.22f, 0.26f, 1.00f);

    color[ImGuiCol_FrameBg]         = ImVec4(0.18f, 0.19f, 0.23f, 1.00f);
    color[ImGuiCol_FrameBgHovered]  = ImVec4(0.22f, 0.24f, 0.29f, 1.00f);
    color[ImGuiCol_FrameBgActive]   = ImVec4(0.25f, 0.27f, 0.33f, 1.00f);

    color[ImGuiCol_Button]          = ImVec4(0.35f, 0.39f, 0.95f, 1.00f);
    color[ImGuiCol_ButtonHovered]   = ImVec4(0.40f, 0.44f, 1.00f, 1.00f);
    color[ImGuiCol_ButtonActive]    = ImVec4(0.30f, 0.34f, 0.90f, 1.00f);

    color[ImGuiCol_Header]          = ImVec4(0.18f, 0.19f, 0.23f, 1.00f);
    color[ImGuiCol_HeaderHovered]   = ImVec4(0.22f, 0.24f, 0.29f, 1.00f);
    color[ImGuiCol_HeaderActive]    = ImVec4(0.25f, 0.27f, 0.33f, 1.00f);

    color[ImGuiCol_Text]            = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);
    color[ImGuiCol_TextDisabled]    = ImVec4(0.60f, 0.62f, 0.67f, 1.00f);
}

void Messenger::RenderLayout()
{
    // left server bar
    ImGui::BeginChild("Servers", ImVec2(72, 0), true);
    ImGui::Text("S");
    ImGui::Separator();
    ImGui::Button("  +  ", ImVec2(-1, 44));
    ImGui::Button("  #  ", ImVec2(-1, 44));
    ImGui::EndChild();

    ImGui::SameLine();

    //  TEMP
    uint64 userId = 0;
    // channels
    ImGui::BeginChild("Channels", ImVec2(240, 0), true);
    ImGui::Text("Guild");
    ImGui::Separator();
    ImGui::Selectable("# general", true);
    ImGui::Selectable("# auth-test");
    // ImGui::Separator();
    // ImGui::TextDisabled("userId: %llu", /*(unsigned long long)session.GetUserId()*/userId);
    ImGui::EndChild();

    ImGui::SameLine();

    // main chat
    ImGui::BeginChild("Chat", ImVec2(0, 0), true);

    ImGui::Text("auth-test");
    ImGui::Separator();

    ImGui::BeginChild("ChatScroll", ImVec2(0, -60), false);
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();

    static char input[512] = {};
    ImGui::Separator();
    ImGui::SetNextItemWidth(-90);
    ImGui::InputTextWithHint("##msg", "Message #auth-test", input, sizeof(input));
    ImGui::SameLine();
    if (ImGui::Button("Send", ImVec2(80, 0)))
    {
        // 아직 메시지 패킷은 다음 스텝에서
        input[0] = '\0';
    }

    ImGui::EndChild();
}

void Messenger::RenderAuthPanel()
{
    std::string host = "127.0.0.1";
    host.reserve(64);
    int port = 9000;

    ImGui::Text("Connection");
    ImGui::SetNextItemWidth(220);
    ImGui::InputText("Host", host.data(), host.capacity()+1); // host는 reserve 해둬야 안전함
    ImGui::SetNextItemWidth(120);
    ImGui::InputInt("Port", &port);

    if (/*!session*/false)
    {
        if (ImGui::Button("Connect", ImVec2(140, 0)))
        {
       
        }
        if (ImGui::BeginPopupModal("ConnectError", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextWrapped("ConnectToServer() is not implemented.\nBind it to your network engine client connect API.");
            if (ImGui::Button("OK")) 
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        return;
    }

    ImGui::SameLine();
    if (ImGui::Button("Disconnect", ImVec2(140, 0)))
    {

        return;
    }

    ImGui::Separator();

    static char username[64] = {};
    static char password[64] = {};
    static char token[256]   = {};

    ImGui::Text("Auth");
    if (ImGui::BeginTabBar("AuthTabs"))
    {
        if (ImGui::BeginTabItem("Login"))
        {
            ImGui::InputText("Username", username, sizeof(username));
            ImGui::InputText("Password", password, sizeof(password), ImGuiInputTextFlags_Password);

            if (ImGui::Button("Login", ImVec2(160, 0)))
            {

            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Register"))
        {
            ImGui::InputText("Username", username, sizeof(username));
            ImGui::InputText("Password", password, sizeof(password), ImGuiInputTextFlags_Password);

            if (ImGui::Button("Register", ImVec2(160, 0)))
            {

            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Resume"))
        {
            // 저장된 토큰을 편하게 가져오기
            if (ImGui::Button("Load saved token"))
            {
      
            }

            ImGui::InputTextMultiline("Token", token, sizeof(token), ImVec2(-1, 90));

            if (ImGui::Button("Resume", ImVec2(160, 0)))
            {

            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();

    // AuthOk에서 token이 오면 저장 (로그가 들어온다고 가정)
    // 더 정확히 하려면 ClientSession에 "마지막 token 업데이트 여부" 플래그를 두면 됨.
    if (ImGui::Button("Save last token", ImVec2(160, 0)))
    {
    
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear saved token", ImVec2(160, 0)))
    {
    
    }
}