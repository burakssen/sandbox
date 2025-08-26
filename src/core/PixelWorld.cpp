#include "PixelWorld.hpp"
#include <algorithm>

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
    for (auto &pixel : m_pixels) {
        pixel.updated = false;
    }

    // bottom-up: sand & water
    for (int y = m_height - 2; y >= 0; y--)
    {
        for (int x = 0; x < m_width; x++)
        {
            Pixel &p = m_pixels[idx(x, y)];
            if (p.type == PixelType::SAND)
                updateSand(x, y);
            else if (p.type == PixelType::WATER)
                updateWater(x, y);
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
    if (cell.updated) return;
    
    // Increase velocity due to gravity
    cell.velocityY = std::min(cell.velocityY + GRAVITY, MAX_VELOCITY);
    
    // Calculate target position based on velocity
    int targetY = y + static_cast<int>(cell.velocityY);
    targetY = std::min(targetY, m_height - 1);
    
    // Find the first solid position below
    int newY = y + 1;
    while (newY <= targetY && newY < m_height) {
        if (m_pixels[idx(x, newY)].type != PixelType::EMPTY && 
            m_pixels[idx(x, newY)].type != PixelType::WATER) {
            break;
        }
        newY++;
    }
    newY--; // Step back to last valid position
    
    if (newY > y) {
        // Move to the new position
        std::swap(cell, m_pixels[idx(x, newY)]);
    } 
    else {
        // Try to move diagonally
        int dir = GetRandomValue(0, 1) ? -1 : 1;
        int nx = x + dir;
        
        if (nx >= 0 && nx < m_width && 
            (m_pixels[idx(nx, y + 1)].type == PixelType::EMPTY || 
             m_pixels[idx(nx, y + 1)].type == PixelType::WATER)) {
            std::swap(cell, m_pixels[idx(nx, y + 1)]);
            cell.velocityY = 1.0f; // Reset velocity when moving diagonally
        } else {
            cell.velocityY = 0; // Stop if can't move
        }
    }
    
    cell.updated = true;
}

void PixelWorld::updateWater(int x, int y)
{
    static const float GRAVITY = 0.05f;
    static const float MAX_VELOCITY = 3.0f;
    
    Pixel &cell = m_pixels[idx(x, y)];
    if (cell.updated) return;
    
    // Increase velocity due to gravity (slower than sand)
    cell.velocityY = std::min(cell.velocityY + GRAVITY, MAX_VELOCITY);
    
    // Calculate target position based on velocity
    int targetY = y + static_cast<int>(cell.velocityY);
    targetY = std::min(targetY, m_height - 1);
    
    // Find the first solid position below
    int newY = y + 1;
    while (newY <= targetY && newY < m_height) {
        if (m_pixels[idx(x, newY)].type != PixelType::EMPTY) {
            break;
        }
        newY++;
    }
    newY--; // Step back to last valid position
    
    if (newY > y) {
        // Move to the new position
        std::swap(cell, m_pixels[idx(x, newY)]);
    } 
    else {
        // Try to move diagonally
        int dir = GetRandomValue(0, 1) ? -1 : 1;
        int nx = x + dir;
        
        if (nx >= 0 && nx < m_width && m_pixels[idx(nx, y + 1)].type == PixelType::EMPTY) {
            std::swap(cell, m_pixels[idx(nx, y + 1)]);
            cell.velocityY = 0.5f; // Reset velocity when moving diagonally
        } 
        // If can't move down, try to spread horizontally
        else if (y < m_height - 1) {
            dir = GetRandomValue(0, 1) ? -1 : 1;
            nx = x + dir;
            
            if (nx >= 0 && nx < m_width) {
                int side = idx(nx, y);
                if (m_pixels[side].type == PixelType::EMPTY) {
                    std::swap(cell, m_pixels[side]);
                    cell.velocityY = 0; // Reset velocity when spreading
                }
            }
        } else {
            cell.velocityY = 0; // Stop if can't move
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
    
    // Fire dissipates faster based on time
    cell.lifetime -= dt * (1.0f + GetRandomValue(0, 100) / 100.0f);
    
    // Randomly extinguish based on lifetime
    if (cell.lifetime <= 0 || (cell.lifetime < 0.5f && GetRandomValue(0, 100) < 5)) {
        cell = {};
        return;
    }
    
    // Randomly move up or to the sides
    bool moved = false;
    
    // Try to move up first
    if (y > 0 && GetRandomValue(0, 100) / 100.0f < fireRiseChance) {
        int above = idx(x, y - 1);
        if (m_pixels[above].type == PixelType::EMPTY) {
            std::swap(cell, m_pixels[above]);
            moved = true;
        }
    }
    
    // If didn't move up, try to spread horizontally
    if (!moved && GetRandomValue(0, 100) / 100.0f < fireSpreadChance) {
        int dir = GetRandomValue(0, 1) ? -1 : 1;
        int nx = x + dir;
        
        if (nx >= 0 && nx < m_width) {
            int side = idx(nx, y);
            if (m_pixels[side].type == PixelType::EMPTY) {
                std::swap(cell, m_pixels[side]);
                moved = true;
            }
        }
    }
    
    // Randomly create smoke/ash when fire is about to go out
    if (cell.lifetime < 0.8f && GetRandomValue(0, 100) < 2) {
        cell.type = PixelType::EMPTY;
    }
}
