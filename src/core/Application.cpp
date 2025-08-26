#include "Application.hpp"
#include <raylib.h>
#include <algorithm>
#include <math.h>

Application::Application(int width, int height, const char *title)
    : m_width(width), m_height(height), m_title(title),
      m_world(width / 2, height / 2),
      m_renderer(2)
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(m_width, m_height, m_title);

    // Initialize raygui
    GuiLoadStyleDefault();

    // Set GUI style
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

    SetTargetFPS(120);
    m_scale = 2;
}

Application::~Application() { CloseWindow(); }

// Forward declaration for Emscripten callback
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

// Static wrapper function for Emscripten main loop
static void emscriptenMainLoop(void* app) {
    static_cast<Application*>(app)->frame();
}
#endif

void Application::frame() {
    if (!m_running || WindowShouldClose()) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
        #endif
        return;
    }

    // Update window dimensions if resized
    int newWidth = GetScreenWidth();
    int newHeight = GetScreenHeight();
    if (newWidth != m_width || newHeight != m_height) {
        m_width = newWidth;
        m_height = newHeight;
        m_renderer.setScale(m_scale);
    }

    // Process input
    processInput();
    
    // Update world state
    m_world.update(GetFrameTime());

    // Render frame
    BeginDrawing();
    ClearBackground(BLACK);

    // Draw the world
    m_renderer.draw(m_world);

    // Draw GUI
    if (!m_guiLock) {
        GuiLock();
        processInput(); // This will draw the GUI
        GuiUnlock();
    } else {
        // If GUI is locked, still process input but don't draw
        processInput();
    }

    // Draw FPS on top of everything
    DrawFPS(m_width - 85, 10);

    EndDrawing();
}

void Application::run() {
    #ifdef __EMSCRIPTEN__
    // Use browser's requestAnimationFrame for WebAssembly
    emscripten_set_main_loop_arg(emscriptenMainLoop, this, 0, 1);
    #else
    // Standard desktop game loop
    while (m_running && !WindowShouldClose()) {
        frame();
    }
    #endif
}

Vector2 Application::getScaledMousePosition()
{
    static Vector2 mouse;
    mouse = GetMousePosition();

    // Cache screen dimensions
    static int lastScreenWidth = 0;
    static int lastScreenHeight = 0;
    static float invScale = 1.0f / static_cast<float>(m_scale);
    
    // Update cache if screen size or scale changes
    int currentWidth = GetScreenWidth();
    int currentHeight = GetScreenHeight();
    if (currentWidth != lastScreenWidth || currentHeight != lastScreenHeight || invScale != 1.0f/static_cast<float>(m_scale))
    {
        lastScreenWidth = currentWidth;
        lastScreenHeight = currentHeight;
        invScale = 1.0f / static_cast<float>(m_scale);
    }

#if !(defined(PLATFORM_WEB) || defined(__EMSCRIPTEN__))
    // Only apply scale on desktop
    mouse.x *= invScale;
    mouse.y *= invScale;
#else
    // For WebAssembly, map to world coordinates
    float sx = static_cast<float>(m_world.width()) / static_cast<float>(currentWidth);
    float sy = static_cast<float>(m_world.height()) / static_cast<float>(currentHeight);
    mouse.x *= sx;
    mouse.y *= sy;
#endif

    return mouse;
}

