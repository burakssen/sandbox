#pragma once
#include "PixelWorld.hpp"

class Renderer
{
public:
    Renderer(int scale) : m_scale(scale) {}
    void draw(const PixelWorld &world);
    void setScale(int scale) { m_scale = scale; }

private:
    int m_scale;
};
