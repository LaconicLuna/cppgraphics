#include "renderer.h"
#include "font.h"

void PutPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
        framebuffer[y * WIDTH + x] = color;
}

void PutPixelAlpha(int x, int y, uint32_t color, float alpha) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;

    uint32_t bg = framebuffer[y * WIDTH + x];

    uint8_t rB = (bg >> 16) & 0xFF;
    uint8_t gB = (bg >> 8) & 0xFF;
    uint8_t bB = bg & 0xFF;

    uint8_t rF = (color >> 16) & 0xFF;
    uint8_t gF = (color >> 8) & 0xFF;
    uint8_t bF = color & 0xFF;

    uint8_t r = rF * alpha + rB * (1 - alpha);
    uint8_t g = gF * alpha + gB * (1 - alpha);
    uint8_t b = bF * alpha + bB * (1 - alpha);

    framebuffer[y * WIDTH + x] = (r << 16) | (g << 8) | b;
}

void DrawSprite(int x, int y, const Texture& tex) {
    for (int py = 0; py < tex.height; py++) {
        int sy = y + py;
        if (sy < 0 || sy >= HEIGHT) continue;

        for (int px = 0; px < tex.width; px++) {
            int sx = x + px;
            if (sx < 0 || sx >= WIDTH) continue;

            uint32_t color = tex.pixels[py * tex.width + px];
            uint8_t a = (color >> 24) & 0xFF;
            if (a == 0) continue;

            PutPixelAlpha(sx, sy, color, a / 255.0f);
        }
    }
}

void DrawChar(int x, int y, char c, uint8_t r, uint8_t g, uint8_t b) {
    if (c >= 'a' && c <= 'z') c -= 32;

    const unsigned char* glyph = font8x8[(unsigned char)c];

    for (int ky = 0; ky < 8; ky++)
        for (int kx = 0; kx < 8; kx++)
            if (glyph[ky] & (1 << kx))
                PutPixel(x + kx, y + ky, (r << 16) | (g << 8) | b);
}

void DrawString(int x, int y, const char* str, uint8_t r, uint8_t g, uint8_t b) {
    while (*str) {
        DrawChar(x, y, *str, r, g, b);
        x += 8;
        str++;
    }
}
