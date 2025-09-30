#ifndef WINDOW_H
#define WINDOW_H

#include "core/types.h"
#include "graphics/renderer.h"
#include "world/chunk_system.h"
#include <windows.h>

// Window structure
typedef struct {
    HWND hwnd;
    HDC hdc;
    int width;
    int height;
    BOOL isRunning;
    BOOL mouseCaptured;
    int mouseX, mouseY;
    int lastMouseX, lastMouseY;
} GameWindow;

// Application states
typedef enum {
    APP_STATE_GAME_ACTIVE,    // Game is running, mouse captured
    APP_STATE_MENU_ACTIVE,    // Menu is open, mouse free
    APP_STATE_WINDOW_FOCUSED, // Window has focus but game paused
    APP_STATE_WINDOW_UNFOCUSED // Window lost focus
} AppState;

// Game state
typedef struct {
    GameWindow window;
    ChunkManager* chunkManager;
    TerrainGenerator terrainGenerator;
    BOOL isInitialized;
    BOOL isRunning;
    BOOL mouseCaptured;
    int mouseX, mouseY;
    int lastMouseX, lastMouseY;
    int centerX, centerY;  // Window center for mouse reset
    
    // Application state management
    AppState currentState;
    AppState previousState;
    
    // Keyboard state for continuous movement
    BOOL keys[256];  // Array para rastrear estado de todas las teclas
} GameState;

// Function declarations
BOOL initialize_game();
void cleanup_game();
void run_game_loop();
void handle_window_messages();

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Input handling
void handle_mouse_input(int deltaX, int deltaY);
void handle_keyboard_input(WPARAM wParam);
void handle_keyboard_input_up(WPARAM wParam);
void update_movement_from_keys();

// Game update
void update_game(float deltaTime);
void render_game();

// Global access
GameState* get_game_state();

// State management functions
void set_app_state(AppState newState);
AppState get_app_state();
BOOL is_game_active();
BOOL is_menu_active();

#endif // WINDOW_H
