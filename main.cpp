#include <windows.h>
#include <math.h>

typedef unsigned int uint32;

static int bytesPerPixel = 4;

struct Buffer {
    BITMAPINFO bitmapInfo;
    void* memory;
    int width;
    int height;
    int memorySize;
};

static Buffer buffer;

uint32 rgb(int r, int g, int b) {
    return r << 16 | g << 8 | b;
}

void point(int x, int y, uint32 color);
void rect(int x, int y, int width, int height, uint32 color);
void fillRect(int x, int y, int width, int height, uint32 color);
void line(int x, int y, int x2, int y2, uint32 color);

void lol(int x, int y) {
    fillRect(x+20, y + 20, 20, 100, 0xaf2f1f);
    fillRect(x + 40, y + 100, 40, 20, 0xaf2f1f);

    fillRect(x + 100, y + 20, 60, 100, 0x14ab12);
    fillRect(x + 120, y + 40, 20, 60, 0);

    fillRect(x + 180, y + 20, 20, 100, 0x0912af);
    fillRect(x + 200, y+100, 40, 20, 0x0912af);
}

void render(float offset) {
    fillRect(0, 0, 800, 800, 0);
    lol(150+50*cos(offset), 100+50*sin(offset));

    rect(320, 20, 620, 2220, 0xffffff);
    line(100, 200, 300, 20, 0xffffff);
}

void allocateBuffer(Buffer *buffer, int width, int height) {

    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }
    buffer->width = width;
    buffer->height = height;

    buffer->bitmapInfo.bmiHeader.biSize = sizeof(buffer->bitmapInfo.bmiHeader);
    buffer->bitmapInfo.bmiHeader.biWidth = buffer->width;
    buffer->bitmapInfo.bmiHeader.biHeight = -buffer->height;
    buffer->bitmapInfo.bmiHeader.biPlanes = 1;
    buffer->bitmapInfo.bmiHeader.biBitCount = 32;
    buffer->bitmapInfo.bmiHeader.biCompression = BI_RGB;
    buffer->bitmapInfo.bmiHeader.biSizeImage = 0;
    buffer->bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
    buffer->bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
    buffer->bitmapInfo.bmiHeader.biClrUsed = 0;
    buffer->bitmapInfo.bmiHeader.biClrImportant = 0;

    buffer->memorySize = (buffer->width * buffer->height) * bytesPerPixel;

    buffer->memory = VirtualAlloc(0, buffer->memorySize, MEM_COMMIT, PAGE_READWRITE);
}

void point(int x, int y, uint32 color) {
    if (x < 0 || y < 0 || x >= buffer.width || y >= buffer.height) {
        return;
    }
    uint32* pixel = (uint32*)buffer.memory;
    int index = x + y * buffer.width;
    pixel[index] = color;
}

void rect(int x, int y, int width, int height, uint32 color) {
    for (int i = 0; i < width; i++) {
        point(x + i, y, color);
        point(x + i, y + height, color);
    }
    for (int i = 0; i <= height; i++) {
        point(x, y + i, color);
        point(x + width, y + i, color);
    }
}

void fillRect(int x, int y, int width, int height, uint32 color) {
    for (int a = 0; a < width; a++) {
        for (int b = 0; b <= height; b++) {
            point(a + x, b + y, color);
        }
    }
}

void line(int x, int y, int x2, int y2, uint32 color) {
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


 void copyBufferToScreen(HDC deviceContext, Buffer *buffer, int width, int height) {


    StretchDIBits(deviceContext,
        0, 0, width, height,
        0, 0, buffer->width, buffer->height,
        buffer->memory,
        &buffer->bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {

    const wchar_t CLASS_NAME[] = L"lol";

    WNDCLASS wc = {};

    wc.style = CS_HREDRAW | CS_VREDRAW;
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

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    allocateBuffer(&buffer, width, height);

    bool running = true;

    float offset = 0;
    float offsetSpeed = 0;
    float ticks = 0;
    while (running) {
        MSG msg;

        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }


        render(offset);

        HDC deviceContent = GetDC(hwnd);

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        int windowWidth = clientRect.right - clientRect.left;
        int windowHeight = clientRect.bottom - clientRect.top;
        copyBufferToScreen(deviceContent, &buffer, windowWidth, windowHeight);
        ticks++;
        offsetSpeed = sin(ticks / 100);
        offset+=offsetSpeed;
    }
    
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {
        
        case WM_DESTROY:

            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            return 0;
        
      
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}