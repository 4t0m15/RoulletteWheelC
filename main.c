#include <windows.h>  // Core Windows API header
#include <math.h>     // For trigonometric functions (sin, cos)

// Window procedure function prototype - handles all window messages
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

// Window class name - used to identify our window class
const char g_szClassName[] = "myWindowClass";

// -------------------- GLOBAL VARIABLES --------------------
// Animation angle in radians (0 to 2π)
double g_angle = 0.0;

// Current color of the line - starts as red (R=255, G=0, B=0)
COLORREF g_lineColor = RGB(255, 0, 0);

// Color phase tracker (0-767) to smoothly cycle through the color spectrum:
// 0-255: Red to Yellow (increasing green)
// 256-511: Yellow to Green (decreasing red)
// 512-767: Green to Blue (increasing blue, decreasing green)
int g_colorPhase = 0;

// Timer identifier - used to update animation at regular intervals
UINT_PTR g_timerID = 1;

// -------------------- MAIN ENTRY POINT --------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    // Define variables for the window class, window handle, and message structure
    WNDCLASSEX wc;      // Extended window class structure
    // Handle to the window
    MSG msg;            // Message structure for processing messages

    // -------------------- WINDOW CLASS REGISTRATION --------------------
    // Fill in the window class structure with parameters describing the window
    wc.cbSize        = sizeof(WNDCLASSEX);    // Size of structure in bytes
    wc.style         = CS_HREDRAW | CS_VREDRAW; // Redraw entire window if size changes
    wc.lpfnWndProc   = WindowProcedure;       // Pointer to window procedure function
    wc.cbClsExtra    = 0;                     // No extra bytes after the window class
    wc.cbWndExtra    = 0;                     // No extra bytes after the window instance
    wc.hInstance     = hInstance;             // Instance handle to the application
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION); // Default application icon
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);     // Default arrow cursor
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);        // Default window background color
    wc.lpszMenuName  = NULL;                  // No menu
    wc.lpszClassName = g_szClassName;         // Name of the window class
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION); // Default small icon

    // Register the window class with Windows
    // If registration fails, show an error message and exit
    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // -------------------- WINDOW CREATION --------------------
    // Create the window with specified attributes
    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE, // Extended window style - client edge
        g_szClassName, // Window class name
        "Rotating Line Animation", // Window title
        WS_OVERLAPPEDWINDOW, // Window style - standard overlapped window
        CW_USEDEFAULT, CW_USEDEFAULT, // x, y positions - let Windows decide
        500, 500, // Width, height in pixels - 500x500 for better visibility
        NULL, // Parent window handle - no parent
        NULL, // Menu handle - no menu
        hInstance, // Application instance handle
        NULL);                  // Pointer to window creation data - not used

    // If window creation fails, show an error message and exit
    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // -------------------- DISPLAY THE WINDOW --------------------
    ShowWindow(hwnd, nCmdShow);   // Make the window visible
    UpdateWindow(hwnd);           // Force an initial paint of the window

    // -------------------- MESSAGE LOOP --------------------
    // Main message loop - continuously retrieve and dispatch messages
    while(GetMessage(&msg, NULL, 0, 0) > 0)  // Get message from queue (0 when WM_QUIT)
    {
        TranslateMessage(&msg);  // Translate virtual-key messages to character messages
        DispatchMessage(&msg);   // Dispatch message to window procedure
    }

    // Return the exit code stored in the quit message's wParam
    return msg.wParam;
}

