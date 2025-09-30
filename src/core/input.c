#include "core/input.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>

// ---------------------------
// Estado global
// ---------------------------
static InputState g_inputState = {0};
static HWND  g_hwnd = NULL;
static BOOL  g_mouseInWindow = FALSE;
static BOOL  g_trackingMouse = FALSE;

// Para manejar movimiento absoluto (tabletas/remote, etc.)
static BOOL  g_haveAbsRef = FALSE;
static LONG  g_absRefX = 0;
static LONG  g_absRefY = 0;

// Utilidad segura para ShowCursor (evita bucles eternos)
static void SetCursorVisible(BOOL show) {
    CURSORINFO ci = { .cbSize = sizeof(CURSORINFO) };
    GetCursorInfo(&ci);
    BOOL visible = (ci.flags & CURSOR_SHOWING) != 0;
    if (show && !visible) ShowCursor(TRUE);
    if (!show && visible) ShowCursor(FALSE);
}

// ---------------------------
// Inicialización / Shutdown
// ---------------------------
BOOL Input_Initialize(HWND hwnd) {
    g_hwnd = hwnd;

    memset(&g_inputState, 0, sizeof(InputState));
    g_inputState.cursorVisible = TRUE;
    g_inputState.hasFocus = TRUE;

    RAWINPUTDEVICE rid[2] = {0};

    // Mouse (NOLEGACY para que Windows no genere WM_MOUSEMOVE/BTN al mismo tiempo)
    rid[0].usUsagePage = 0x01;      // Generic Desktop
    rid[0].usUsage     = 0x02;      // Mouse
    rid[0].dwFlags     = RIDEV_INPUTSINK | RIDEV_NOLEGACY;
    rid[0].hwndTarget  = hwnd;

    // Keyboard (opcional NOLEGACY; lo dejamos normal para no romper atajos globales)
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage     = 0x06;      // Keyboard
    rid[1].dwFlags     = RIDEV_INPUTSINK;
    rid[1].hwndTarget  = hwnd;

    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
        printf("Failed to register Raw Input devices (err=%lu)\n", GetLastError());
        return FALSE;
    }

    printf("Raw Input initialized successfully\n");
    return TRUE;
}

void Input_Shutdown() {
    if (g_inputState.cursorLocked) {
        Input_LockCursor(g_hwnd, FALSE);
    }
    if (!g_inputState.cursorVisible) {
        Input_ShowCursor(TRUE);
    }
    ClipCursor(NULL);

    g_hwnd = NULL;
    g_haveAbsRef = FALSE;
    printf("Input system shutdown\n");
}

// ---------------------------
// Ciclo por frame
// ---------------------------
void Input_BeginFrame() {
    // Reset deltas
    g_inputState.dx = 0;
    g_inputState.dy = 0;
    g_inputState.wheel = 0;
    
    // Edge detection: prev = keys and prevButtons = buttons
    memcpy(g_inputState.prev, g_inputState.keys, 256);
    memcpy(g_inputState.prevButtons, g_inputState.buttons, 5);
}

void Input_EndFrame() {
    // (Reservado para future use)
}

// ---------------------------
// Cursor lock / show / clip
// ---------------------------
void Input_LockCursor(HWND hwnd, BOOL lock) {
    (void)hwnd; // Suppress unused parameter warning
    if (lock) {
        // Solo ocultar cursor, no bloquear con ClipCursor
        // El centrado se hará cada frame en update_game()
        g_inputState.cursorLocked = TRUE;
        SetCursorVisible(FALSE);
        printf("Cursor hidden (Minecraft-style centering)\n");
    } else {
        g_inputState.cursorLocked = FALSE;
        SetCursorVisible(TRUE);
        printf("Cursor shown\n");
    }
}

void Input_ShowCursor(BOOL show) {
    SetCursorVisible(show);
    g_inputState.cursorVisible = show ? TRUE : FALSE;
}

void Input_ClipCursor(HWND hwnd, BOOL clip) {
    if (clip) {
        RECT rect;
        GetClientRect(hwnd, &rect);

        POINT tl = { rect.left, rect.top };
        POINT br = { rect.right, rect.bottom };
        ClientToScreen(hwnd, &tl);
        ClientToScreen(hwnd, &br);

        RECT r = { tl.x, tl.y, br.x, br.y };
        ClipCursor(&r);
    } else {
        ClipCursor(NULL);
    }
}

// ---------------------------
// Getters
// ---------------------------
InputState* Input_Get() { return &g_inputState; }
BOOL Input_IsMouseInWindow() { return g_mouseInWindow; }

