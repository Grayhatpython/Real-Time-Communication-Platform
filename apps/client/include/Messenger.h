#pragma once

class GLFWwindow;
class Messenger
{
public:
    bool Initialize();
    void Run();

private:
    void ApplyStyle();
    void RenderLayout();
    void RenderAuthPanel();

private:
    GLFWwindow* _window = nullptr;
};