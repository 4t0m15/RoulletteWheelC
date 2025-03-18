#include <windows.h>
#include <math.h>

// Window procedure function prototype
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

// Window class name
const char g_szClassName[] = "myWindowClass";

// Global variables for animation
double g_angle = 0.0;
COLORREF g_lineColor = RGB(255, 0, 0); // Start with red
int g_colorPhase = 0;
UINT_PTR g_timerID = 1;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG msg;

    // Register the window class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW; // Redraw on size change
    wc.lpfnWndProc   = WindowProcedure;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create the window with larger dimensions
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Rotating Line Animation",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, // Larger window
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

// Process messages for the window
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    int centerX, centerY, endX, endY;
    int lineLength;

    switch(message)
    {
        case WM_CREATE:
            // Set up a timer to update the animation (30 milliseconds ≈ 33 FPS)
            SetTimer(hwnd, g_timerID, 30, NULL);
            break;

        case WM_TIMER:
            if(wParam == g_timerID)
            {
                // Update the angle for rotation
                g_angle += 0.05; // Speed of rotation
                if(g_angle > 2 * 3.14159265359) // 2π
                    g_angle = 0;

                // Update the color (cycle through hues)
                g_colorPhase = (g_colorPhase + 1) % 768;

                if(g_colorPhase < 256)
                    g_lineColor = RGB(255, g_colorPhase, 0); // Red to Yellow
                else if(g_colorPhase < 512)
                    g_lineColor = RGB(511 - g_colorPhase, 255, g_colorPhase - 256); // Yellow to Green
                else
                    g_lineColor = RGB(0, 767 - g_colorPhase, g_colorPhase - 512); // Green to Blue

                // Force the window to redraw
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);

            // Get the dimensions of the client area
            GetClientRect(hwnd, &rect);

            // Calculate the center of the window
            centerX = rect.right / 2;
            centerY = rect.bottom / 2;

            // Calculate line length based on window size (75% of half the minimum dimension)
            lineLength = (min(rect.right, rect.bottom) / 2) * 0.75;

            // Calculate endpoint using trigonometry
            endX = centerX + (int)(lineLength * cos(g_angle));
            endY = centerY + (int)(lineLength * sin(g_angle));

            // Create and select a colored pen
            HPEN hPen = CreatePen(PS_SOLID, 3, g_lineColor);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

            // Draw the line from center to calculated endpoint
            MoveToEx(hdc, centerX, centerY, NULL);
            LineTo(hdc, endX, endY);

            // Clean up GDI objects
            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);

            EndPaint(hwnd, &ps);
            break;

        case WM_CLOSE:
            KillTimer(hwnd, g_timerID); // Clean up the timer
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}