#include <windows.h>
#include "math2d.h"
#include "math3d.h"
#include "render.h"

// Variables globales
HWND mainW;
int mouseX, mouseY;

typedef struct {
    LPWSTR lpWindowName;
    DWORD dwExStyle;
    DWORD dwStyle;
    int x, y, width, height;
    HWND parent;
} WindowParams;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_MOUSEMOVE:
            {
                mouseX = LOWORD(lParam);
                mouseY = HIWORD(lParam);

                // se deja eln TRUE el borrar el fondo para oblifgar al PAINT a hacer un repintado y actualice
                static DWORD lastUpdate = 0;
                DWORD currentTime = GetTickCount();
                if (currentTime - lastUpdate > 16) {
                    InvalidateRect(hwnd, NULL, TRUE);
                    lastUpdate = currentTime;
                }
                     
            }
            return 0;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                //Obtener dimensiones de la ventana para poder sacar nuestro width y height cuando hagamos render_2d_test tambien
                RECT rect;
                GetClientRect(hwnd, &rect);

                int ScreenWidth = rect.right - rect.left;
                int ScreenHeight = rect.bottom - rect.top;

                //Limpiar pantalla con fondo negro
                FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));

                // AQUI LLAMAMOS A LA FUNCION RENDER_2D_TEST
                render_3d_test(hdc, ScreenWidth, ScreenHeight, mouseX, mouseY);
                render_2d_test(hdc, ScreenWidth, ScreenHeight, mouseX, mouseY);

                EndPaint(hwnd, &ps);
            }
            return 0;
        case WM_CREATE:
            SetTimer(hwnd, 1, 16, NULL); // Se setea a 60fps
            return 0;

        case WM_TIMER:
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

ATOM RegisterMyClass(WindowParams* params) {

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = "MiVentana";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);

    RegisterClassEx(&wc);

    mainW = CreateWindowExW(
        params->dwExStyle,
        L"MiVentana",
        params->lpWindowName,
        params->dwStyle,
        params->x,params->y,
        params->width,params->height,
        params->parent,
        NULL,
        NULL,
        NULL 
    );

    return (mainW ? TRUE : FALSE);
}
