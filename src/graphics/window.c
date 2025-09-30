#include "graphics/window.h"
#include "graphics/opengl/simple_opengl.h"
#include "graphics/ui/menu.h"
#include "core/math3d.h"
#include "world/chunk_system.h"
#include <stdio.h>
#include <stdlib.h>

// Global game state
static GameState g_game_state = {0};

// Global menu system
static MenuSystem* g_menu = NULL;

// State management functions
void set_app_state(AppState newState) {
    g_game_state.previousState = g_game_state.currentState;
    g_game_state.currentState = newState;
    printf("App state changed: %d -> %d\n", g_game_state.previousState, g_game_state.currentState);
}

AppState get_app_state() {
    return g_game_state.currentState;
}

BOOL is_game_active() {
    return g_game_state.currentState == APP_STATE_GAME_ACTIVE;
}

BOOL is_menu_active() {
    return g_game_state.currentState == APP_STATE_MENU_ACTIVE;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SETFOCUS:
            // Window gained focus - determine state based on menu
            if (g_menu && MenuSystem_IsPaused(g_menu)) {
                set_app_state(APP_STATE_MENU_ACTIVE);
                ShowCursor(TRUE);
                ReleaseCapture();
                g_game_state.mouseCaptured = FALSE;
            } else {
                set_app_state(APP_STATE_GAME_ACTIVE);
                SetCapture(hwnd);
                g_game_state.mouseCaptured = TRUE;
                ShowCursor(FALSE);
                // Reset mouse to center
                SetCursorPos(g_game_state.centerX, g_game_state.centerY);
                g_game_state.mouseX = g_game_state.centerX;
                g_game_state.mouseY = g_game_state.centerY;
                g_game_state.lastMouseX = g_game_state.centerX;
                g_game_state.lastMouseY = g_game_state.centerY;
            }
            return 0;
            
        case WM_KILLFOCUS:
            // Window lost focus - release mouse and show cursor
            set_app_state(APP_STATE_WINDOW_UNFOCUSED);
            ReleaseCapture();
            g_game_state.mouseCaptured = FALSE;
            ShowCursor(TRUE);
            return 0;
            
        case WM_DESTROY:
            g_game_state.isRunning = FALSE;
            PostQuitMessage(0);
            return 0;

        case WM_MOUSEMOVE:
            {
                g_game_state.mouseX = LOWORD(lParam);
                g_game_state.mouseY = HIWORD(lParam);

                // Handle menu mouse movement
                if (is_menu_active()) {
                    MenuSystem_HandleMouseMove(g_menu, g_game_state.mouseX, g_game_state.mouseY);
                }

                // CONTINUOUS MOUSE CONTROL - Only when game is active
                if (is_game_active() && g_game_state.mouseCaptured) {
                    int deltaX = g_game_state.mouseX - g_game_state.lastMouseX;
                    int deltaY = g_game_state.mouseY - g_game_state.lastMouseY;
                
                    // Procesar movimiento del mouse (Minecraft style)
                    // Solo procesar si hay movimiento significativo (evitar micro-movimientos)
                    if (abs(deltaX) > 1 || abs(deltaY) > 1) {
                        // Procesar movimiento del mouse continuamente
                        handle_mouse_input(deltaX, deltaY);
                        
                        // Reset mouse to center to prevent cursor from leaving window
                        // IMPORTANTE: Actualizar lastMouseX/Y ANTES de SetCursorPos para evitar loops
                        g_game_state.lastMouseX = g_game_state.centerX;
                        g_game_state.lastMouseY = g_game_state.centerY;
                        SetCursorPos(g_game_state.centerX, g_game_state.centerY);
                    }
                }
            }
            return 0;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Store HDC in game state for renderer
                g_game_state.window.hdc = hdc;
                
                render_game();
                
                // Render menu if visible
                if (g_menu && MenuSystem_IsPaused(g_menu)) {
                    MenuSystem_Render(g_menu);
                }
                
                EndPaint(hwnd, &ps);
            }
            return 0;
            
        case WM_CREATE:
            {
                // Initialize renderer
                HDC hdc = GetDC(hwnd);
                RECT rect;
                GetClientRect(hwnd, &rect);
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                
                if (!initialize_renderer(hdc, width, height)) {
                    MessageBoxA(hwnd, "Failed to initialize renderer", "Error", MB_OK);
                    return -1;
                }
                
                // Initialize menu system
                g_menu = MenuSystem_Create(hwnd, hdc);
                if (g_menu) {
                    // Create pause menu buttons
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    int centerX = (rect.right - rect.left) / 2;
                    int centerY = (rect.bottom - rect.top) / 2;
                    
                    // Resume button
                    MenuSystem_AddButton(g_menu, centerX - 100, centerY - 20, 200, 40,
                                       "Resume", (Color){50, 150, 50}, (Color){255, 255, 255},
                                       MenuButton_Resume);
                    
                    // Quit button
                    MenuSystem_AddButton(g_menu, centerX - 100, centerY + 30, 200, 40,
                                       "Quit", (Color){150, 50, 50}, (Color){255, 255, 255},
                                       MenuButton_Quit);
                    
                    SetMenuSystem(g_menu);
                    printf("Menu system initialized successfully\n");
                }
                
                ReleaseDC(hwnd, hdc);
                SetTimer(hwnd, 1, 16, NULL); // ~60 FPS
                
                // Initialize mouse position and capture
                g_game_state.centerX = width / 2;
                g_game_state.centerY = height / 2;
                g_game_state.lastMouseX = g_game_state.centerX;
                g_game_state.lastMouseY = g_game_state.centerY;
                g_game_state.mouseX = g_game_state.centerX;
                g_game_state.mouseY = g_game_state.centerY;
                
                // Initialize application state
                set_app_state(APP_STATE_GAME_ACTIVE);
                
                // Initialize mouse capture for continuous control
                SetCapture(hwnd);
                g_game_state.mouseCaptured = TRUE;
                ShowCursor(FALSE);
                SetCursorPos(g_game_state.centerX, g_game_state.centerY);
            }
            return 0;

        case WM_TIMER:
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;

        case WM_LBUTTONDOWN:
            // Handle menu click
            if (is_menu_active()) {
                MenuSystem_HandleMouseClick(g_menu, g_game_state.mouseX, g_game_state.mouseY, TRUE);
                return 0;
            }
            
            // No need to capture mouse here - it's captured automatically on focus
            return 0;

        case WM_LBUTTONUP:
            // Handle menu click release
            if (is_menu_active()) {
                MenuSystem_HandleMouseClick(g_menu, g_game_state.mouseX, g_game_state.mouseY, FALSE);
                return 0;
            }
            
            // No need to release mouse here - it's managed by focus events
            return 0;
            
        case WM_KEYDOWN:
            // Handle ESC key for menu toggle
            if (wParam == VK_ESCAPE) {
                if (g_menu) {
                    MenuSystem_HandleKeyPress(g_menu, wParam);
                }
                return 0;  // Always return early for ESC to prevent double processing
            }
            
            // Only handle game input when game is active
            if (is_game_active()) {
                handle_keyboard_input(wParam);
            }
            return 0;
            
        case WM_KEYUP:
            handle_keyboard_input_up(wParam);
            return 0;

        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                resize_renderer(width, height);
                
                // Re-center mouse when window is resized
                int centerX = width / 2;
                int centerY = height / 2;
                POINT centerPoint = {centerX, centerY};
                ClientToScreen(hwnd, &centerPoint);
                SetCursorPos(centerPoint.x, centerPoint.y);
                g_game_state.lastMouseX = centerX;
                g_game_state.lastMouseY = centerY;
            }
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Initialize game
BOOL initialize_game() {
    printf("Inicializando juego...\n");
    
    // Initialize keyboard state
    printf("Inicializando estado de teclado...\n");
    for (int i = 0; i < 256; i++) {
        g_game_state.keys[i] = FALSE;
    }
    
    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = "VoxelEngineWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    wc.hInstance = GetModuleHandle(NULL);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    
    if (!RegisterClassEx(&wc)) {
        MessageBoxA(NULL, "Failed to register window class", "Error", MB_OK);
        return FALSE;
    }
    
    // Create window
    g_game_state.window.hwnd = CreateWindowExA(
        WS_EX_WINDOWEDGE,
        "VoxelEngineWindow",
        "Voxel Engine - Refactored Architecture",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1200, 800,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!g_game_state.window.hwnd) {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_OK);
        return FALSE;
    }
    
    // Initialize block blueprints system
    printf("Inicializando sistema de blueprints de bloques...\n");
    BlockBlueprint* blueprints = create_block_blueprints();
    if (!blueprints) {
        MessageBoxA(NULL, "Failed to create block blueprints", "Error", MB_OK);
        return FALSE;
    }
    
    // Initialize chunk system for procedural loading
    g_game_state.chunkManager = create_chunk_manager(100, 1); // Aumentar a 100 chunks para más espacio
    if (!g_game_state.chunkManager) {
        MessageBoxA(NULL, "Failed to create chunk manager", "Error", MB_OK);
        return FALSE;
    }
    
    // Initialize terrain generator with world persistence
    g_game_state.terrainGenerator = create_terrain_generator(12345);
    
    // Try to load existing world data
    if (!load_world_data(&g_game_state.terrainGenerator, "world_data.bin")) {
        printf("Mundo nuevo creado con seed: %d\n", g_game_state.terrainGenerator.seed);
        // Save the new world data
        save_world_data(&g_game_state.terrainGenerator, "world_data.bin");
    } else {
        printf("Mundo existente cargado con seed: %d\n", g_game_state.terrainGenerator.seed);
    }
    
    // Generate initial chunks around origin (3x3 grid)
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            VoxelChunk* chunk = get_or_create_chunk(g_game_state.chunkManager, x, y, 0);
            if (chunk && !chunk->isGenerated) {
                generate_chunk_terrain(chunk, &g_game_state.terrainGenerator);
            }
        }
    }
    
    // Initialize mouse position to center
    RECT rect;
    GetClientRect(g_game_state.window.hwnd, &rect);
    g_game_state.mouseX = (rect.right - rect.left) / 2;
    g_game_state.mouseY = (rect.bottom - rect.top) / 2;
    g_game_state.lastMouseX = g_game_state.mouseX;
    g_game_state.lastMouseY = g_game_state.mouseY;
    
    // Store window dimensions
    g_game_state.window.width = rect.right - rect.left;
    g_game_state.window.height = rect.bottom - rect.top;
    
    g_game_state.isRunning = TRUE;
    g_game_state.isInitialized = TRUE;
    
    ShowWindow(g_game_state.window.hwnd, SW_SHOWNORMAL);
    UpdateWindow(g_game_state.window.hwnd);
    
    // Force initial paint
    InvalidateRect(g_game_state.window.hwnd, NULL, TRUE);
    
    printf("Juego inicializado correctamente\n");
    return TRUE;
}

