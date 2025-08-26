#include "PixelWorld.hpp"
#include <algorithm>
#include <raylib.h>

PixelWorld::PixelWorld(int width, int height)
    : m_width(width), m_height(height), m_pixels(width * height) {}

void PixelWorld::clear()
{
    std::fill(m_pixels.begin(), m_pixels.end(), Pixel{});
}

void PixelWorld::addPixel(int x, int y, PixelType type)
{
    if (x < 0 || y < 0 || x >= m_width || y >= m_height)
        return;
    Pixel &p = m_pixels[idx(x, y)];
    p.type = type;
    if (type == PixelType::FIRE)
    {
        p.lifetime = 2.0f + GetRandomValue(0, 1000) / 1000.0f * 2.0f;
    }
}

void PixelWorld::update(float dt)
{
    // Reset updated flags
    for (auto &pixel : m_pixels)
    {
        pixel.updated = false;
    }

    // bottom-up: sand & water & oil (process heaviest first so they sink properly)
    for (int y = m_height - 2; y >= 0; y--)
    {
        for (int x = 0; x < m_width; x++)
        {
            Pixel &p = m_pixels[idx(x, y)];
            if (p.updated)
                continue;

            // Process sand first (heaviest - sinks through everything)
            if (p.type == PixelType::SAND)
                updateSand(x, y);
        }

        // Second pass for water (heavier than oil - sinks through oil)
        for (int x = 0; x < m_width; x++)
        {
            Pixel &p = m_pixels[idx(x, y)];
            if (p.updated)
                continue;

            if (p.type == PixelType::WATER)
                updateWater(x, y);
        }

        // Third pass for oil (lightest liquid - floats on water)
        for (int x = 0; x < m_width; x++)
        {
            Pixel &p = m_pixels[idx(x, y)];
            if (p.updated)
                continue;

            if (p.type == PixelType::OIL)
                updateOil(x, y);
        }
    }

    // top-down: fire
    for (int y = 1; y < m_height; y++)
    {
        for (int x = 0; x < m_width; x++)
        {
            Pixel &p = m_pixels[idx(x, y)];
            if (p.type == PixelType::FIRE && !p.updated)
            {
                updateFire(x, y, dt);
                p.updated = true;
            }
        }
    }
}

void PixelWorld::updateSand(int x, int y)
{
    static const float GRAVITY = 0.1f;
    static const float MAX_VELOCITY = 5.0f;

    Pixel &cell = m_pixels[idx(x, y)];
    if (cell.updated)
        return;

    // Increase velocity due to gravity
    cell.velocityY = std::min(cell.velocityY + GRAVITY, MAX_VELOCITY);

    // Calculate target position based on velocity
    int targetY = y + static_cast<int>(cell.velocityY);
    targetY = std::min(targetY, m_height - 1);

    // Find the first solid position below (sand can fall through liquids)
    int newY = y + 1;
    while (newY <= targetY && newY < m_height)
    {
        PixelType belowType = m_pixels[idx(x, newY)].type;
        if (belowType != PixelType::EMPTY &&
            belowType != PixelType::WATER &&
            belowType != PixelType::OIL)
        {
            break;
        }
        newY++;
    }
    newY--; // Step back to last valid position

    if (newY > y)
    {
        std::swap(cell, m_pixels[idx(x, newY)]);
    }
    else
    {
        // Try to move diagonally
        int dir = GetRandomValue(0, 1) ? -1 : 1;
        int nx = x + dir;

        if (nx >= 0 && nx < m_width && y + 1 < m_height)
        {
            PixelType belowDiagType = m_pixels[idx(nx, y + 1)].type;
            if (belowDiagType == PixelType::EMPTY ||
                belowDiagType == PixelType::WATER ||
                belowDiagType == PixelType::OIL)
            {
                std::swap(cell, m_pixels[idx(nx, y + 1)]);
                cell.velocityY = 1.0f;
            }
            else
            {
                cell.velocityY = 0;
            }
        }
        else
        {
            cell.velocityY = 0;
        }
    }

    cell.updated = true;
}

void PixelWorld::updateWater(int x, int y)
{
    static const float GRAVITY = 0.05f;
    static const float MAX_VELOCITY = 3.0f;

    Pixel &cell = m_pixels[idx(x, y)];
    if (cell.updated)
        return;

    cell.velocityY = std::min(cell.velocityY + GRAVITY, MAX_VELOCITY);

    int targetY = y + static_cast<int>(cell.velocityY);
    targetY = std::min(targetY, m_height - 1);

    // Water sinks through oil but stops at solids
    int newY = y + 1;
    while (newY <= targetY && newY < m_height)
    {
        PixelType belowType = m_pixels[idx(x, newY)].type;
        if (belowType != PixelType::EMPTY && belowType != PixelType::OIL)
        {
            break;
        }
        newY++;
    }
    newY--;

    if (newY > y)
    {
        std::swap(cell, m_pixels[idx(x, newY)]);
    }
    else
    {
        // Try diagonal movement first
        int dir = GetRandomValue(0, 1) ? -1 : 1;
        int nx = x + dir;

        if (nx >= 0 && nx < m_width && y + 1 < m_height)
        {
            PixelType belowDiagType = m_pixels[idx(nx, y + 1)].type;
            if (belowDiagType == PixelType::EMPTY || belowDiagType == PixelType::OIL)
            {
                std::swap(cell, m_pixels[idx(nx, y + 1)]);
                cell.velocityY = 0.5f;
            }
            else if (nx >= 0 && nx < m_width && m_pixels[idx(nx, y)].type == PixelType::EMPTY)
            {
                // Try horizontal movement
                std::swap(cell, m_pixels[idx(nx, y)]);
                cell.velocityY = 0;
            }
            else
            {
                cell.velocityY = 0;
            }
        }
        else
        {
            cell.velocityY = 0;
        }
    }

    cell.updated = true;
}

