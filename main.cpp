#include <windows.h>
#include <cstdint>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include "font.h"
#include <fstream>
#include <iostream> // For error logging if a file missing

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float dt = 0;
int WIDTH = 1080; 
int HEIGHT = 720;


uint32_t* framebuffer = nullptr;
HWND globalHwnd = NULL;
BITMAPINFO bmi = {};

const int MAP_X = 1024;
const int MAP_Y = 1024;







struct Texture {
    uint32_t* pixels = nullptr;
    int width = 0; int height = 0;
    ~Texture() { if (pixels) stbi_image_free(pixels); }
};

void LoadPNG(const char* filename, Texture& outTex) {
    int channels;
    outTex.pixels = (uint32_t*)stbi_load(filename, &outTex.width, &outTex.height, &channels, 4);
    if (!outTex.pixels) {
        outTex.width = 16; outTex.height = 16;
        outTex.pixels = new uint32_t[16 * 16];
        std::fill_n(outTex.pixels, 16 * 16, 0xFFFF00FF);
    }
}

void PutPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        framebuffer[y * WIDTH + x] = color;
    }
}

void PutPixelAlpha(int x, int y, uint32_t color, float alpha) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        int idx = y * WIDTH + x;
        uint32_t bg = framebuffer[idx];
        uint8_t rB = (bg >> 16) & 0xFF; uint8_t gB = (bg >> 8) & 0xFF; uint8_t bB = bg & 0xFF;
        uint8_t rF = (color >> 16) & 0xFF; uint8_t gF = (color >> 8) & 0xFF; uint8_t bF = color & 0xFF;
        uint8_t r = (uint8_t)(rF * alpha + rB * (1.0f - alpha));
        uint8_t g = (uint8_t)(gF * alpha + gB * (1.0f - alpha));
        uint8_t b = (uint8_t)(bF * alpha + bB * (1.0f - alpha));
        framebuffer[idx] = (r << 16) | (g << 8) | b;
    }
}

void DrawChar(int x, int y, char c, uint8_t r, uint8_t g, uint8_t b) {
    if (c >= 'a' && c <= 'z') {
        c -= 32;
    }
    unsigned char index = (unsigned char)c;
    if (index > 127) return;
    const unsigned char* glyph = font8x8[index];
    for (int ky = 0; ky < 8; ky++) {
        for (int kx = 0; kx < 8; kx++) {
            if (glyph[ky] & (1 << (7 - kx))) {
                PutPixel(x + kx + 1, y + ky + 1, 0x000000);
                PutPixel(x + kx, y + ky, (r << 16) | (g << 8) | b);
            }
        }
    }
}



void DrawString(int x, int y, const char* str, uint8_t r, uint8_t g, uint8_t b) {
    while (*str) {
        DrawChar(x, y, *str, r, g, b);
        x += 8; 
        str++;
    }
}





void RenderUI() {
    char fpsBuffer[32];
    snprintf(fpsBuffer,sizeof(fpsBuffer), "FPS: %f",(int)1/dt);
    DrawString(WIDTH-100, 10, fpsBuffer, 255, 255, 255);
}
void RenderScene() {
    std::fill_n(framebuffer, WIDTH * HEIGHT, 0x0A0F1D);
}



float ComputeDeltaTime() {
    static auto last = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = now - last; last = now;
    return delta.count();
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_KEYDOWN: if (wp == VK_ESCAPE) DestroyWindow(hwnd); break;
        case WM_SIZE: {
            WIDTH = LOWORD(lp); HEIGHT = HIWORD(lp);
            if (WIDTH < 100 || HEIGHT < 100) { WIDTH = 320; HEIGHT = 200; }
            if (framebuffer != nullptr) delete[] framebuffer;
            framebuffer = new uint32_t[WIDTH * HEIGHT];
            bmi.bmiHeader.biWidth = WIDTH; bmi.bmiHeader.biHeight = -HEIGHT;
            break;
        }
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASS wc = { 0, WndProc, 0, 0, hInst, LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_WINDOW+1), NULL, "RSC_Class" };
    RegisterClass(&wc);
    HWND hwnd = CreateWindow("RSC_Class", "graphics c++", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT, NULL, NULL, hInst, NULL);
    globalHwnd = hwnd; ShowWindow(hwnd, nCmdShow);
    framebuffer = new uint32_t[WIDTH * HEIGHT];
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); bmi.bmiHeader.biWidth = WIDTH; bmi.bmiHeader.biHeight = -HEIGHT;
    bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(hwnd); 

    MSG msg = {}; bool running = true;
    while (running) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; }
            TranslateMessage(&msg); DispatchMessage(&msg);
        } else {
            dt = ComputeDeltaTime();
            RenderScene();
            RenderUI();
            StretchDIBits(hdc, 0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, framebuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
        }
    }
    ReleaseDC(hwnd, hdc); delete[] framebuffer; return 0;
}
