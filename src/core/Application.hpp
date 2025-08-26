#pragma once
#include "PixelWorld.hpp"
#include "Renderer.hpp"
#include <raylib.h>
#include <raygui.h>

class Application
{
public:
    Application(int width, int height, const char *title);
    ~Application();
    void run();
    void frame();

private:
    Vector2 getScaledMousePosition();
    void processInput();

    int m_width, m_height;
    int m_scale;
    const char *m_title;
    bool m_running = true;

    PixelType m_currentType = PixelType::SAND;
    PixelWorld m_world;
    Renderer m_renderer;
    bool m_guiLock = false; // Controls whether GUI is locked (not visible)
};
