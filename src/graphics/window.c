#include "world/chunk_system.h"  // Must be included before renderer.h
#include "graphics/window.h"
#include "graphics/opengl/simple_opengl.h"
#include "graphics/ui/menu.h"
#include "core/math3d.h"
#include "core/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <windowsx.h>

// Global game state
static GameState g_game_state = {0};

// Global menu system
static MenuSystem* g_menu = NULL;

// Function declarations
void render_crosshair();

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

// Helpers de modo
static void EnterGameMode(HWND hwnd) {
    set_app_state(APP_STATE_GAME_ACTIVE);
    Input_ShowCursor(FALSE);
    Input_LockCursor(hwnd, TRUE);
    get_game_state()->mouseCaptured = TRUE;
}

static void EnterMenuMode(HWND hwnd) {
    set_app_state(APP_STATE_MENU_ACTIVE);
    Input_LockCursor(hwnd, FALSE);
    Input_ShowCursor(TRUE);
    get_game_state()->mouseCaptured = FALSE;
}


// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SETFOCUS:
            if (g_menu && MenuSystem_IsPaused(g_menu)) {
                EnterMenuMode(hwnd);
            } else {
                EnterGameMode(hwnd);
            }
            Input_HandleMessage(hwnd, uMsg, wParam, lParam); // ← IMPORTANTE
            return 0;
        
        case WM_KILLFOCUS:
            set_app_state(APP_STATE_WINDOW_UNFOCUSED);
            Input_LockCursor(hwnd, FALSE);
            Input_ShowCursor(TRUE);
            g_game_state.mouseCaptured = FALSE;
            Input_HandleMessage(hwnd, uMsg, wParam, lParam); // ← IMPORTANTE
            return 0;
            
        case WM_DESTROY:
            g_game_state.isRunning = FALSE;
            PostQuitMessage(0);
            return 0;

        case WM_SETCURSOR:
            if (is_menu_active()) {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE; // ya lo manejé
            }
            // en juego, si está oculto, deja que Windows lo esconda
            break;
        

        case WM_MOUSEMOVE:
        {
            if (is_menu_active()) {
                g_game_state.mouseX = GET_X_LPARAM(lParam);
                g_game_state.mouseY = GET_Y_LPARAM(lParam);
            }
            // Solo tracking enter/leave
            Input_HandleMessage(hwnd, uMsg, wParam, lParam);
            return 0;
        }
        
        case WM_MOUSELEAVE:
            Input_HandleMessage(hwnd, uMsg, wParam, lParam);
            return 0;
        
        case WM_INPUT:
            // NO proceses Raw Input cuando el menú está abierto
            if (is_game_active()) {
                Input_HandleMessage(hwnd, uMsg, wParam, lParam);
            }
            return 0;
        
        case WM_MOUSEWHEEL:
            // La rueda sirve también en menú si quisieras, pero por consistencia:
            if (is_game_active()) {
                Input_HandleMessage(hwnd, uMsg, wParam, lParam);
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
                
                // Initialize mouse for camera control (locked to window, centered)
                Input_LockCursor(hwnd, TRUE);
                Input_ShowCursor(FALSE);
                g_game_state.mouseCaptured = TRUE;
                

                
                printf("Game started: Mouse locked to window center\n");
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
            
            // Handle block breaking in game mode
            if (is_game_active() && g_game_state.mouseCaptured && g_game_state.chunkManager) {
                RenderCamera* camera = get_render_camera();
                if (camera) {
                    BlockSelection selection = raycast_from_camera(camera, g_game_state.chunkManager, 5.0f);
                    handle_block_interaction(g_game_state.chunkManager, &selection, TRUE, FALSE);
                }
            }
            return 0;

        case WM_LBUTTONUP:
            // Handle menu click release
            if (is_menu_active()) {
                MenuSystem_HandleMouseClick(g_menu, g_game_state.mouseX, g_game_state.mouseY, FALSE);
                return 0;
            }
            
            // No need to release mouse here - it's managed by focus events
            return 0;
        
        case WM_RBUTTONDOWN:
            // Handle block placing in game mode
            if (is_game_active() && g_game_state.mouseCaptured && g_game_state.chunkManager) {
                RenderCamera* camera = get_render_camera();
                if (camera) {
                    BlockSelection selection = raycast_from_camera(camera, g_game_state.chunkManager, 5.0f);
                    handle_block_interaction(g_game_state.chunkManager, &selection, FALSE, TRUE);
                }
            }
            return 0;
        
        case WM_RBUTTONUP:
            // Right click release - no special handling needed
            return 0;
        
            case WM_KEYDOWN:
                if (wParam == VK_ESCAPE) {
                    // ignorar autorepeat (bit 30 encendido cuando es repetido)
                    if (lParam & (1 << 30)) return 0;
            
                    if (is_game_active()) {
                        if (g_menu) MenuSystem_Show(g_menu, MENU_PAUSED);
                        EnterMenuMode(hwnd);   // ← estado + cursor coherente
                    } else if (is_menu_active()) {
                        if (g_menu) MenuSystem_Hide(g_menu);
                        EnterGameMode(hwnd);   // ← estado + cursor coherente
                    }
                    return 0;
                }
            
                // resto igual
                Input_HandleMessage(hwnd, uMsg, wParam, lParam);
                if (is_game_active()) handle_keyboard_input(wParam);
                return 0;
            
        case WM_KEYUP:
            // Handle Raw Input keyboard
            Input_HandleMessage(hwnd, uMsg, wParam, lParam);
            handle_keyboard_input_up(wParam);
            return 0;

        case WM_SIZE:
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                resize_renderer(width, height);
                
                // Update center coordinates
                g_game_state.centerX = width / 2;
                g_game_state.centerY = height / 2;
            }
            return 0;

        case WM_ERASEBKGND:
            return 1; // ← ya nos encargamos con OpenGL, evita flicker
        

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
    wc.hbrBackground = NULL;
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
    g_game_state.centerX = (rect.right - rect.left) / 2;
    g_game_state.centerY = (rect.bottom - rect.top) / 2;
    
    // Store window dimensions
    g_game_state.window.width = rect.right - rect.left;
    g_game_state.window.height = rect.bottom - rect.top;
    
    // Initialize Raw Input system
    if (!Input_Initialize(g_game_state.window.hwnd)) {
        MessageBoxA(NULL, "Failed to initialize input system", "Error", MB_OK);
        return FALSE;
    }
    
    g_game_state.isRunning = TRUE;
    g_game_state.isInitialized = TRUE;
    
    ShowWindow(g_game_state.window.hwnd, SW_SHOWNORMAL);
    UpdateWindow(g_game_state.window.hwnd);
    
    // Force focus to window
    SetForegroundWindow(g_game_state.window.hwnd);
    SetFocus(g_game_state.window.hwnd);
    
    // Force initial paint
    InvalidateRect(g_game_state.window.hwnd, NULL, TRUE);
    
    printf("Juego inicializado correctamente\n");
    return TRUE;
}

