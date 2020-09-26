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

struct KeyMap {
    bool keyUp;
    bool keyDown;
    bool keyLeft;
    bool keyRight;
};

KeyMap keymap;

static Buffer buffer;

uint32 rgb(int r, int g, int b) {
    return r << 16 | g << 8 | b;
}

void pixel(Buffer *buffer, int x, int y, uint32 color);
void rect(Buffer* buffer, int x, int y, int width, int height, uint32 color);
void fillRect(Buffer* buffer, int x, int y, int width, int height, uint32 color);
void line(Buffer* buffer, int x, int y, int x2, int y2, uint32 color);

void lol(Buffer* buffer, int x, int y) {
    fillRect(buffer, x+20, y + 20, 20, 100, 0xaf2f1f);
    fillRect(buffer, x + 40, y + 100, 40, 20, 0xaf2f1f);
              
    fillRect(buffer, x + 100, y + 20, 60, 100, 0x14ab12);
    fillRect(buffer, x + 120, y + 40, 20, 60, 0);
             
    fillRect(buffer, x + 180, y + 20, 20, 100, 0x0912af);
    fillRect(buffer, x + 200, y+100, 40, 20, 0x0912af);
}

void render(float x, float y) {
    fillRect(&buffer, 0, 0, 800, 800, 0);
    lol(&buffer, x, y);

    rect(&buffer, 320, 20, 620, 2220, 0xffffff);
    line(&buffer, 100, 200, 300, 20, 0xffffff);
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

void pixel(Buffer* buffer, int x, int y, uint32 color) {
    if (x < 0 || y < 0 || x >= buffer->width || y >= buffer->height) {
        return;
    }
    uint32* pixel = (uint32*)buffer->memory;
    int index = x + y * buffer->width;
    pixel[index] = color;
}

void rect(Buffer* buffer, int x, int y, int width, int height, uint32 color) {
    for (int i = 0; i < width; i++) {
        pixel(buffer, x + i, y, color);
        pixel(buffer, x + i, y + height, color);
    }
    for (int i = 0; i <= height; i++) {
        pixel(buffer, x, y + i, color);
        pixel(buffer, x + width, y + i, color);
    }
}

void fillRect(Buffer* buffer, int x, int y, int width, int height, uint32 color) {
    for (int a = 0; a < width; a++) {
        for (int b = 0; b <= height; b++) {
            pixel(buffer, a + x, b + y, color);
        }
    }
}

void line(Buffer* buffer, int x, int y, int x2, int y2, uint32 color) {
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
        pixel(buffer, round(currentX), round(currentY), color);
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

float x = 100;
float y = 100;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {

    const wchar_t CLASS_NAME[] = L"lol";

    WNDCLASS windowClass = {};

    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = CLASS_NAME;

    RegisterClass(&windowClass);

    HWND windowHandle = CreateWindowEx(0, CLASS_NAME, L"hells yeah", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 400,
        NULL, NULL, hInstance, NULL);

    if (windowHandle == NULL) {
        return 0;
    }


    RECT clientRect;
    GetClientRect(windowHandle, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    allocateBuffer(&buffer, width, height);

    bool running = true;

    float offset = 0;
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
        if (keymap.keyUp) {
            y--;
        }
        if (keymap.keyDown) {
            y++;
        }
        if (keymap.keyLeft) {
            x--;
        }
        if (keymap.keyRight) {
            x++;
        }

        render(x,y);

        HDC deviceContent = GetDC(windowHandle);

        RECT clientRect;
        GetClientRect(windowHandle, &clientRect);

        int windowWidth = clientRect.right - clientRect.left;
        int windowHeight = clientRect.bottom - clientRect.top;
        copyBufferToScreen(deviceContent, &buffer, windowWidth, windowHeight);
        ticks++;
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
            if (wParam == VK_UP) {
                keymap.keyUp = true;
            }
            if (wParam == VK_DOWN) {
                keymap.keyDown = true;
            }
            if (wParam == VK_LEFT) {
                keymap.keyLeft = true;
            }
            if (wParam == VK_RIGHT) {
                keymap.keyRight = true;
            }
            return 0;
        case WM_KEYUP:
            if (wParam == VK_UP) {
                keymap.keyUp = false;
            }
            if (wParam == VK_DOWN) {
                keymap.keyDown = false;
            }
            if (wParam == VK_LEFT) {
                keymap.keyLeft = false;
            }
            if (wParam == VK_RIGHT) {
                keymap.keyRight = false;
            }
            return 0;
      
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}