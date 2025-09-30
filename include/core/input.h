#ifndef INPUT_H
#define INPUT_H

#include "types.h"
#include <windows.h>

// ---- Input state structure ----
typedef struct {
    // Mouse (raw, relativo)
    int  dx, dy;          // delta por frame (raw)
    int  wheel;           // pasos de rueda normalizados (WHEEL_DELTA=120)
    BOOL buttons[5];      // 0=L,1=R,2=M,3=X1,4=X2
    BOOL prevButtons[5];  // estado anterior de botones (para edge detection)

    // Teclado
    BOOL keys[256];       // estado actual
    BOOL prev[256];       // estado del frame anterior (para KeyPressed)

    // Estado global
    BOOL cursorLocked;    // cursor atrapado/clippeado en la ventana
    BOOL cursorVisible;   // cursor visible
    BOOL hasFocus;        // ventana con foco
} InputState;

// ---- Init / shutdown / frame ----
BOOL Input_Initialize(HWND hwnd);
void Input_Shutdown(void);

// Llamar al inicio de CADA frame (limpia dx/dy/wheel y hace prev=keys)
void Input_BeginFrame(void);

// Reservado (por si haces algo al final de frame)
void Input_EndFrame(void);

// ---- Mouse helpers ----
void Input_LockCursor(HWND hwnd, BOOL lock);
void Input_ShowCursor(BOOL show);
void Input_ClipCursor(HWND hwnd, BOOL clip);

// ---- State access ----
InputState* Input_Get(void);
BOOL Input_IsMouseInWindow(void);

// (Opcional) fuerza dx/dy/wheel=0, útil al entrar/salir de menú
static inline void Input_ResetMouse(void) {
    InputState* s = Input_Get();
    if (s) {
        s->dx = s->dy = 0;
        s->wheel = 0;
    }
}

// ---- Raw Input / Win32 message hook ----
LRESULT Input_HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ---- Keyboard query (edge detection) ----
// Devuelve 1 si la tecla está abajo este frame.
static inline int KeyDown(int vk) {
    const InputState* s = Input_Get();
    if (!s) return 0;
    return (vk >= 0 && vk < 256) ? (s->keys[vk] != 0) : 0;
}

// Devuelve 1 solo en el frame en que la tecla pasó de "no" a "sí".
static inline int KeyPressed(int vk) {
    const InputState* s = Input_Get();
    if (!s) return 0;
    if (vk < 0 || vk >= 256) return 0;
    return (s->keys[vk] != 0) && (s->prev[vk] == 0);
}

#endif // INPUT_H
