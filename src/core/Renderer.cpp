#include "Renderer.hpp"
#include <raylib.h>

void Renderer::draw(const PixelWorld &world)
{
    for (int y = 0; y < world.height(); y++)
    {
        for (int x = 0; x < world.width(); x++)
        {
            const Pixel &p = world.data()[y * world.width() + x];
            Color c = BLACK;
            switch (p.type)
            {
            case PixelType::SAND:
                c = {200, 180, 50, 255};
                break;
            case PixelType::WATER:
                c = {50, 100, 220, 255};
                break;
            case PixelType::STONE:
                c = {120, 120, 120, 255};
                break;
            case PixelType::FIRE:
                c = {255, 80, 20, 255};
                break;
            case PixelType::OIL:
                c = {30, 30, 30, 255};
                break;
            default:
                break;
            }
            if (p.type != PixelType::EMPTY)
            {
                if (p.type == PixelType::FIRE)
                {
                    c.r = GetRandomValue(100, 200);
                    c.g = GetRandomValue(40, 80);
                    c.b = GetRandomValue(10, 20);
                }
                DrawRectangle(x * m_scale, y * m_scale, m_scale, m_scale, c);
            }
        }
    }
}
