// @IMPORTANT: error handling ignored

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#include <windows.h>
#include <dwrite.h>
#include <d2d1.h>
#include <string>

template <typename T>
void SafeRelease(T** p) {
  if (*p) {
    (*p)->Release();
    *p = nullptr;
  }
}

LRESULT CALLBACK windowMessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_PAINT: ValidateRect(window, nullptr); return 0;
    case WM_KEYDOWN: if (wparam == VK_ESCAPE) { PostQuitMessage(0); } return 0;
    default: return DefWindowProcA(window, message, wparam, lparam);
  }
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCommand) {
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  WNDCLASSA wc = {.lpfnWndProc = windowMessageHandler, .hInstance = instance, .lpszClassName = "class"};
  RegisterClassA(&wc);

  SIZE windowSize = {500, 500};
  HWND targetWindow = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST, "class", "title", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, windowSize.cx, windowSize.cy, nullptr, nullptr, instance, nullptr);
  ShowWindow(targetWindow, SW_SHOW);

  HDC targetWindowDC = GetDC(targetWindow);
  HDC memoryDC = CreateCompatibleDC(targetWindowDC);
  HBITMAP bitmap = CreateCompatibleBitmap(targetWindowDC, windowSize.cx, windowSize.cy);
  HGDIOBJ original = SelectObject(memoryDC, bitmap);

  ID2D1Factory* d2d = nullptr;
  IDWriteFactory* dwrite = nullptr;
  IDWriteTextFormat* textFormat = nullptr;
  ID2D1DCRenderTarget* rt = nullptr;
  ID2D1SolidColorBrush* brush = nullptr;

  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d);
  DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite));
  dwrite->CreateTextFormat(L"Segoe UI Variable Display", nullptr, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 64.0f, L"en-FI", &textFormat);
  textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  D2D1_RENDER_TARGET_PROPERTIES props = { };
  props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
  props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
  props.dpiX = 96.0f;
  props.dpiY = 96.0f;
  props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
  props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
  d2d->CreateDCRenderTarget(&props, &rt);
  rt->CreateSolidColorBrush(D2D1::ColorF(0.0f, 1.0f, 1.0f, 1.0f), &brush);

  MSG msg = {};
  while (msg.message != WM_QUIT) {
    if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    } else {
      RECT bindRect = {0, 0, windowSize.cx, windowSize.cy};
      rt->BindDC(memoryDC, &bindRect);
      rt->BeginDraw();
      rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE); // D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE blending is fucked
      rt->SetTransform(D2D1::IdentityMatrix());
      rt->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

      static std::wstring text = L"DirectWrite text";
      brush->SetColor(D2D1::ColorF(0.0f, 1.0f, 1.0f, 0.3f));
      rt->DrawText(text.c_str(), static_cast<UINT>(text.length()), textFormat, D2D1::RectF(0.0f, 0.0f, static_cast<float>(windowSize.cx), static_cast<float>(windowSize.cy)), brush);

      brush->SetColor(D2D1::ColorF(1.0f, 0.0f, 0.0f, 0.6f));
      rt->FillRectangle(D2D1::RectF(0.0f, 0.0f, 100.0f, 100.0f), brush);
      rt->EndDraw();

      POINT layerLocation = { };
      BLENDFUNCTION blend = {.BlendOp = AC_SRC_OVER, .SourceConstantAlpha = 255, .AlphaFormat = AC_SRC_ALPHA};
      UpdateLayeredWindow(targetWindow, nullptr, nullptr, &windowSize, memoryDC, &layerLocation, 0, &blend, ULW_ALPHA);
    }
  }

  SafeRelease(&brush);
  SafeRelease(&rt);
  SafeRelease(&textFormat);
  SafeRelease(&dwrite);
  SafeRelease(&d2d);

  SelectObject(memoryDC, original);
  DeleteObject(bitmap);
  DeleteDC(memoryDC);
  ReleaseDC(targetWindow, targetWindowDC);
  DestroyWindow(targetWindow);

  return 0;
}