// ---------------------------
// Procesamiento de mensajes
// ---------------------------
LRESULT Input_HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_MOUSEMOVE: {
        // Solo trackeamos entrada/salida; no usamos WM_MOUSEMOVE para deltas
        if (!g_trackingMouse) {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            g_trackingMouse = TRUE;
            g_mouseInWindow = TRUE;
            // printf("Mouse entered window\n");
        }
        return 0;
    }
    case WM_MOUSELEAVE: {
        g_trackingMouse = FALSE;
        g_mouseInWindow = FALSE;
        // printf("Mouse left window\n");
        return 0;
    }
    case WM_INPUT: {
        if (!g_inputState.hasFocus) return 0; // ignora si no hay foco

        UINT size = 0;
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER)) != 0 || size == 0)
            return 0;

        // Buffer en stack si es pequeño; si no, alloc puntual
        BYTE stackBuf[256];
        BYTE* buf = stackBuf;
        BOOL heap = FALSE;
        if (size > sizeof(stackBuf)) {
            buf = (BYTE*)malloc(size);
            if (!buf) return 0;
            heap = TRUE;
        }

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buf, &size, sizeof(RAWINPUTHEADER)) != size) {
            if (heap) free(buf);
            return 0;
        }

        RAWINPUT* ri = (RAWINPUT*)buf;
        if (ri->header.dwType == RIM_TYPEMOUSE) {
            const RAWMOUSE* m = &ri->data.mouse;
            

            // Movimiento
            if (m->usFlags & MOUSE_MOVE_ABSOLUTE) {
                // Algunos dispositivos reportan ABS. Convertimos a delta.
                // Referencia: valores suelen venir en 0..65535 en pantalla virtual
                // Para estabilidad, tomamos deltas simples desde el último absolute.
                LONG ax = m->lLastX;
                LONG ay = m->lLastY;
                if (!g_haveAbsRef) {
                    g_absRefX = ax;
                    g_absRefY = ay;
                    g_haveAbsRef = TRUE;
                } else {
                    g_inputState.dx += (int)(ax - g_absRefX);
                    g_inputState.dy += (int)(ay - g_absRefY);
                    g_absRefX = ax;
                    g_absRefY = ay;
                }
            } else {
                // Relativo: este es el caso normal de mouse
                g_inputState.dx += (int)m->lLastX;
                g_inputState.dy += (int)m->lLastY;
                g_haveAbsRef = FALSE; // regresamos a relativo
            }

            // Botones
            USHORT bf = m->usButtonFlags;
            if (bf & RI_MOUSE_LEFT_BUTTON_DOWN)   g_inputState.buttons[0] = TRUE;
            if (bf & RI_MOUSE_LEFT_BUTTON_UP)     g_inputState.buttons[0] = FALSE;
            if (bf & RI_MOUSE_RIGHT_BUTTON_DOWN)  g_inputState.buttons[1] = TRUE;
            if (bf & RI_MOUSE_RIGHT_BUTTON_UP)    g_inputState.buttons[1] = FALSE;
            if (bf & RI_MOUSE_MIDDLE_BUTTON_DOWN) g_inputState.buttons[2] = TRUE;
            if (bf & RI_MOUSE_MIDDLE_BUTTON_UP)   g_inputState.buttons[2] = FALSE;

            // Wheel (normalizado en pasos)
            if (bf & RI_MOUSE_WHEEL) {
                SHORT raw = (SHORT)m->usButtonData;
                g_inputState.wheel += (int)(raw / WHEEL_DELTA);
            }
            // (Horizontal wheel si lo necesitas)
            // if (bf & RI_MOUSE_HWHEEL) { ... }
        }

        if (heap) free(buf);
        return 0;
    }
    case WM_MOUSEWHEEL: {
        // Si por algo llega (NOLEGACY a veces no cubre todo), normaliza también
        g_inputState.wheel += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        return 0;
    }
    case WM_SETFOCUS: {
        g_inputState.hasFocus = TRUE;
        // printf("Window gained focus\n");
        return 0;
    }
    case WM_KILLFOCUS: {
        g_inputState.hasFocus = FALSE;
        // Resetea deltas/rueda/botones, pero no fuerces cursor visible aquí (lo decide tu app)
        g_inputState.dx = g_inputState.dy = 0;
        g_inputState.wheel = 0;
        g_inputState.buttons[0] = g_inputState.buttons[1] = g_inputState.buttons[2] = FALSE;
        g_haveAbsRef = FALSE;
        // printf("Window lost focus\n");
        return 0;
    }
    case WM_KEYDOWN: {
        if ((lParam & (1<<30)) == 0) { // sin autorepeat
            if (wParam < 256) g_inputState.keys[wParam] = TRUE;
        }
        return 0;
    }
    case WM_KEYUP: {
        if (wParam < 256) g_inputState.keys[wParam] = FALSE;
        return 0;
    }
    default:
        break;
    }
    return 0;
}
