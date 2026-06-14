#include <windows.h>
#include <cstdint>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include "font.h"
#include <fstream>
#include <iostream> 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float dt = 0;
int WIDTH = 1080; 
int HEIGHT = 720;


uint32_t* framebuffer = nullptr;
HWND globalHwnd = NULL;
BITMAPINFO bmi = {};


void RenderUI() {
    char fpsBuffer[32];
    snprintf(fpsBuffer,sizeof(fpsBuffer), "FPS: %f",(int)1/dt);
    DrawString(WIDTH-100, 10, fpsBuffer, 255, 255, 255);
}
void RenderScene() {
    std::fill_n(framebuffer, WIDTH * HEIGHT, 0x0A0F1D);

}
void Update(float dt){


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

    InitMap();
    InitTrains();

    while (running) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; }
            TranslateMessage(&msg); DispatchMessage(&msg);
        } else {
            dt = ComputeDeltaTime();
            Update(dt);
            RenderScene();
            RenderUI();
            StretchDIBits(hdc, 0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, framebuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
        }
    }
    ReleaseDC(hwnd, hdc); delete[] framebuffer; return 0;
}
