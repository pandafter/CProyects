#include <windows.h>
#include "window.h"

int main() {

    mouseX = 400;
    mouseY = 300;

    WindowParams params = {0};
    params.lpWindowName = L"Mi Ventana";
    params.dwExStyle = WS_EX_WINDOWEDGE;
    params.dwStyle = WS_OVERLAPPEDWINDOW;
    params.x = CW_USEDEFAULT;
    params.y = CW_USEDEFAULT;
    params.width = CW_USEDEFAULT;
    params.height = CW_USEDEFAULT;
    params.parent = NULL;

    RegisterMyClass(&params);

    ShowWindow(mainW, SW_SHOWNORMAL | SW_RESTORE);
    UpdateWindow(mainW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return(0);
}
