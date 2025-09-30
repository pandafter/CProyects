#ifndef MENU_H
#define MENU_H

#include <windows.h>
#include "core/math3d.h"

// Button types
typedef enum {
    BUTTON_TEXT,
    BUTTON_IMAGE,
    BUTTON_IMAGE_TEXT
} ButtonType;

// Button structure
typedef struct {
    int x, y, width, height;
    char text[256];
    Color backgroundColor;
    Color textColor;
    HBITMAP image;
    ButtonType type;
    BOOL isHovered;
    BOOL isPressed;
    void (*onClick)(void);
} MenuButton;

// Menu state
typedef enum {
    MENU_MAIN,
    MENU_PAUSED,
    MENU_OPTIONS,
    MENU_NONE
} MenuState;

// Menu system
typedef struct {
    MenuState currentState;
    MenuButton* buttons;
    int buttonCount;
    int maxButtons;
    BOOL isVisible;
    BOOL isPaused;
    DWORD pauseTimestamp;
    HWND hwnd;
    HDC hdc;
    HFONT font;
    HFONT titleFont;
} MenuSystem;

// Function prototypes
MenuSystem* MenuSystem_Create(HWND hwnd, HDC hdc);
void MenuSystem_Destroy(MenuSystem* menu);

// Button management
MenuButton* MenuSystem_AddButton(MenuSystem* menu, int x, int y, int width, int height, 
                                const char* text, Color bgColor, Color textColor, 
                                void (*onClick)(void));
MenuButton* MenuSystem_AddImageButton(MenuSystem* menu, int x, int y, int width, int height,
                                     HBITMAP image, const char* text, void (*onClick)(void));

// Menu control
void MenuSystem_Show(MenuSystem* menu, MenuState state);
void MenuSystem_Hide(MenuSystem* menu);
void MenuSystem_TogglePause(MenuSystem* menu);
BOOL MenuSystem_IsPaused(MenuSystem* menu);

// Input handling
void MenuSystem_HandleMouseMove(MenuSystem* menu, int x, int y);
void MenuSystem_HandleMouseClick(MenuSystem* menu, int x, int y, BOOL isLeftClick);
void MenuSystem_HandleKeyPress(MenuSystem* menu, WPARAM wParam);

// Rendering
void MenuSystem_Render(MenuSystem* menu);
void MenuSystem_RenderButton(MenuSystem* menu, MenuButton* button);
void MenuSystem_RenderBackground(MenuSystem* menu);

// Utility functions
BOOL MenuSystem_IsPointInButton(MenuButton* button, int x, int y);
void MenuSystem_UpdateButtonState(MenuButton* button, int mouseX, int mouseY, BOOL isPressed);

// Callback functions for buttons
void MenuButton_Resume(void);
void MenuButton_Quit(void);

// Global functions for external access
MenuSystem* GetMenuSystem(void);
void SetMenuSystem(MenuSystem* menu);

#endif // MENU_H