// Cleanup game
void cleanup_game() {
    printf("Limpiando juego...\n");
    
    // Shutdown input system
    Input_Shutdown();
    
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

// Test all functionalities for debugging
void test_all_functionalities() {
    printf("\n=== COMPREHENSIVE FUNCTIONALITY TEST ===\n");
    
    // Test 1: Player System
    printf("\n1. PLAYER SYSTEM TEST:\n");
    Player* player = get_player();
    if (player) {
        printf("   ✓ Player exists\n");
        printf("   - Position: (%.2f, %.2f, %.2f)\n", player->position.x, player->position.y, player->position.z);
        printf("   - Velocity: (%.2f, %.2f, %.2f)\n", player->velocity.x, player->velocity.y, player->velocity.z);
        printf("   - Camera Yaw: %.3f, Pitch: %.3f\n", player->camera_yaw, player->camera_pitch);
        printf("   - Flying: %s\n", player->isFlying ? "YES" : "NO");
        printf("   - Grounded: %s\n", player->isGrounded ? "YES" : "NO");
        printf("   - Camera ID: %d\n", player->camera_id);
    } else {
        printf("   ✗ Player is NULL!\n");
    }
    
    // Test 2: Camera System
    printf("\n2. CAMERA SYSTEM TEST:\n");
    RenderCamera* camera = get_render_camera();
    if (camera) {
        printf("   ✓ Camera exists\n");
        printf("   - Position: (%.2f, %.2f, %.2f)\n", camera->position.x, camera->position.y, camera->position.z);
        printf("   - Yaw: %.3f, Pitch: %.3f\n", camera->yaw, camera->pitch);
        printf("   - Forward: (%.3f, %.3f, %.3f)\n", camera->forward.x, camera->forward.y, camera->forward.z);
        printf("   - Right: (%.3f, %.3f, %.3f)\n", camera->right.x, camera->right.y, camera->right.z);
        printf("   - Up: (%.3f, %.3f, %.3f)\n", camera->up.x, camera->up.y, camera->up.z);
    } else {
        printf("   ✗ Camera is NULL!\n");
    }
    
    // Test 3: Input System
    printf("\n3. INPUT SYSTEM TEST:\n");
    InputState* input = Input_Get();
    if (input) {
        printf("   ✓ Input system exists\n");
        printf("   - Mouse Delta: dx=%d, dy=%d\n", input->dx, input->dy);
        printf("   - Mouse In Window: %s\n", Input_IsMouseInWindow() ? "YES" : "NO");
        printf("   - Cursor Visible: %s\n", input->cursorVisible ? "YES" : "NO");
        printf("   - Cursor Locked: %s\n", input->cursorLocked ? "YES" : "NO");
        printf("   - Wheel Delta: %d\n", input->wheel);
        printf("   - Left Mouse Button: %s\n", input->buttons[0] ? "PRESSED" : "RELEASED");
        printf("   - Right Mouse Button: %s\n", input->buttons[1] ? "PRESSED" : "RELEASED");
        printf("   - Middle Mouse Button: %s\n", input->buttons[2] ? "PRESSED" : "RELEASED");
    } else {
        printf("   ✗ Input system is NULL!\n");
    }
    
    // Test 4: Game State
    printf("\n4. GAME STATE TEST:\n");
    printf("   - Mouse Captured: %s\n", g_game_state.mouseCaptured ? "YES" : "NO");
    printf("   - App State: %d\n", g_game_state.currentState);
    printf("   - Center: (%d, %d)\n", g_game_state.centerX, g_game_state.centerY);
    printf("   - Mouse Pos: (%d, %d)\n", g_game_state.mouseX, g_game_state.mouseY);
    printf("   - Last Mouse: (%d, %d)\n", g_game_state.lastMouseX, g_game_state.lastMouseY);
    
    // Test 5: Lighting System
    printf("\n5. LIGHTING SYSTEM TEST:\n");
    RenderLight* lights = get_render_lights();
    int lightCount = get_render_light_count();
    printf("   - Light Count: %d\n", lightCount);
    for (int i = 0; i < lightCount; i++) {
        printf("   - Light %d: Type=%d, Intensity=%.2f\n", i, lights[i].type, lights[i].intensity);
        printf("     Direction: (%.3f, %.3f, %.3f)\n", lights[i].direction.x, lights[i].direction.y, lights[i].direction.z);
        printf("     Color: (%d, %d, %d)\n", lights[i].color.r, lights[i].color.g, lights[i].color.b);
    }
    
    // Test 6: Fog System
    printf("\n6. FOG SYSTEM TEST:\n");
    RenderFog* fog = get_render_fog();
    if (fog) {
        printf("   ✓ Fog system exists\n");
        printf("   - Enabled: %s\n", fog->enabled ? "YES" : "NO");
        printf("   - Density: %.3f\n", fog->density);
        printf("   - Color: (%d, %d, %d)\n", fog->color.r, fog->color.g, fog->color.b);
    } else {
        printf("   ✗ Fog system is NULL!\n");
    }
    
    // Test 7: Volumetric System
    printf("\n7. VOLUMETRIC SYSTEM TEST:\n");
    VolumetricSystem* volumetric = get_volumetric_system();
    if (volumetric) {
        printf("   ✓ Volumetric system exists\n");
        printf("   - Enabled: %s\n", volumetric->enableVolumetrics ? "YES" : "NO");
        printf("   - Fog Density: %.3f\n", volumetric->fogDensity);
        printf("   - Max Distance: %.2f\n", volumetric->fogMaxDistance);
    } else {
        printf("   ✗ Volumetric system is NULL!\n");
    }
    
    // Test 8: Shadow System
    printf("\n8. SHADOW SYSTEM TEST:\n");
    AdvancedShadowSystem* shadow = get_shadow_system();
    if (shadow) {
        printf("   ✓ Shadow system exists\n");
        printf("   - Enabled: %s\n", shadow->enableShadows ? "YES" : "NO");
        printf("   - Bias: %.4f\n", shadow->bias);
        printf("   - PCF Size: %d\n", shadow->pcfSize);
    } else {
        printf("   ✗ Shadow system is NULL!\n");
    }
    
    // Test 9: Key States
    printf("\n9. KEY STATES TEST:\n");
    printf("   - W: %s\n", g_game_state.keys['W'] ? "PRESSED" : "RELEASED");
    printf("   - A: %s\n", g_game_state.keys['A'] ? "PRESSED" : "RELEASED");
    printf("   - S: %s\n", g_game_state.keys['S'] ? "PRESSED" : "RELEASED");
    printf("   - D: %s\n", g_game_state.keys['D'] ? "PRESSED" : "RELEASED");
    printf("   - Q: %s\n", g_game_state.keys['Q'] ? "PRESSED" : "RELEASED");
    printf("   - E: %s\n", g_game_state.keys['E'] ? "PRESSED" : "RELEASED");
    printf("   - Space: %s\n", g_game_state.keys[VK_SPACE] ? "PRESSED" : "RELEASED");
    printf("   - Shift: %s\n", g_game_state.keys[VK_SHIFT] ? "PRESSED" : "RELEASED");
    
    printf("\n=== TEST COMPLETE ===\n");
    printf("Press T again to run another test.\n\n");
}

// Handle mouse input (Raw Input) - now integrated with player
void handle_mouse_input(int deltaX, int deltaY) {
    Player* player = get_player();
    if (player) {
        // Use player's camera control system
        player_update_camera_from_mouse(player, deltaX, deltaY);
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
                printf("Fog %s\n", fog->enabled ? "ENABLED" : "DISABLED");
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
        case VK_SPACE:
            // Handle space input for flight toggle
            Player* player = get_player();
            if (player) {
                player_handle_space_input(player);
            }
            break;
        case VK_ESCAPE:
            // ESC is handled in WM_KEYDOWN, not here
            break;
        case 'T':
            // T key for testing/debugging
            printf("=== FUNCTIONALITY TEST ===\n");
            test_all_functionalities();
            break;
    }
}

// Handle key release (KEYUP) - necesario para movimiento continuo
void handle_keyboard_input_up(WPARAM wParam) {
    // Marcar tecla como liberada
    if (wParam < 256) {
        g_game_state.keys[wParam] = FALSE;
    }
    
    // Handle space release for variable jump
    if (wParam == VK_SPACE) {
        Player* player = get_player();
        if (player) {
            player_handle_space_release(player);
        }
    }
}

// Update movement from current key states
void update_movement_from_keys() {
    Player* player = get_player();
    if (!player) return;
    
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
    
    // A/D: Movimiento lateral (prioridad sobre Q/E)
    if (g_game_state.keys['A']) {
        right_input -= 1.0f;
    } else if (g_game_state.keys['D']) {
        right_input += 1.0f;
    }
    
    // Q/E: Strafe adicional solo si A/D no están presionadas
    if (!g_game_state.keys['A'] && !g_game_state.keys['D']) {
        if (g_game_state.keys['Q']) {
            right_input -= 1.0f;
        }
        if (g_game_state.keys['E']) {
            right_input += 1.0f;
        }
    }
    
    // SPACE: Handle flight vertical movement only (jump is handled separately)
    if (g_game_state.keys[VK_SPACE]) {
        if (player->isFlying) {
            up_input += 1.0f; // Fly up when in flight mode
        }
        // Note: Variable jump and flight toggle are handled in handle_keyboard_input
    }
    
    // SHIFT: Fly down when in flight mode
    if (g_game_state.keys[VK_SHIFT]) {
        if (player->isFlying) {
            up_input -= 1.0f; // Fly down when in flight mode
        }
    }
    
    // Aplicar movimiento continuo al jugador
    player_set_movement_input(player, forward_input, right_input, up_input);
}

// Update game
void update_game(float deltaTime) {
    // Procesar input del mouse ANTES de resetear los deltas
    if (is_game_active() && g_game_state.mouseCaptured) {
        InputState* input = Input_Get();
        
        // Debug: mostrar estado del mouse cada 60 frames (1 segundo)
        static int debugCounter = 0;
        if (++debugCounter >= 60) {
            debugCounter = 0;
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            ScreenToClient(g_game_state.window.hwnd, &cursorPos);
            printf("DEBUG: Mouse captured=%s, dx=%d, dy=%d, cursor=(%ld,%ld), hasFocus=%s\n", 
                   g_game_state.mouseCaptured ? "YES" : "NO",
                   input->dx, input->dy, (long)cursorPos.x, (long)cursorPos.y,
                   input->hasFocus ? "YES" : "NO");
        }
        
        if (input->dx || input->dy) {
            printf("DEBUG: Processing mouse input: dx=%d, dy=%d\n", input->dx, input->dy);
            handle_mouse_input(input->dx, input->dy);
        }
        
        // Centrar cursor cada frame (estilo Minecraft)
        RECT rect;
        GetClientRect(g_game_state.window.hwnd, &rect);
        int centerX = (rect.left + rect.right) / 2;
        int centerY = (rect.top + rect.bottom) / 2;
        
        POINT centerPoint = {centerX, centerY};
        ClientToScreen(g_game_state.window.hwnd, &centerPoint);
        SetCursorPos(centerPoint.x, centerPoint.y);
    }
    
    // Resetear deltas DESPUÉS de procesarlos
    Input_BeginFrame();

    update_movement_from_keys();
    
    // Update player physics
    Player* player = get_player();
    if (player) {
        player_update_jump_system(player, deltaTime);
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
    
    // Handle block interaction with edge detection
    if (is_game_active() && g_game_state.mouseCaptured && g_game_state.chunkManager) {
        InputState* input = Input_Get();
        
        // Edge detection for mouse buttons
        BOOL leftClick = input->buttons[0] && !input->prevButtons[0];  // Left click edge
        BOOL rightClick = input->buttons[1] && !input->prevButtons[1]; // Right click edge
        
        if (leftClick || rightClick) {
            RenderCamera* camera = get_render_camera();
            if (camera) {
                BlockSelection selection = raycast_from_camera(camera, g_game_state.chunkManager, 6.0f);
                handle_block_interaction(g_game_state.chunkManager, &selection, leftClick, rightClick);
            }
        }
        
        // Handle mouse wheel for block type selection
        if (input->wheel != 0) {
            Player* player = get_player();
            if (player) {
                VoxelType currentType = player_get_current_block_type(player);
                VoxelType newType = currentType;
                
                if (input->wheel > 0) {
                    // Scroll up - next block type
                    newType = (currentType + 1) % 16; // Cycle through 0-15
                } else {
                    // Scroll down - previous block type
                    newType = (currentType - 1 + 16) % 16; // Cycle through 0-15
                }
                
                player_change_block_type(player, newType);
            }
        }
        
        // Handle number keys 1-9 for block type selection
        for (int i = 1; i <= 9; i++) {
            if (input->keys['0' + i] && !input->prev['0' + i]) { // Edge detection for number keys
                Player* player = get_player();
                if (player) {
                    VoxelType newType = (VoxelType)((i - 1) % 16); // Map 1-9 to 0-15 block types
                    player_change_block_type(player, newType);
                }
            }
        }
    }
    
    // End input frame
    Input_EndFrame();
}

// Render game
void render_game() {
    RenderCamera* camera = get_render_camera();
    RenderLight* lights = get_render_lights();
    int lightCount = get_render_light_count();
    RenderFog* fog = get_render_fog();
    Player* player = get_player();
    
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
    
    // Render block selection (raycast from camera)
    if (is_game_active() && g_game_state.chunkManager) {
        BlockSelection selection = raycast_from_camera(camera, g_game_state.chunkManager, 5.0f);
        if (selection.hit) {
            render_block_selection(&selection);
        }
    }
    
    // Render player hitbox (debug)
    if (player) {
        render_player_hitbox(player);
    }
    
    // Render crosshair when in game mode
    if (is_game_active() && g_game_state.mouseCaptured) {
        render_crosshair();
    }
    
    end_frame(g_game_state.window.hdc);
}

// Render crosshair in center of screen
void render_crosshair() {
    RECT rect;
    GetClientRect(g_game_state.window.hwnd, &rect);
    int centerX = (rect.left + rect.right) / 2;
    int centerY = (rect.top + rect.bottom) / 2;
    
    // Switch to 2D rendering mode
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, rect.right - rect.left, rect.bottom - rect.top, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable depth testing for 2D
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    // Draw crosshair (white lines)
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
    // Horizontal line
    glVertex2f(centerX - 10, centerY);
    glVertex2f(centerX + 10, centerY);
    // Vertical line
    glVertex2f(centerX, centerY - 10);
    glVertex2f(centerX, centerY + 10);
    glEnd();
    
    // Restore 3D rendering mode
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Global access
GameState* get_game_state() {
    return &g_game_state;
}