// Cleanup game
void cleanup_game() {
    printf("Limpiando juego...\n");
    
    if (g_game_state.isInitialized) {
        // Cleanup chunk system
        if (g_game_state.chunkManager) {
            destroy_chunk_manager(g_game_state.chunkManager);
        }
        
        // Cleanup terrain generator
        destroy_terrain_generator(&g_game_state.terrainGenerator);
        
        // Cleanup block blueprints
        BlockBlueprint* blueprints = create_block_blueprints(); // Get global blueprints
        if (blueprints) {
            destroy_block_blueprints(blueprints);
        }
        
        // Cleanup player
        Player* player = get_player();
        if (player) {
            destroy_player(player);
        }
        
        // Cleanup menu system
        if (g_menu) {
            MenuSystem_Destroy(g_menu);
            g_menu = NULL;
        }
        
        // Cleanup renderer
        OpenGLContext* context = get_renderer_context();
        if (context) {
            cleanup_renderer(context);
        }
        
        g_game_state.isInitialized = FALSE;
    }
    
    printf("Juego limpiado\n");
}

// Run game loop
void run_game_loop() {
    MSG msg;
    printf("Iniciando bucle principal del juego...\n");
    
    while (g_game_state.isRunning) {
        // Process all pending messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_game_state.isRunning = FALSE;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Update game
        update_game(0.016f); // 60 FPS
        
        // Small sleep to prevent 100% CPU usage
        Sleep(1);
    }
    
    printf("Bucle principal terminado.\n");
}

