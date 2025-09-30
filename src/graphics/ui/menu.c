#include "world/chunk_system.h"  // Must be included before renderer.h
#include "graphics/ui/menu.h"
#include "graphics/window.h"
#include "core/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global menu system instance
static MenuSystem* g_menuSystem = NULL;

// Create menu system
MenuSystem* MenuSystem_Create(HWND hwnd, HDC hdc) {
    MenuSystem* menu = (MenuSystem*)malloc(sizeof(MenuSystem));
    if (!menu) return NULL;
    
    memset(menu, 0, sizeof(MenuSystem));
    menu->hwnd = hwnd;
    menu->hdc = hdc;
    menu->currentState = MENU_NONE;
    menu->isVisible = FALSE;
    menu->isPaused = FALSE;
    menu->maxButtons = 10;
    menu->buttonCount = 0;
    
    // Allocate button array
    menu->buttons = (MenuButton*)malloc(sizeof(MenuButton) * menu->maxButtons);
    if (!menu->buttons) {
        free(menu);
        return NULL;
    }
    
    // Create fonts
    menu->font = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    
    menu->titleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    
    printf("Menu system created successfully\n");
    return menu;
}

// Destroy menu system
void MenuSystem_Destroy(MenuSystem* menu) {
    if (!menu) return;
    
    if (menu->buttons) {
        free(menu->buttons);
    }
    
    if (menu->font) {
        DeleteObject(menu->font);
    }
    
    if (menu->titleFont) {
        DeleteObject(menu->titleFont);
    }
    
    free(menu);
    printf("Menu system destroyed\n");
}

// Add text button
MenuButton* MenuSystem_AddButton(MenuSystem* menu, int x, int y, int width, int height, 
                                const char* text, Color bgColor, Color textColor, 
                                void (*onClick)(void)) {
    if (!menu || menu->buttonCount >= menu->maxButtons) return NULL;
    
    MenuButton* button = &menu->buttons[menu->buttonCount];
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->backgroundColor = bgColor;
    button->textColor = textColor;
    button->onClick = onClick;
    button->type = BUTTON_TEXT;
    button->isHovered = FALSE;
    button->isPressed = FALSE;
    button->image = NULL;
    
    strncpy(button->text, text, sizeof(button->text) - 1);
    button->text[sizeof(button->text) - 1] = '\0';
    
    menu->buttonCount++;
    return button;
}

// Add image button
MenuButton* MenuSystem_AddImageButton(MenuSystem* menu, int x, int y, int width, int height,
                                     HBITMAP image, const char* text, void (*onClick)(void)) {
    if (!menu || menu->buttonCount >= menu->maxButtons) return NULL;
    
    MenuButton* button = &menu->buttons[menu->buttonCount];
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->image = image;
    button->onClick = onClick;
    button->type = (text && strlen(text) > 0) ? BUTTON_IMAGE_TEXT : BUTTON_IMAGE;
    button->isHovered = FALSE;
    button->isPressed = FALSE;
    
    if (text) {
        strncpy(button->text, text, sizeof(button->text) - 1);
        button->text[sizeof(button->text) - 1] = '\0';
    } else {
        button->text[0] = '\0';
    }
    
    // Default colors for image buttons
    button->backgroundColor = (Color){100, 100, 100};
    button->textColor = (Color){255, 255, 255};
    
    menu->buttonCount++;
    return button;
}

// Show menu
void MenuSystem_Show(MenuSystem* menu, MenuState state) {
    if (!menu) return;
    
    menu->currentState = state;
    menu->isVisible = TRUE;
    
    if (state == MENU_PAUSED) {
        menu->isPaused = TRUE;
        menu->pauseTimestamp = GetTickCount();
        printf("Game paused at timestamp: %lu\n", menu->pauseTimestamp);
    }
    
    // Show cursor and release mouse capture when menu is shown
    Input_ShowCursor(TRUE);
    Input_LockCursor(menu->hwnd, FALSE);
    
    // Force cursor to be visible and set to arrow
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    
    // Update application state to menu active
    set_app_state(APP_STATE_MENU_ACTIVE);
}

