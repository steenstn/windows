#include <windows.h>
#include <stdint.h>
#include <math.h>

static BITMAPINFO bitmapInfo;
static void* bitmapMemory;
static int bitmapWidth;
static int bitmapHeight;
static int bytesPerPixel = 4;
static int bitmapMemorySize;

uint32_t rgb(int r, int g, int b) {
    return r << 16 | g << 8 | b;
}

void point(int x, int y, uint32_t color);
void rect(int x, int y, int width, int height, uint32_t color);
void fillRect(int x, int y, int width, int height, uint32_t color);
void line(int x, int y, int x2, int y2, uint32_t color);

void resizeDIBSection(int width, int height) {

    if (bitmapMemory) {
        VirtualFree(bitmapMemory, 0, MEM_RELEASE);
    }
    bitmapWidth = width;
    bitmapHeight = height;

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = bitmapWidth;
    bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    bitmapInfo.bmiHeader.biSizeImage = 0;
    bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
    bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
    bitmapInfo.bmiHeader.biClrUsed = 0;
    bitmapInfo.bmiHeader.biClrImportant = 0;

    bitmapMemorySize = (bitmapWidth * bitmapHeight) * bytesPerPixel;

    bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    fillRect(20, 20, 20, 100, 0xaf2f1f);
    fillRect(40, 100, 40, 20, 0xaf2f1f);

    fillRect(100, 20, 60, 100, 0x14ab12);
    fillRect(120, 40, 20, 60, 0);
    
    fillRect(180, 20, 20, 100, 0x0912af);
    fillRect(200, 100, 40, 20, 0x0912af);

    rect(320, 20, 620, 2220, 0xffffff);
    line(100, 200, 300, 20, 0xffffff);
}

void point(int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= bitmapWidth || y >= bitmapHeight) {
        return;
    }
    uint32_t* pixel = (uint32_t*)bitmapMemory;
    int index = x + y * bitmapWidth;
    pixel[index] = color;
}

void rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        point(x + i, y, color);
        point(x + i, y + height, color);
    }
    for (int i = 0; i <= height; i++) {
        point(x, y + i, color);
        point(x + width, y + i, color);
    }
}

void fillRect(int x, int y, int width, int height, uint32_t color) {
    for (int a = 0; a < width; a++) {
        for (int b = 0; b <= height; b++) {
            point(a + x, b + y, color);
        }
    }
}

void line(int x, int y, int x2, int y2, uint32_t color) {
    double angle = atan2((double)y2 - (double)y, (double)x2 - (double)x);
    double cosAngle = cos(angle);
    double sinAngle = sin(angle);
    double currentX = x;
    double currentY = y;
    int safeGuard = 0;
    while (!(fabs(currentX - x2) < 2 && fabs(currentY - y2) < 2)) {
        if (safeGuard++ > 1000) {
            break;
        }
        point(round(currentX), round(currentY), color);
        currentX += cosAngle;
        currentY += sinAngle;
    }
}


 void win32updateWindow(HDC deviceContext, RECT *windowRect, int x, int y, int width, int height) {

    int windowWidth = windowRect->right - windowRect->left;
    int windowHeight = windowRect->bottom - windowRect->top;

    StretchDIBits(deviceContext,
        0, 0, bitmapWidth, bitmapHeight,
        0, 0, windowWidth, windowHeight,
        bitmapMemory,
        &bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {

    const wchar_t CLASS_NAME[] = L"lol";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"hells yeah", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 400,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {
        
        case WM_SIZE: {
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            GetClientRect(hwnd, &clientRect);
            resizeDIBSection(width, height);
        }
            return 0;
        case WM_CREATE:
             
            return 0;
        case WM_DESTROY:

            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            return 0;
        
        case WM_PAINT:
        {
            PAINTSTRUCT     ps;
            HDC             hdc;
            HDC             hdcMem;
            HGDIOBJ         oldBitmap;

            HDC deviceContent = BeginPaint(hwnd, &ps);
            int x = ps.rcPaint.left;
            int y = ps.rcPaint.top;
            int width = ps.rcPaint.right - ps.rcPaint.left;
            int height = ps.rcPaint.bottom - ps.rcPaint.top;

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            win32updateWindow(deviceContent,&clientRect,  x, y, width, height);
            EndPaint(hwnd, &ps);
        }
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}