// -------------------- WINDOW PROCEDURE --------------------
// Processes messages sent to the window
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Variables for painting operations
    HDC hdc;                // Handle to Device Context (for drawing)
    PAINTSTRUCT ps;         // Structure with painting information
    RECT rect;              // Rectangle structure for client area dimensions
    int centerX, centerY;   // Center coordinates of the window
    int endX, endY;         // End coordinates of the rotating line
    int lineLength;         // Length of the rotating line

    // Handle different window messages
    switch(message)
    {
        // -------------------- WINDOW CREATION --------------------
        case WM_CREATE:
            // Window is being created
            // Set up a timer that will trigger every 30 milliseconds (approx. 33 FPS)
            // Timer sends WM_TIMER messages to this window procedure
            SetTimer(hwnd, g_timerID, 30, NULL);
            break;

        // -------------------- TIMER EVENTS --------------------
        case WM_TIMER:
            // Check if this is our animation timer
            if(wParam == g_timerID)
            {
                // --------- UPDATE ROTATION ANGLE ---------
                // Increment angle by 0.05 radians for smooth rotation
                g_angle += 0.05;

                // Reset angle when it completes a full circle (2π radians)
                if(g_angle > 2 * 3.14159265359)
                    g_angle = 0;

                // --------- UPDATE COLOR ---------
                // Increment color phase (0-767) to cycle through color spectrum
                g_colorPhase = (g_colorPhase + 1) % 768;

                // Calculate the RGB values based on color phase:
                if(g_colorPhase < 256)
                {
                    // Phase 0-255: Red to Yellow (R=255, G increases from 0 to 255, B=0)
                    g_lineColor = RGB(255, g_colorPhase, 0);
                }
                else if(g_colorPhase < 512)
                {
                    // Phase 256-511: Yellow to Green (R decreases from 255 to 0, G=255, B increases from 0 to 255)
                    g_lineColor = RGB(511 - g_colorPhase, 255, g_colorPhase - 256);
                }
                else
                {
                    // Phase 512-767: Green to Blue (R=0, G decreases from 255 to 0, B increases from 0 to 255)
                    g_lineColor = RGB(0, 767 - g_colorPhase, g_colorPhase - 512);
                }

                // --------- FORCE REDRAW ---------
                // Tell Windows that the entire window needs to be redrawn
                // TRUE parameter means the background should be erased first
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        // -------------------- PAINTING --------------------
        case WM_PAINT:
            // Begin painting operation - get device context for drawing
            hdc = BeginPaint(hwnd, &ps);

            // --------- GET WINDOW DIMENSIONS ---------
            // Retrieve the dimensions of the client area (drawable area)
            GetClientRect(hwnd, &rect);

            // --------- CALCULATE CENTER POINT ---------
            // Find the center of the window for the rotation origin
            centerX = rect.right / 2;   // Half the width
            centerY = rect.bottom / 2;  // Half the height

            // --------- CALCULATE LINE LENGTH ---------
            // Set line length to 75% of half the smallest window dimension
            // This ensures the line doesn't extend beyond the window even when rotated
            lineLength = (min(rect.right, rect.bottom) / 2) * 0.75;

            // --------- CALCULATE ENDPOINT ---------
            // Use trigonometry to find the endpoint of the rotated line:
            // x = center_x + length * cos(angle)
            // y = center_y + length * sin(angle)
            endX = centerX + (int)(lineLength * cos(g_angle));
            endY = centerY + (int)(lineLength * sin(g_angle));

            // --------- CREATE AND CONFIGURE PEN ---------
            // Create a pen with the current color and 3-pixel width
            HPEN hPen = CreatePen(PS_SOLID, 3, g_lineColor);

            // Save the old pen and select our new pen into the device context
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

            // --------- DRAW THE LINE ---------
            // Move to the center point (no line drawn yet)
            MoveToEx(hdc, centerX, centerY, NULL);

            // Draw a line from the center to the calculated endpoint
            LineTo(hdc, endX, endY);

            // --------- CLEAN UP GDI OBJECTS ---------
            // Restore the old pen
            SelectObject(hdc, hOldPen);

            // Delete our pen to prevent resource leaks
            DeleteObject(hPen);

            // End painting operation
            EndPaint(hwnd, &ps);
            break;

        // -------------------- WINDOW CLOSING --------------------
        case WM_CLOSE:
            // User is trying to close the window

            // Stop the animation timer to prevent messages after window destruction
            KillTimer(hwnd, g_timerID);

            // Destroy the window (generates WM_DESTROY message)
            DestroyWindow(hwnd);
            break;

        // -------------------- WINDOW DESTRUCTION --------------------
        case WM_DESTROY:
            // Window is being destroyed

            // Post WM_QUIT message to terminate the application
            PostQuitMessage(0);
            break;

        // -------------------- DEFAULT HANDLING --------------------
        default:
            // Let Windows handle any messages we don't explicitly process
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    // Return 0 to indicate we've processed the message
    return 0;
}