// Handle mouse input (MINECRAFT STYLE)
void handle_mouse_input(int deltaX, int deltaY) {
    RenderCamera* camera = get_render_camera();
    if (camera) {
        // MINECRAFT: deltaX = yaw (horizontal), deltaY = pitch (vertical)
        camera_rotate(camera, deltaX, deltaY);
    }
}

// Handle keyboard input (MINECRAFT STYLE with continuous movement)
void handle_keyboard_input(WPARAM wParam) {
    RenderFog* fog = get_render_fog();
    
    // Marcar tecla como presionada
    if (wParam < 256) {
        g_game_state.keys[wParam] = TRUE;
    }
    
    // Procesar acciones especiales (no movimiento)
    switch (wParam) {
        case 'F':
            if (fog) {
                fog->enabled = !fog->enabled;
            }
            break;
                case 'C':
                    // Emergency cleanup
                    if (g_game_state.chunkManager) {
                        emergency_cleanup_chunks(g_game_state.chunkManager);
                    }
                    break;
                case 'N':
                    // New world - delete existing data and create new
                    printf("Creando nuevo mundo...\n");
                    delete_world_data("world_data.bin");
                    destroy_terrain_generator(&g_game_state.terrainGenerator);
                    g_game_state.terrainGenerator = create_terrain_generator(rand() % 1000000); // Random seed
                    save_world_data(&g_game_state.terrainGenerator, "world_data.bin");
                    printf("Nuevo mundo creado con seed: %d\n", g_game_state.terrainGenerator.seed);
                    break;
        case VK_ESCAPE:
            // ESC is handled in WM_KEYDOWN, not here
            break;
    }
}