// Hide menu
void MenuSystem_Hide(MenuSystem* menu) {
    if (!menu) return;
    
    menu->isVisible = FALSE;
    menu->isPaused = FALSE;
    
    // Hide cursor and re-capture mouse when returning to game
    Input_ShowCursor(FALSE);
    Input_LockCursor(menu->hwnd, TRUE);
    
    // Update application state to game active
    set_app_state(APP_STATE_GAME_ACTIVE);
    
    // Re-capture mouse (centering se hace automáticamente cada frame)
    GameState* gameState = get_game_state();
    if (gameState) {
        gameState->mouseCaptured = TRUE;
        printf("Game resumed: Mouse captured (auto-centering enabled)\n");
    }
}

// Toggle pause
void MenuSystem_TogglePause(MenuSystem* menu) {
    if (!menu) return;
    
    if (menu->isVisible && menu->currentState == MENU_PAUSED) {
        MenuSystem_Hide(menu);
    } else {
        MenuSystem_Show(menu, MENU_PAUSED);
    }
}

// Check if paused
BOOL MenuSystem_IsPaused(MenuSystem* menu) {
    return menu ? menu->isPaused : FALSE;
}

// Handle mouse move
void MenuSystem_HandleMouseMove(MenuSystem* menu, int x, int y) {
    if (!menu || !menu->isVisible) return;
    
    for (int i = 0; i < menu->buttonCount; i++) {
        MenuButton* button = &menu->buttons[i];
        BOOL wasHovered = button->isHovered;
        button->isHovered = MenuSystem_IsPointInButton(button, x, y);
        
        // Update cursor if hover state changed
        if (wasHovered != button->isHovered) {
            SetCursor(button->isHovered ? LoadCursor(NULL, IDC_HAND) : LoadCursor(NULL, IDC_ARROW));
        }
    }
}

// Handle mouse click
void MenuSystem_HandleMouseClick(MenuSystem* menu, int x, int y, BOOL isLeftClick) {
    if (!menu || !menu->isVisible || !isLeftClick) return;
    
    for (int i = 0; i < menu->buttonCount; i++) {
        MenuButton* button = &menu->buttons[i];
        if (MenuSystem_IsPointInButton(button, x, y)) {
            button->isPressed = TRUE;
            if (button->onClick) {
                button->onClick();
            }
            break;
        }
    }
}

// Handle key press
void MenuSystem_HandleKeyPress(MenuSystem* menu, WPARAM wParam) {
    if (!menu) return;
    
    if (wParam == VK_ESCAPE) {
        // Simple toggle with debouncing to prevent flickering
        static DWORD lastToggleTime = 0;
        DWORD currentTime = GetTickCount();
        
        // Debounce: only allow toggle if 200ms have passed since last toggle
        if (currentTime - lastToggleTime > 200) {
            if (menu->isVisible) {
                MenuSystem_Hide(menu);
            } else {
                MenuSystem_Show(menu, MENU_PAUSED);
            }
            lastToggleTime = currentTime;
        }
    }
}

// Render menu
void MenuSystem_Render(MenuSystem* menu) {
    if (!menu || !menu->isVisible) return;
    
    MenuSystem_RenderBackground(menu);
    
    for (int i = 0; i < menu->buttonCount; i++) {
        MenuSystem_RenderButton(menu, &menu->buttons[i]);
    }
}

