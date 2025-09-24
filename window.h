#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include "math2d.h"
#include "math3d.h"
#include "render.h"

// Variables globales
extern HWND mainW;
extern int mouseX, mouseY;

typedef struct {
    LPWSTR lpWindowName;
    DWORD dwExStyle;
    DWORD dwStyle;
    int x, y, width, height;
    HWND parent;
} WindowParams;

// Declaraciones de funciones
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM RegisterMyClass(WindowParams* params);

#endif
