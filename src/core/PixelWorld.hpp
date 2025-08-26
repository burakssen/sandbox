#pragma once
#include <vector>
#include <raylib.h>

enum class PixelType
{
    EMPTY,
    SAND,
    WATER,
    STONE,
    FIRE
};

// Packed struct for better memory efficiency
struct alignas(8) Pixel
{
    PixelType type : 4;    // 4 bits for type (0-15 possible values)
    bool updated : 1;      // 1 bit for update flag
    float lifetime;        // 4 bytes for lifetime
    float velocityY;       // 4 bytes for vertical velocity
    
    Pixel() : type(PixelType::EMPTY), updated(false), lifetime(0.0f), velocityY(0.0f) {}
};

class PixelWorld
{
public:
    PixelWorld(int width, int height);
    void clear();
    void addPixel(int x, int y, PixelType type);
    void update(float dt);

    int width() const { return m_width; }
    int height() const { return m_height; }
    const std::vector<Pixel> &data() const { return m_pixels; }

private:
    int m_width, m_height;
    std::vector<Pixel> m_pixels;

    void updateSand(int x, int y);
    void updateWater(int x, int y);
    void updateFire(int x, int y, float dt);

    inline int idx(int x, int y) const { return y * m_width + x; }
};