void PixelWorld::updateFire(int x, int y, float dt)
{
    static float fireSpreadChance = 0.3f;
    static float fireRiseChance = 0.7f;

    int i = idx(x, y);
    Pixel &cell = m_pixels[i];

    cell.lifetime -= dt * (1.0f + GetRandomValue(0, 100) / 100.0f);

    if (cell.lifetime <= 0 || (cell.lifetime < 0.5f && GetRandomValue(0, 100) < 5))
    {
        cell = {};
        return;
    }

    bool moved = false;

    if (y > 0 && GetRandomValue(0, 100) / 100.0f < fireRiseChance)
    {
        int above = idx(x, y - 1);
        if (m_pixels[above].type == PixelType::EMPTY)
        {
            std::swap(cell, m_pixels[above]);
            moved = true;
        }
    }

    if (!moved && GetRandomValue(0, 100) / 100.0f < fireSpreadChance)
    {
        int dir = GetRandomValue(0, 1) ? -1 : 1;
        int nx = x + dir;

        if (nx >= 0 && nx < m_width)
        {
            int side = idx(nx, y);
            if (m_pixels[side].type == PixelType::EMPTY)
            {
                std::swap(cell, m_pixels[side]);
                moved = true;
            }
        }
    }

    if (cell.lifetime < 0.8f && GetRandomValue(0, 100) < 2)
    {
        cell.type = PixelType::EMPTY;
    }
}

void PixelWorld::updateOil(int x, int y)
{
    static const float GRAVITY = 0.04f;
    static const float MAX_VELOCITY = 2.5f;

    Pixel &cell = m_pixels[idx(x, y)];
    if (cell.updated)
        return;

    // Check for fire nearby and ignite oil
    const int dirs[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
    for (auto &d : dirs)
    {
        int nx = x + d[0];
        int ny = y + d[1];
        if (nx >= 0 && nx < m_width && ny >= 0 && ny < m_height)
        {
            Pixel &neighbor = m_pixels[idx(nx, ny)];
            if (neighbor.type == PixelType::FIRE)
            {
                cell.type = PixelType::FIRE;
                cell.lifetime = 2.0f + GetRandomValue(0, 1000) / 1000.0f * 2.0f;
                cell.velocityY = 0;
                cell.updated = true;
                return;
            }
        }
    }

    // Oil floats on water - check if there's water below and don't fall through it
    if (y + 1 < m_height)
    {
        Pixel &below = m_pixels[idx(x, y + 1)];
        if (below.type == PixelType::WATER)
        {
            // Oil should stay on top of water, don't swap
            // Instead try to move sideways if possible
            int dir = GetRandomValue(0, 1) ? -1 : 1;
            int nx = x + dir;

            if (nx >= 0 && nx < m_width && m_pixels[idx(nx, y)].type == PixelType::EMPTY)
            {
                std::swap(cell, m_pixels[idx(nx, y)]);
                cell.velocityY = 0;
            }
            cell.updated = true;
            return;
        }
    }

    // Normal falling behavior through empty space
    cell.velocityY = std::min(cell.velocityY + GRAVITY, MAX_VELOCITY);
    int targetY = y + static_cast<int>(cell.velocityY);
    targetY = std::min(targetY, m_height - 1);

    int newY = y + 1;
    while (newY <= targetY && newY < m_height)
    {
        PixelType belowType = m_pixels[idx(x, newY)].type;
        if (belowType != PixelType::EMPTY)
            break;
        newY++;
    }
    newY--;

    if (newY > y)
    {
        std::swap(cell, m_pixels[idx(x, newY)]);
    }
    else
    {
        // Try diagonal movement first
        int dir = GetRandomValue(0, 1) ? -1 : 1;
        int nx = x + dir;

        if (nx >= 0 && nx < m_width && y + 1 < m_height &&
            m_pixels[idx(nx, y + 1)].type == PixelType::EMPTY)
        {
            std::swap(cell, m_pixels[idx(nx, y + 1)]);
            cell.velocityY = 0.5f;
        }
        else if (nx >= 0 && nx < m_width && m_pixels[idx(nx, y)].type == PixelType::EMPTY)
        {
            // Try horizontal movement
            std::swap(cell, m_pixels[idx(nx, y)]);
            cell.velocityY = 0;
        }
        else
        {
            cell.velocityY = 0;
        }
    }

    cell.updated = true;
}