// Render button
void MenuSystem_RenderButton(MenuSystem* menu, MenuButton* button) {
    if (!menu || !button) return;
    
    HDC hdc = menu->hdc;
    HBRUSH brush;
    HPEN pen;
    
    // Set button colors based on state
    Color bgColor = button->backgroundColor;
    Color textColor = button->textColor;
    
    if (button->isHovered) {
        // Lighten background color on hover
        bgColor.r = (bgColor.r + 50 > 255) ? 255 : bgColor.r + 50;
        bgColor.g = (bgColor.g + 50 > 255) ? 255 : bgColor.g + 50;
        bgColor.b = (bgColor.b + 50 > 255) ? 255 : bgColor.b + 50;
    }
    
    if (button->isPressed) {
        // Darken background color when pressed
        bgColor.r = (bgColor.r < 50) ? 0 : bgColor.r - 50;
        bgColor.g = (bgColor.g < 50) ? 0 : bgColor.g - 50;
        bgColor.b = (bgColor.b < 50) ? 0 : bgColor.b - 50;
    }
    
    // Create brush and pen
    brush = CreateSolidBrush(RGB(bgColor.r, bgColor.g, bgColor.b));
    pen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
    
    // Select objects
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    
    // Draw button rectangle
    Rectangle(hdc, button->x, button->y, button->x + button->width, button->y + button->height);
    
    // Draw image if present
    if (button->image && (button->type == BUTTON_IMAGE || button->type == BUTTON_IMAGE_TEXT)) {
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, button->image);
        
        BITMAP bm;
        GetObject(button->image, sizeof(BITMAP), &bm);
        
        // Center image in button
        int imgX = button->x + (button->width - bm.bmWidth) / 2;
        int imgY = button->y + (button->height - bm.bmHeight) / 2;
        
        if (button->type == BUTTON_IMAGE_TEXT) {
            // Adjust image position to make room for text
            imgY = button->y + 10;
        }
        
        BitBlt(hdc, imgX, imgY, bm.bmWidth, bm.bmHeight, memDC, 0, 0, SRCCOPY);
        
        SelectObject(memDC, oldBitmap);
        DeleteDC(memDC);
    }
    
    // Draw text
    if (strlen(button->text) > 0) {
        HFONT oldFont = (HFONT)SelectObject(hdc, menu->font);
        SetTextColor(hdc, RGB(textColor.r, textColor.g, textColor.b));
        SetBkMode(hdc, TRANSPARENT);
        
        RECT textRect = {button->x, button->y, button->x + button->width, button->y + button->height};
        
        if (button->type == BUTTON_IMAGE_TEXT) {
            // Adjust text position for image
            textRect.top += 40;
        }
        
        DrawText(hdc, button->text, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, oldFont);
    }
    
    // Restore objects
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    
    // Clean up
    DeleteObject(brush);
    DeleteObject(pen);
}

// Render background
void MenuSystem_RenderBackground(MenuSystem* menu) {
    if (!menu) return;
    
    HDC hdc = menu->hdc;
    RECT clientRect;
    GetClientRect(menu->hwnd, &clientRect);
    
    // Create semi-transparent background with alpha blending
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    
    // Draw dark overlay that covers the entire screen
    FillRect(hdc, &clientRect, brush);
    
    // Draw title
    if (menu->currentState == MENU_PAUSED) {
        HFONT oldFont = (HFONT)SelectObject(hdc, menu->titleFont);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        
        RECT titleRect = {0, 100, clientRect.right, 150};
        DrawText(hdc, "GAME PAUSED", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, oldFont);
    }
    
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

// Check if point is in button
BOOL MenuSystem_IsPointInButton(MenuButton* button, int x, int y) {
    if (!button) return FALSE;
    
    return (x >= button->x && x <= button->x + button->width &&
            y >= button->y && y <= button->y + button->height);
}

// Update button state
void MenuSystem_UpdateButtonState(MenuButton* button, int mouseX, int mouseY, BOOL isPressed) {
    if (!button) return;
    
    button->isHovered = MenuSystem_IsPointInButton(button, mouseX, mouseY);
    button->isPressed = isPressed && button->isHovered;
}

// Callback functions
void MenuButton_Resume(void) {
    if (g_menuSystem) {
        MenuSystem_Hide(g_menuSystem);
        printf("Game resumed\n");
    }
}

void MenuButton_Quit(void) {
    if (g_menuSystem && g_menuSystem->hwnd) {
        PostMessage(g_menuSystem->hwnd, WM_CLOSE, 0, 0);
        printf("Game quit requested\n");
    }
}

// Global functions for external access
MenuSystem* GetMenuSystem(void) {
    return g_menuSystem;
}

void SetMenuSystem(MenuSystem* menu) {
    g_menuSystem = menu;
}