// Handle key release (KEYUP) - necesario para movimiento continuo
void handle_keyboard_input_up(WPARAM wParam) {
    // Marcar tecla como liberada
    if (wParam < 256) {
        g_game_state.keys[wParam] = FALSE;
    }
}

// Update movement from current key states
void update_movement_from_keys() {
    RenderCamera* camera = get_render_camera();
    if (!camera) return;
    
    // Calcular input de movimiento basado en teclas presionadas
    float forward_input = 0.0f;
    float right_input = 0.0f;
    float up_input = 0.0f;
    
    // W/S: Movimiento hacia adelante/atrás
    if (g_game_state.keys['W']) {
        forward_input += 1.0f;
    }
    if (g_game_state.keys['S']) {
        forward_input -= 1.0f;
    }
    
    // A/D: Movimiento lateral
    if (g_game_state.keys['A']) {
        right_input -= 1.0f;
    }
    if (g_game_state.keys['D']) {
        right_input += 1.0f;
    }
    
    // Q/E: Movimiento vertical
    if (g_game_state.keys['Q']) {
        up_input += 1.0f;
    }
    if (g_game_state.keys['E']) {
        up_input -= 1.0f;
    }
    
    // SPACE/SHIFT: Alternativas para movimiento vertical
    if (g_game_state.keys[VK_SPACE]) {
        up_input += 1.0f;
    }
    if (g_game_state.keys[VK_SHIFT]) {
        up_input -= 1.0f;
    }
    
    // Aplicar movimiento continuo al jugador
    Player* player = get_player();
    if (player) {
        player_set_movement_input(player, forward_input, right_input, up_input);
    }
}

// Update game
void update_game(float deltaTime) {
    // Update movement from key states
    update_movement_from_keys();
    
    // Update player physics
    Player* player = get_player();
    if (player) {
        player_update_physics(player, deltaTime);
    }
    
    // Update camera to follow player
    RenderCamera* camera = get_render_camera();
    if (camera && player) {
        camera_follow_player(camera, player);
    }
    
    // Update procedural chunk loading
    if (player && g_game_state.chunkManager) {
        ChunkLoadingConfig config = {
            .renderDistance = 2,      // Renderizar 2 chunks de distancia (reducido)
            .loadDistance = 3,        // Cargar 3 chunks de distancia (reducido)
            .unloadDistance = 4,      // Descargar 4 chunks de distancia (reducido)
            .enableChunkLoading = TRUE  // Habilitar carga dinámica
        };
        
        update_chunk_loading(g_game_state.chunkManager, player->position, config);
    }
}

// Render game
void render_game() {
    RenderCamera* camera = get_render_camera();
    RenderLight* lights = get_render_lights();
    int lightCount = get_render_light_count();
    RenderFog* fog = get_render_fog();
    
    if (!camera || !lights || !fog) {
        printf("ERROR: Renderer components not initialized\n");
        return;
    }
    
    begin_frame();
    
    // Render world chunks
    if (g_game_state.chunkManager) {
        render_chunk_manager(g_game_state.chunkManager, camera->position, camera->forward);
    }
    
    // Render scene
    render_scene(*camera, lights, lightCount, *fog);
    
    end_frame(g_game_state.window.hdc);
}

// Global access
GameState* get_game_state() {
    return &g_game_state;
}
