// @IMPORTANT: error handling ignored

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

#include <windows.h>
#include <math.h>
#include <stdint.h>

LRESULT CALLBACK windowMessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_PAINT: ValidateRect(window, nullptr); return 0;
    case WM_KEYDOWN: if (wparam == VK_ESCAPE) { PostQuitMessage(0); } return 0;
    default: return DefWindowProcA(window, message, wparam, lparam);
  }
}

uint32_t toUintColor(float value) {
  return static_cast<uint32_t>(value * 255.0f + 0.5f);
}

float premultiplyAlpha(float component, float alpha) {
  return component * alpha;
}

float sdcircle(float px, float py, float radius) {
  return sqrtf(px * px + py * py) - radius;
}

float lerp(float from, float to, float u) {
  return (1.0f - u) * from + u * to;
}

float clamp(float value, float min, float max) {
  return value < min ? min : value > max ? max : value;
}

float smoothstep(float edge0, float edge1, float value) {
  float t = clamp((value - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCommand) {
  //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  WNDCLASSA wc = {.lpfnWndProc = windowMessageHandler, .hInstance = instance, .lpszClassName = "class"};
  RegisterClassA(&wc);

  SIZE windowSize = {500, 500};
  HWND window = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST, "class", "title", WS_POPUP, 2160 - 500, 3840 - 400, windowSize.cx, windowSize.cy, nullptr, nullptr, instance, nullptr);
  ShowWindow(window, SW_SHOW);

  HDC windowDC = GetDC(window);
  HDC desktopDC = GetDC(nullptr);
  HDC memoryDC = CreateCompatibleDC(windowDC);

  BITMAPINFO info = { };
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  info.bmiHeader.biWidth = windowSize.cx;
  info.bmiHeader.biHeight = -windowSize.cy;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;

  uint32_t* pixels = nullptr;
  HBITMAP bitmap = CreateDIBSection(memoryDC, &info, DIB_RGB_COLORS, reinterpret_cast<void**>(&pixels), nullptr, 0);
  HGDIOBJ original = SelectObject(memoryDC, bitmap);

  MSG msg = {};
  while (msg.message != WM_QUIT) {
    if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    } else {
      double time = static_cast<double>(GetTickCount()) / 1000.0;
      float dx = cos(time * 0.1 * 11.0) * 0.3;
      float dy = sin(time * 0.1 * 3.0) * 0.3;

      for (int y = 0; y < windowSize.cy; ++y) {
        for (int x = 0; x < windowSize.cx; ++x) {
          float u = static_cast<float>(x) / static_cast<float>(windowSize.cx - 1) * 2.0f - 1.0f;
          float v = static_cast<float>(y) / static_cast<float>(windowSize.cy - 1) * 2.0f - 1.0f;
          float c = smoothstep(-0.1f, 0.f, -sdcircle(u + dx, v + dy, 0.2f));

          uint32_t r = toUintColor(premultiplyAlpha(lerp(0.94f, 0.98f, c), c));
          uint32_t g = toUintColor(premultiplyAlpha(lerp(0.54f, 0.93f, c), c));
          uint32_t b = toUintColor(premultiplyAlpha(lerp(0.36f, 0.41f, c), c));
          uint32_t a = toUintColor(c);

          pixels[y * windowSize.cx + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
      }

      POINT source = { };
      BLENDFUNCTION blend = {.BlendOp = AC_SRC_OVER, .SourceConstantAlpha = 255, .AlphaFormat = AC_SRC_ALPHA};
      UpdateLayeredWindow(window, desktopDC, nullptr, &windowSize, memoryDC, &source, 0, &blend, ULW_ALPHA);
    }
  }

  SelectObject(memoryDC, original);
  DeleteObject(bitmap);
  DeleteDC(memoryDC);
  ReleaseDC(nullptr, desktopDC);
  ReleaseDC(window, windowDC);
  DestroyWindow(window);

  return 0;
}