void Application::processInput()
{
    // Set up button styles
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    GuiSetStyle(BUTTON, TEXT_PADDING, 10);

    // Define button colors
    Color btnBorderColor = Fade(LIGHTGRAY, 0.5f);
    Color btnTextColor = WHITE;

    // Define button rectangles
    Rectangle sandBtn = {10, 40, 100, 30};
    Rectangle waterBtn = {120, 40, 100, 30};
    Rectangle stoneBtn = {10, 80, 100, 30};
    Rectangle fireBtn = {120, 80, 100, 30};
    Rectangle clearBtn = {10, 120, 210, 30};

    // Set button colors based on current type
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(btnBorderColor));
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Fade(DARKGRAY, 0.6f)));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(btnTextColor));

    // Set hover colors
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt(GOLD));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(Fade(GOLD, 0.2f)));

    // Set pressed colors
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, ColorToInt(GOLD));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(Fade(GOLD, 0.4f)));

    // Set active button color
    Color activeColor = Fade(GREEN, 0.4f);
    if (m_currentType == PixelType::SAND)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(activeColor));

    // Get scaled mouse position
    Vector2 mousePos = getScaledMousePosition();

    // Check if mouse is over any GUI element
    bool mouseOverGUI = CheckCollisionPointRec(mousePos, sandBtn) ||
                       CheckCollisionPointRec(mousePos, waterBtn) ||
                       CheckCollisionPointRec(mousePos, stoneBtn) ||
                       CheckCollisionPointRec(mousePos, fireBtn) ||
                       CheckCollisionPointRec(mousePos, clearBtn);

    // Update GUI lock state based on mouse interaction with GUI elements
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
        // Only lock if clicking on GUI elements
        m_guiLock = mouseOverGUI;
    }
    else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
    {
        m_guiLock = false;
    }

    // Allow particle creation if not over GUI elements or if we're already drawing
    bool canCreateParticles = !mouseOverGUI || (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !m_guiLock);

    // Handle mouse input for drawing particles
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && canCreateParticles)
    {
        Vector2 mouse = getScaledMousePosition();
        int centerX = mouse.x;
        int centerY = mouse.y;

        // Create a more natural distribution of particles
        int radius = 10;
        int radiusSq = radius * radius;
        int numParticles = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 5 : 20; // More particles for right click

        for (int i = 0; i < numParticles; i++)
        {
            // Create a more natural distribution using polar coordinates
            float angle = GetRandomValue(0, 628) / 100.0f; // 0-2π in radians * 100
            float dist = GetRandomValue(0, radius * 100) / 100.0f;

            // Convert to cartesian coordinates
            int x = centerX + (int)(cosf(angle) * dist);
            int y = centerY + (int)(sinf(angle) * dist);

            // Add some randomness to the position for a more natural look
            x += GetRandomValue(-1, 1);
            y += GetRandomValue(-1, 1);

            // Only add if within world bounds
            if (x >= 0 && x < m_world.width() && y >= 0 && y < m_world.height())
            {
                m_world.addPixel(x, y, m_currentType);
            }
        }
    }

    // Set scissor mode just for GUI elements
    // Note: raylib's scissor mode is global, so we'll just set it for GUI
    // and then disable it after we're done
    BeginScissorMode(0, 0, 240, 160);

    // Sand Button
    if (m_currentType == PixelType::SAND)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(activeColor));
    else
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Fade(DARKGRAY, 0.6f)));
    if (GuiButton(sandBtn, "Sand"))
    {
        m_currentType = PixelType::SAND;
    }

    // Water Button
    if (m_currentType == PixelType::WATER)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(activeColor));
    else
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Fade(DARKGRAY, 0.6f)));
    if (GuiButton(waterBtn, "Water"))
    {
        m_currentType = PixelType::WATER;
    }

    // Stone Button
    if (m_currentType == PixelType::STONE)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(activeColor));
    else
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Fade(DARKGRAY, 0.6f)));
    if (GuiButton(stoneBtn, "Stone"))
    {
        m_currentType = PixelType::STONE;
    }

    // Fire Button
    if (m_currentType == PixelType::FIRE)
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(activeColor));
    else
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Fade(DARKGRAY, 0.6f)));
    if (GuiButton(fireBtn, "Fire"))
    {
        m_currentType = PixelType::FIRE;
    }

    // Clear Button (special style)
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(Fade(RED, 0.6f)));
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt(RED));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(Fade(RED, 0.3f)));
    if (GuiButton(clearBtn, "Clear All"))
    {
        m_world.clear();
    }

    // Disable scissor mode after GUI drawing
    EndScissorMode();

    // Draw current selection indicator
    const char *currentTypeText = "";
    switch (m_currentType)
    {
    case PixelType::SAND:
        currentTypeText = "Current: Sand";
        break;
    case PixelType::WATER:
        currentTypeText = "Current: Water";
        break;
    case PixelType::STONE:
        currentTypeText = "Current: Stone";
        break;
    case PixelType::FIRE:
        currentTypeText = "Current: Fire";
        break;
    case PixelType::EMPTY:
        currentTypeText = "Current: Eraser";
        break;
    }
    DrawText(currentTypeText, 10, 10, 20, WHITE);

    // Only process particle creation if not interacting with GUI
    if (!m_guiLock && (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)))
    {
        Vector2 mouse = getScaledMousePosition();
        int centerX = mouse.x;
        int centerY = mouse.y;

        // Create a more natural distribution of particles
        int radius = 10;
        int radiusSq = radius * radius;
        int numParticles = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 5 : 20; // More particles for right click

        for (int i = 0; i < numParticles; i++)
        {
            // Create a more natural distribution using polar coordinates
            float angle = GetRandomValue(0, 628) / 100.0f; // 0-2π in radians * 100
            float dist = GetRandomValue(0, radius * 100) / 100.0f;

            // Convert to cartesian coordinates
            int x = centerX + (int)(cosf(angle) * dist);
            int y = centerY + (int)(sinf(angle) * dist);

            // Add some randomness to the position for a more natural look
            x += GetRandomValue(-1, 1);
            y += GetRandomValue(-1, 1);

            // Only add if within world bounds
            if (x >= 0 && x < m_world.width() && y >= 0 && y < m_world.height())
            {
                m_world.addPixel(x, y, m_currentType);
            }
        }
    }
    // Clear with C key (kept for convenience)
    if (IsKeyPressed(KEY_C))
        m_world.clear();
}
