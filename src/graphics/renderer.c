#include "world/chunk_system.h"  // Include first to avoid circular dependency
#include "graphics/renderer.h"
#include "graphics/window.h"
#include "graphics/effects/Volumetrics.h"
#include "graphics/effects/Shadow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <windows.h>
#include <psapi.h>

// Global renderer state
static OpenGLContext g_renderer_context = {0};
static RenderCamera g_render_camera = {0};
static Player* g_player = NULL;
static RenderLight g_render_lights[4] = {0};
static int g_render_light_count = 2;
static RenderFog g_render_fog = {0};

// Volumetric effects
static VolumetricSystem* g_volumetric_system = NULL;
static AdvancedShadowSystem* g_shadow_system = NULL;

// Performance monitoring
static int g_frame_count = 0;
static float g_last_fps_time = 0.0f;
static float g_current_fps = 0.0f;
static float g_delta_time = 0.0f;

// Initialize renderer
BOOL initialize_renderer(HDC hdc, int width, int height) {
    g_renderer_context.hdc = hdc;
    g_renderer_context.width = width;
    g_renderer_context.height = height;
    
    // Set pixel format
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (!pixelFormat) return FALSE;
    
    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) return FALSE;
    
    // Create OpenGL context
    g_renderer_context.hrc = wglCreateContext(hdc);
    if (!g_renderer_context.hrc) return FALSE;
    
    if (!wglMakeCurrent(hdc, g_renderer_context.hrc)) return FALSE;
    
    // Set viewport
    glViewport(0, 0, width, height);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Initialize player - MINECRAFT STYLE starting position
    g_player = create_player(render_vect3_create(0, 0, 10));
    
    // Initialize camera - MINECRAFT STYLE starting position
    g_render_camera = create_render_camera(render_vect3_create(0, 0, 10), 0, 0, 60.0f);
    
    // Initialize lights - realistic sun lighting
    g_render_lights[0] = (RenderLight){
        render_vect3_create(0, 0, 100),  // position - above the scene
        render_vect3_create(0.2f, -0.8f, -0.6f),   // direction - realistic sun angle
        (Color){255, 248, 220},           // color - warm sunlight
        1.2f,                             // intensity - strong sun
        0,                                // range (directional)
        0                                 // type (directional)
    };
    
    g_render_lights[1] = (RenderLight){
        render_vect3_create(0, 0, 30),   // position - ambient light
        render_vect3_create(0, 0, -1),   // direction
        (Color){135, 206, 235},           // color - sky blue ambient
        0.3f,                             // intensity - soft ambient
        100.0f,                           // range
        1                                 // type (point)
    };
    
    // Initialize fog - realistic atmospheric fog
    g_render_fog = (RenderFog){
        TRUE,  // enabled - enable for realistic atmosphere
        render_vect3_create(0, 0, 0),  // center
        50.0f,  // radius
        (Color){100, 100, 120},  // color
        0.1f,  // density - reduced
        0.05f   // falloff - reduced
    };
    
    // Initialize volumetric effects
    g_volumetric_system = InitVolumetrics(width, height);
    g_shadow_system = InitShadow(width, height);
    
    if (g_volumetric_system) {
        printf("Volumetric fog system initialized\n");
    }
    if (g_shadow_system) {
        printf("Shadow system initialized\n");
    }
    
    printf("Renderer inicializado: %dx%d\n", width, height);
    
    return TRUE;
}

// Cleanup renderer
void cleanup_renderer(OpenGLContext* context) {
    if (context && context->hrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(context->hrc);
        printf("Renderer limpiado\n");
    }
}

// Resize renderer
void resize_renderer(int width, int height) {
    g_renderer_context.width = width;
    g_renderer_context.height = height;
    glViewport(0, 0, width, height);
}

// Create player with hitbox and physics
Player* create_player(Vect3 position) {
    Player* player = (Player*)malloc(sizeof(Player));
    if (!player) return NULL;
    
    player->position = position;
    player->velocity = render_vect3_create(0, 0, 0);
    player->target_velocity = render_vect3_create(0, 0, 0);
    
    // Player dimensions: 0.6m (un poco menos que 1 bloque para caer entre espacios)
    player->width = 0.6f;      // Ancho del hitbox (0.6 < 1.0 bloque)
    player->height = 0.6f;     // Altura del hitbox (0.6 < 1.0 bloque)
    player->depth = 1.8f;      // Profundidad del hitbox (altura del jugador)
    
    // Movement and physics
    player->move_speed = 5.0f;     // Velocidad base
    player->damping = 0.85f;       // Damping suave
    player->acceleration = 8.0f;   // Aceleración rápida
    player->gravity = -20.0f;      // Fuerza de gravedad
    player->isGrounded = FALSE;    // No está en el suelo inicialmente
    player->jump_force = 8.0f;     // Fuerza de salto
    
    // Camera control integrated into player
    player->camera_id = 1;         // ID único para referencia de mouse
    player->camera_yaw = 0.0f;     // Rotación horizontal inicial
    player->camera_pitch = 0.0f;   // Rotación vertical inicial
    player->camera_sensitivity = 1.0f; // Sensibilidad del mouse (estándar Minecraft)
    
    // Flight system
    player->isFlying = FALSE;      // No está volando inicialmente
    player->flight_speed = 8.0f;   // Velocidad de vuelo
    player->lastSpacePress = 0;    // Timestamp inicial
    player->spacePressCount = 0;   // Contador inicial
    
    // Variable jump system
    player->isSpacePressed = FALSE;    // No está presionando espacio
    player->spacePressStart = 0;       // Timestamp inicial
    player->maxJumpForce = 12.0f;      // Fuerza máxima de salto
    player->minJumpForce = 4.0f;       // Fuerza mínima de salto
    player->jumpChargeTime = 0.5f;     // 500ms para cargar salto máximo
    player->canJump = TRUE;            // Puede saltar inicialmente
    player->lastJumpTime = 0;          // Timestamp inicial
    player->jumpCooldown = 100;        // 100ms cooldown entre saltos
    
    // Block interaction system
    player->blockReach = 6.0f;         // Maximum reach distance (6 blocks)
    player->lastBlockAction = 0;       // Timestamp inicial
    player->blockActionCooldown = 200; // 200ms cooldown between block actions
    player->currentBlockType = VOXEL_GRASS; // Default block type for placing
    
    printf("Player creado: posición (%.2f, %.2f, %.2f), hitbox %.2fx%.2fx%.2f\n", 
           position.x, position.y, position.z, player->width, player->height, player->depth);
    
    return player;
}

// Destroy player
void destroy_player(Player* player) {
    if (player) {
        free(player);
        printf("Player destruido\n");
    }
}

// Player camera control functions
void player_update_camera_from_mouse(Player* player, int deltaX, int deltaY) {
    if (!player) {
        printf("ERROR: player_update_camera_from_mouse called with NULL player\n");
        return;
    }
    
    printf("player_update_camera_from_mouse: deltaX=%d, deltaY=%d, sensitivity=%.3f\n", 
           deltaX, deltaY, player->camera_sensitivity);
    
    // Update yaw (horizontal rotation) - infinite rotation
    // Minecraft-style sensitivity: 0.002f is the standard sensitivity
    float yawDelta = deltaX * player->camera_sensitivity * 0.002f;
    float pitchDelta = deltaY * player->camera_sensitivity * 0.002f;
    
    printf("Calculated deltas: yawDelta=%.6f, pitchDelta=%.6f\n", yawDelta, pitchDelta);
    
    player->camera_yaw += yawDelta;
    
    // Update pitch (vertical rotation) with limits
    player->camera_pitch -= pitchDelta;  // Invert for natural movement
    
    // Clamp pitch to ±89 degrees (1.5533 radians) to prevent gimbal lock
    const float maxPitch = 1.5533f;  // ~89 degrees
    if (player->camera_pitch > maxPitch) player->camera_pitch = maxPitch;
    if (player->camera_pitch < -maxPitch) player->camera_pitch = -maxPitch;
    
    // Update the render camera to match player camera
    RenderCamera* camera = get_render_camera();
    if (camera) {
        printf("Updating render camera: yaw=%.3f, pitch=%.3f\n", player->camera_yaw, player->camera_pitch);
        camera->yaw = player->camera_yaw;
        camera->pitch = player->camera_pitch;
        update_camera_vectors(camera);
        printf("Render camera updated: forward=(%.3f,%.3f,%.3f)\n", camera->forward.x, camera->forward.y, camera->forward.z);
    } else {
        printf("ERROR: Render camera is NULL!\n");
    }
}

void player_toggle_flight(Player* player) {
    if (!player) return;
    
    player->isFlying = !player->isFlying;
    
    if (player->isFlying) {
        printf("Player started flying\n");
        // When starting to fly, set vertical velocity to 0
        player->velocity.z = 0.0f;
    } else {
        printf("Player stopped flying\n");
        // When stopping flight, apply gravity immediately
        player->velocity.z = 0.0f;
    }
}

void player_handle_space_input(Player* player) {
    if (!player) return;
    
    DWORD currentTime = GetTickCount();
    
    // Check if this is a double press within 200ms for flight toggle
    if (currentTime - player->lastSpacePress < 200) {
        player->spacePressCount++;
        if (player->spacePressCount >= 2) {
            // Double press detected - toggle flight
            player_toggle_flight(player);
            player->spacePressCount = 0; // Reset counter
            player->isSpacePressed = FALSE; // Reset space state
            return;
        }
    } else {
        // Single press or too much time passed
        if (player->spacePressCount == 1) {
            // Single press - start variable jump if not flying and grounded
            if (!player->isFlying && player->isGrounded && player->canJump) {
                player->isSpacePressed = TRUE;
                player->spacePressStart = currentTime;
                printf("Jump charging started\n");
            }
        }
        player->spacePressCount = 1; // Start new sequence
    }
    
    player->lastSpacePress = currentTime;
}

void player_handle_space_release(Player* player) {
    if (!player || !player->isSpacePressed) return;
    
    DWORD currentTime = GetTickCount();
    DWORD pressDuration = currentTime - player->spacePressStart;
    
    // Calculate jump force based on press duration
    float chargeRatio = (float)pressDuration / (player->jumpChargeTime * 1000.0f);
    if (chargeRatio > 1.0f) chargeRatio = 1.0f;
    
    float jumpForce = player->minJumpForce + (player->maxJumpForce - player->minJumpForce) * chargeRatio;
    
    // Execute jump
    if (player->isGrounded && player->canJump) {
        player->velocity.z = jumpForce;
        player->isGrounded = FALSE;
        player->canJump = FALSE;
        player->lastJumpTime = currentTime;
        
        printf("Jump executed: force=%.2f, duration=%lums, ratio=%.2f\n", 
               jumpForce, pressDuration, chargeRatio);
    }
    
    player->isSpacePressed = FALSE;
}

void player_update_jump_system(Player* player, float deltaTime) {
    if (!player) return;
    (void)deltaTime; // Suppress unused parameter warning
    
    DWORD currentTime = GetTickCount();
    
    // Update canJump based on ground state and cooldown
    if (player->isGrounded && (currentTime - player->lastJumpTime) > player->jumpCooldown) {
        player->canJump = TRUE;
    }
    
    // Auto-release jump if held too long
    if (player->isSpacePressed) {
        DWORD pressDuration = currentTime - player->spacePressStart;
        if (pressDuration > (player->jumpChargeTime * 1000.0f)) {
            player_handle_space_release(player);
        }
    }
}

// Create render camera
RenderCamera create_render_camera(Vect3 position, float yaw, float pitch, float fov) {
    RenderCamera camera = {0};
    camera.position = position;
    camera.yaw = yaw;
    camera.pitch = pitch;
    camera.fov = fov;
    camera.nearPlane = 0.1f;
    camera.farPlane = 1000.0f;
    camera.up = render_vect3_create(0, 0, 1);
    camera.eye_height = 1.8f;  // Altura de los ojos desde el suelo del jugador
    
    update_camera_vectors(&camera);
    return camera;
}

// Update camera vectors (MINECRAFT STYLE)
void update_camera_vectors(RenderCamera* camera) {
    // MINECRAFT CAMERA SYSTEM:
    // - Yaw: rotación horizontal (izquierda/derecha)
    // - Pitch: rotación vertical (arriba/abajo)
    // - Z es UP (altura), Y es FORWARD (profundidad), X es RIGHT (lateral)
    
    // Forward vector (hacia donde mira la cámara) - CORREGIDO para mirar hacia Y positivo
    camera->forward = render_vect3_create(
        sinf(camera->yaw) * cosf(camera->pitch),    // X: lateral
        cosf(camera->yaw) * cosf(camera->pitch),    // Y: profundidad (hacia adelante)
        sinf(camera->pitch)                         // Z: altura
    );
    camera->forward = render_vect3_normalize(camera->forward);
    
    // Right vector (perpendicular a forward, en el plano horizontal)
    camera->right = render_vect3_create(
        cosf(camera->yaw),    // X: lateral
        -sinf(camera->yaw),   // Y: lateral
        0.0f                  // Z: sin componente vertical
    );
    camera->right = render_vect3_normalize(camera->right);
    
    // Up vector (siempre apunta hacia arriba en Z)
    camera->up = render_vect3_create(0, 0, 1);
    
    // Target (punto hacia donde mira la cámara)
    camera->target = render_vect3_add(camera->position, camera->forward);
}

// Camera movement functions (MINECRAFT STYLE)
void camera_rotate(RenderCamera* camera, float deltaYaw, float deltaPitch) {
    // MINECRAFT MOUSE SENSITIVITY - Restaurada a valor original
    float sensitivity = 0.002f;  // Sensibilidad estándar de Minecraft
    
    // MINECRAFT CAMERA MECHANICS:
    // - deltaX (horizontal) = yaw rotation (izquierda/derecha)
    // - deltaY (vertical) = pitch rotation (arriba/abajo)
    // - Eje vertical invertido para comportamiento natural
    
    camera->yaw += deltaYaw * sensitivity;
    camera->pitch -= deltaPitch * sensitivity;  // Invertir para comportamiento natural
    
    // Limit pitch (no puede mirar más allá de arriba/abajo)
    // Permitir mirar hacia arriba (hasta 90 grados) y hacia abajo (hasta 90 grados)
    if (camera->pitch > 1.57f) camera->pitch = 1.57f;   // 90 grados arriba (π/2)
    if (camera->pitch < -1.57f) camera->pitch = -1.57f; // 90 grados abajo (-π/2)
    
    // INFINITE YAW ROTATION - Allow unlimited horizontal rotation
    // No need to normalize yaw - let it accumulate for infinite rotation
    // This allows the camera to rotate 360° continuously in the horizontal plane
    
    update_camera_vectors(camera);
}

// Camera update from mouse input (Raw Input)
void Camera_UpdateFromMouse(RenderCamera* camera, int deltaX, int deltaY, float sensitivity) {
    if (!camera) return;
    
    // Default sensitivity if not provided - increased for better responsiveness
    if (sensitivity <= 0.0f) {
        sensitivity = 0.002f;  // Increased sensitivity for better camera control
    }
    
    // Convert pixel deltas to radians with proper scaling
    // Raw Input gives pixel deltas, convert to degrees then to radians
    float yawDelta = deltaX * sensitivity;
    float pitchDelta = deltaY * sensitivity;
    
    // Update yaw (horizontal rotation) - infinite rotation
    camera->yaw += yawDelta;
    
    // Update pitch (vertical rotation) with limits
    camera->pitch -= pitchDelta;  // Invert for natural movement
    
    // Clamp pitch to ±89 degrees (1.5533 radians) to prevent gimbal lock
    const float maxPitch = 1.5533f;  // ~89 degrees
    if (camera->pitch > maxPitch) camera->pitch = maxPitch;
    if (camera->pitch < -maxPitch) camera->pitch = -maxPitch;
    
    // Update camera vectors
    update_camera_vectors(camera);
}

// MINECRAFT MOVEMENT: W/S = adelante/atrás, A/D = izquierda/derecha
void camera_move_forward(RenderCamera* camera, float distance) {
    // Movimiento hacia donde mira la cámara (incluyendo componente vertical)
    Vect3 movement = render_vect3_scale(camera->forward, distance);
    camera->position = render_vect3_add(camera->position, movement);
    camera->target = render_vect3_add(camera->position, camera->forward);
}

void camera_move_right(RenderCamera* camera, float distance) {
    // Movimiento lateral perpendicular a donde mira la cámara
    Vect3 movement = render_vect3_scale(camera->right, distance);
    camera->position = render_vect3_add(camera->position, movement);
    camera->target = render_vect3_add(camera->position, camera->forward);
}

// MINECRAFT MOVEMENT: Q/E = subir/bajar
void camera_move_up(RenderCamera* camera, float distance) {
    // Movimiento vertical directo en Z
    Vect3 movement = render_vect3_create(0, 0, distance);
    camera->position = render_vect3_add(camera->position, movement);
    camera->target = render_vect3_add(camera->position, camera->forward);
}

// Legacy camera movement functions (deprecated - use player instead)
void camera_set_movement_input(RenderCamera* camera, float forward, float right, float up) {
    (void)camera; // Suppress unused parameter warning
    (void)forward; // Suppress unused parameter warning
    (void)right; // Suppress unused parameter warning
    (void)up; // Suppress unused parameter warning
    // This function is deprecated - movement is now handled by the player
    // Keeping for compatibility but it does nothing
}

void camera_update_smooth_movement(RenderCamera* camera, float deltaTime) {
    (void)camera; // Suppress unused parameter warning
    (void)deltaTime; // Suppress unused parameter warning
    // This function is deprecated - movement is now handled by the player
    // Keeping for compatibility but it does nothing
}

void camera_apply_damping(RenderCamera* camera, float deltaTime) {
    (void)camera; // Suppress unused parameter warning
    (void)deltaTime; // Suppress unused parameter warning
    // This function is deprecated - movement is now handled by the player
    // Keeping for compatibility but it does nothing
}

// Begin frame
void begin_frame() {
    // Clear buffers with improved sky color
    glClearColor(0.2f, 0.4f, 0.8f, 1.0f); // Enhanced sky blue
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect = (float)g_renderer_context.width / (float)g_renderer_context.height;
    gluPerspective(g_render_camera.fov, aspect, g_render_camera.nearPlane, g_render_camera.farPlane);
    
    // Set up modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Set up camera
    gluLookAt(
        g_render_camera.position.x, g_render_camera.position.y, g_render_camera.position.z,
        g_render_camera.target.x, g_render_camera.target.y, g_render_camera.target.z,
        g_render_camera.up.x, g_render_camera.up.y, g_render_camera.up.z
    );
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    
    // Set up lights
    for (int i = 0; i < g_render_light_count; i++) {
        GLenum light = (i == 0) ? GL_LIGHT0 : GL_LIGHT1;
        
        float lightPos[4];
        if (g_render_lights[i].type == 0) { // Directional
            lightPos[0] = -g_render_lights[i].direction.x;
            lightPos[1] = -g_render_lights[i].direction.y;
            lightPos[2] = -g_render_lights[i].direction.z;
            lightPos[3] = 0.0f;
        } else { // Point
            lightPos[0] = g_render_lights[i].position.x;
            lightPos[1] = g_render_lights[i].position.y;
            lightPos[2] = g_render_lights[i].position.z;
            lightPos[3] = 1.0f;
        }
        
        float lightColor[4] = {
            g_render_lights[i].color.r / 255.0f * g_render_lights[i].intensity,
            g_render_lights[i].color.g / 255.0f * g_render_lights[i].intensity,
            g_render_lights[i].color.b / 255.0f * g_render_lights[i].intensity,
            1.0f
        };
        
        glLightfv(light, GL_POSITION, lightPos);
        glLightfv(light, GL_DIFFUSE, lightColor);
        glLightfv(light, GL_SPECULAR, lightColor);
    }
    
    // Update volumetric effects with current light direction
    if (g_volumetric_system && g_render_light_count > 0) {
        Vect3 sunDir = g_render_lights[0].direction;
        Color sunColor = g_render_lights[0].color;
        float sunIntensity = g_render_lights[0].intensity;
        UpdateVolumetricParameters(g_volumetric_system, sunDir, sunColor, sunIntensity);
    }
    
    // Render shadow pass
    if (g_shadow_system && g_render_light_count > 0) {
        Vect3 lightDir = g_render_lights[0].direction;
        Vect3 sceneCenter = render_vect3_create(0, 0, 0);
        float sceneRadius = 100.0f;
        RenderShadowPass(g_shadow_system, lightDir, sceneCenter, sceneRadius);
    }
}

// End frame
void end_frame(HDC hdc) {
    // Render volumetric fog pass
    if (g_volumetric_system) {
        Vect3 cameraPos = g_render_camera.position;
        Vect3 cameraDir = g_render_camera.forward;
        float fov = g_render_camera.fov;
        RenderVolumetricsPass(g_volumetric_system, g_shadow_system, cameraPos, cameraDir, fov);
    }
    
    // Swap buffers
    SwapBuffers(hdc);
}

// Render scene
void render_scene(RenderCamera camera, RenderLight* lights, int lightCount, RenderFog fog) {
    // Suppress unused parameter warnings
    (void)lights;
    (void)lightCount;
    (void)fog;
    
    // Update FPS counter
    update_fps_counter();
    
    // Update fog center to follow camera
    fog.center = camera.position;
    
    // ESPACIO DE PRUEBAS - Renderizar chunks visibles
    render_test_environment();
}

// Render test environment with visible chunks
void render_test_environment() {
    
    // Set up lighting for the scene - reduced intensity
    float light0_pos[] = {0.0f, 0.0f, 100.0f, 0.0f};  // Directional light from above
    float light0_diff[] = {0.8f, 0.8f, 0.8f, 1.0f};   // Increased white light
    float light0_amb[] = {0.3f, 0.3f, 0.3f, 1.0f};    // Increased ambient light
    
    float light1_pos[] = {0.0f, 0.0f, 30.0f, 1.0f};   // Point light
    float light1_diff[] = {0.4f, 0.4f, 0.5f, 1.0f};   // Increased blue light
    float light1_amb[] = {0.1f, 0.1f, 0.1f, 1.0f};    // Increased ambient
    
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_amb);
    
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diff);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_amb);
    
    // Test cube removed - only render procedural chunks
    
    // Render chunks with procedural colors
    GameState* gameState = get_game_state();
    if (gameState && gameState->chunkManager) {
        ChunkManager* manager = gameState->chunkManager;
        
        for (int i = 0; i < manager->maxChunks; i++) {
            if (manager->chunks[i] && manager->chunks[i]->isGenerated) {
                VoxelChunk* chunk = manager->chunks[i];
                
                // Render each block in the chunk - TODAS LAS CAPAS (incluyendo árboles)
                for (int x = 0; x < 16; x++) {
                    for (int y = 0; y < 16; y++) {
                        for (int z = 0; z < 16; z++) {
                            VoxelBlock* block = &chunk->blocks[x][y][z];
                            
                            if (block->type != VOXEL_AIR && block->isVisible) {
                                // Calculate world position
                                float worldX = chunk->chunkX * 16 + x;
                                float worldY = chunk->chunkY * 16 + y;
                                float worldZ = (float)z; // Renderizar en la posición correcta del bloque (z=0 para el suelo)
                                
                                glPushMatrix();
                                glTranslatef(worldX, worldY, worldZ);
                                
                                // Use the block's random color
                                glColor3f(block->color.r / 255.0f, 
                                         block->color.g / 255.0f, 
                                         block->color.b / 255.0f);
                                
                                // Render block as a cube
                                glBegin(GL_QUADS);
                                
                                // Top face
                                glNormal3f(0, 0, 1);
                                glVertex3f(-0.5f, -0.5f, 0.5f);
                                glVertex3f(0.5f, -0.5f, 0.5f);
                                glVertex3f(0.5f, 0.5f, 0.5f);
                                glVertex3f(-0.5f, 0.5f, 0.5f);
                                
                                // Bottom face
                                glNormal3f(0, 0, -1);
                                glVertex3f(-0.5f, -0.5f, -0.5f);
                                glVertex3f(-0.5f, 0.5f, -0.5f);
                                glVertex3f(0.5f, 0.5f, -0.5f);
                                glVertex3f(0.5f, -0.5f, -0.5f);
                                
                                // Front face
                                glNormal3f(0, 1, 0);
                                glVertex3f(-0.5f, 0.5f, -0.5f);
                                glVertex3f(-0.5f, 0.5f, 0.5f);
                                glVertex3f(0.5f, 0.5f, 0.5f);
                                glVertex3f(0.5f, 0.5f, -0.5f);
                                
                                // Back face
                                glNormal3f(0, -1, 0);
                                glVertex3f(-0.5f, -0.5f, -0.5f);
                                glVertex3f(0.5f, -0.5f, -0.5f);
                                glVertex3f(0.5f, -0.5f, 0.5f);
                                glVertex3f(-0.5f, -0.5f, 0.5f);
                                
                                // Right face
                                glNormal3f(1, 0, 0);
                                glVertex3f(0.5f, -0.5f, -0.5f);
                                glVertex3f(0.5f, 0.5f, -0.5f);
                                glVertex3f(0.5f, 0.5f, 0.5f);
                                glVertex3f(0.5f, -0.5f, 0.5f);
                                
                                // Left face
                                glNormal3f(-1, 0, 0);
                                glVertex3f(-0.5f, -0.5f, -0.5f);
                                glVertex3f(-0.5f, -0.5f, 0.5f);
                                glVertex3f(-0.5f, 0.5f, 0.5f);
                                glVertex3f(-0.5f, 0.5f, -0.5f);
                                
                                glEnd();
                                glPopMatrix();
                            }
                        }
                    }
                }
            }
        }
    }
    
            // Render player hitbox (transparent cube)
            if (g_player) {
                render_player_hitbox(g_player);
            }
            
            // Render performance info
            render_performance_info();
            
            // printf("Entorno de prueba renderizado.\n"); // Comentado para evitar spam
}

// Vector utility functions
Vect3 render_vect3_create(float x, float y, float z) {
    Vect3 v = {x, y, z};
    return v;
}

Vect3 render_vect3_add(Vect3 a, Vect3 b) {
    return render_vect3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vect3 render_vect3_subtract(Vect3 a, Vect3 b) {
    return render_vect3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vect3 render_vect3_scale(Vect3 v, float scalar) {
    return render_vect3_create(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vect3 render_vect3_normalize(Vect3 v) {
    float mag = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (mag == 0) return render_vect3_create(0, 0, 0);
    return render_vect3_create(v.x / mag, v.y / mag, v.z / mag);
}

float render_vect3_dot(Vect3 a, Vect3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vect3 render_vect3_cross(Vect3 a, Vect3 b) {
    return render_vect3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

// Player movement and physics functions
void player_set_movement_input(Player* player, float forward, float right, float up) {
    if (!player) return;
    
    // Calculate movement based on camera orientation
    RenderCamera* camera = get_render_camera();
    if (!camera) return;
    
    Vect3 forward_dir = camera->forward;
    Vect3 right_dir = camera->right;
    Vect3 up_dir = render_vect3_create(0, 0, 1);
    
    // Calculate target velocity
    Vect3 target_vel = render_vect3_create(0, 0, 0);
    
    // Forward/backward movement
    if (forward != 0.0f) {
        Vect3 forward_vel = render_vect3_scale(forward_dir, forward * player->move_speed);
        target_vel = render_vect3_add(target_vel, forward_vel);
    }
    
    // Left/right movement
    if (right != 0.0f) {
        Vect3 right_vel = render_vect3_scale(right_dir, right * player->move_speed);
        target_vel = render_vect3_add(target_vel, right_vel);
    }
    
    // Vertical movement (only for flight mode)
    if (up != 0.0f) {
        if (player->isFlying) {
            // In flight mode, add vertical velocity
            Vect3 up_vel = render_vect3_scale(up_dir, up * player->flight_speed);
            target_vel = render_vect3_add(target_vel, up_vel);
        }
        // Note: Normal jumping is now handled by the variable jump system
    }
    
    // Set target velocity (all components for flight, horizontal only for normal mode)
    if (player->isFlying) {
        player->target_velocity = target_vel;
    } else {
        player->target_velocity.x = target_vel.x;
        player->target_velocity.y = target_vel.y;
        // Z component is handled by gravity/jump
    }
}

// Helper function to check if a block is solid
static BOOL is_block_solid_at(ChunkManager* manager, int worldX, int worldY, int worldZ) {
    if (!manager) return FALSE;
    
    // Calculate chunk coordinates
    int chunkX = worldX / 16;
    int chunkY = worldY / 16;
    int chunkZ = worldZ / 16;
    
    // Calculate local block coordinates within chunk
    int localX = worldX % 16;
    int localY = worldY % 16;
    int localZ = worldZ % 16;
    
    // Handle negative coordinates correctly
    if (localX < 0) { localX += 16; chunkX--; }
    if (localY < 0) { localY += 16; chunkY--; }
    if (localZ < 0) { localZ += 16; chunkZ--; }
    
    // Find the chunk
    for (int i = 0; i < manager->loadedChunks; i++) {
        VoxelChunk* chunk = manager->chunks[i];
        if (chunk && chunk->chunkX == chunkX && chunk->chunkY == chunkY && chunk->chunkZ == chunkZ) {
            VoxelBlock* block = &chunk->blocks[localX][localY][localZ];
            
            // Check if block is solid using blueprints
            BlockBlueprint* blueprints = create_block_blueprints();
            if (blueprints) {
                BlockBlueprint* blueprint = get_block_blueprint(blueprints, block->type);
                if (blueprint) {
                    return blueprint->isSolid;
                }
            }
        }
    }
    
    return FALSE; // No chunk found or block is air
}

// Helper function to check collision with blocks in a region
static BOOL check_collision_with_blocks(ChunkManager* manager, Vect3 position, float width, float height, float depth) {
    if (!manager) return FALSE;
    
    // Check blocks in a box around the player
    int minX = (int)floorf(position.x - width / 2.0f);
    int maxX = (int)ceilf(position.x + width / 2.0f);
    int minY = (int)floorf(position.y - height / 2.0f);
    int maxY = (int)ceilf(position.y + height / 2.0f);
    int minZ = (int)floorf(position.z);
    int maxZ = (int)ceilf(position.z + depth);
    
    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            for (int z = minZ; z <= maxZ; z++) {
                if (is_block_solid_at(manager, x, y, z)) {
                    return TRUE; // Collision detected
                }
            }
        }
    }
    
    return FALSE; // No collision
}

void player_update_physics(Player* player, float deltaTime) {
    if (!player) return;
    
    // Get chunk manager for collision detection
    extern GameState* get_game_state();
    GameState* gameState = get_game_state();
    ChunkManager* manager = gameState ? gameState->chunkManager : NULL;
    
    // Apply gravity only if not flying
    if (!player->isFlying && !player->isGrounded) {
        player->velocity.z += player->gravity * deltaTime;
    }
    
    // Handle flight mode
    if (player->isFlying) {
        // In flight mode, interpolate all velocity components towards target
        Vect3 velocity_diff = render_vect3_subtract(player->target_velocity, player->velocity);
        Vect3 acceleration = render_vect3_scale(velocity_diff, player->acceleration * deltaTime);
        
        player->velocity.x += acceleration.x;
        player->velocity.y += acceleration.y;
        player->velocity.z += acceleration.z;
        
        // Apply damping when no input
        if (render_vect3_dot(player->target_velocity, player->target_velocity) < 0.01f) {
            player->velocity.x *= player->damping;
            player->velocity.y *= player->damping;
            player->velocity.z *= player->damping;
        }
    } else {
        // Normal mode - only horizontal movement
        Vect3 horizontal_velocity = render_vect3_create(player->velocity.x, player->velocity.y, 0);
        Vect3 horizontal_target = render_vect3_create(player->target_velocity.x, player->target_velocity.y, 0);
        Vect3 velocity_diff = render_vect3_subtract(horizontal_target, horizontal_velocity);
        Vect3 acceleration = render_vect3_scale(velocity_diff, player->acceleration * deltaTime);
        
        player->velocity.x += acceleration.x;
        player->velocity.y += acceleration.y;
        
        // Apply damping when no input
        if (render_vect3_dot(horizontal_target, horizontal_target) < 0.01f) {
            player->velocity.x *= player->damping;
            player->velocity.y *= player->damping;
        }
    }
    
    // Apply movement with collision detection on each axis separately
    // X axis
    Vect3 newPosX = player->position;
    newPosX.x += player->velocity.x * deltaTime;
    if (!check_collision_with_blocks(manager, newPosX, player->width, player->height, player->depth)) {
        player->position.x = newPosX.x;
    } else {
        player->velocity.x = 0; // Stop X velocity on collision
    }
    
    // Y axis
    Vect3 newPosY = player->position;
    newPosY.y += player->velocity.y * deltaTime;
    if (!check_collision_with_blocks(manager, newPosY, player->width, player->height, player->depth)) {
        player->position.y = newPosY.y;
    } else {
        player->velocity.y = 0; // Stop Y velocity on collision
    }
    
    // Z axis
    Vect3 newPosZ = player->position;
    newPosZ.z += player->velocity.z * deltaTime;
    if (!check_collision_with_blocks(manager, newPosZ, player->width, player->height, player->depth)) {
        player->position.z = newPosZ.z;
    } else {
        if (player->velocity.z < 0) {
            // Landing on ground or block
            player->velocity.z = 0;
            player->isGrounded = TRUE;
        } else {
            // Hit ceiling
            player->velocity.z = 0;
        }
    }
    
    // Simple ground collision (z = 0) - only when not flying
    if (!player->isFlying && player->position.z <= 0) {
        player->position.z = 0;
        player->velocity.z = 0;
        player->isGrounded = TRUE;
    } else if (player->isFlying) {
        player->isGrounded = FALSE; // Flying players are never grounded
    } else if (player->velocity.z < -0.1f) {
        // Check if grounded by trying to move down a bit
        Vect3 testPos = player->position;
        testPos.z -= 0.1f;
        if (check_collision_with_blocks(manager, testPos, player->width, player->height, player->depth)) {
            player->isGrounded = TRUE;
        } else {
            player->isGrounded = FALSE;
        }
    }
}

void player_jump(Player* player) {
    if (!player || !player->isGrounded) return;
    
    player->velocity.z = player->jump_force;
    player->isGrounded = FALSE;
}

// Raycast from camera to select blocks (DDA algorithm - Minecraft style)
BlockSelection raycast_from_camera(RenderCamera* camera, ChunkManager* manager, float maxDistance) {
    BlockSelection selection = {0};
    selection.hit = FALSE;
    selection.distance = maxDistance;
    
    if (!camera || !manager) return selection;
    
    // Ray origin (camera position - this is the center of the crosshair)
    // The camera position is already at eye height, so this is the exact crosshair center
    Vect3 origin = camera->position;
    Vect3 direction = camera->forward;
    
    // Normalize direction for precise vector projection
    direction = render_vect3_normalize(direction);
    
    // Ensure direction is not zero vector
    if (direction.x == 0.0f && direction.y == 0.0f && direction.z == 0.0f) {
        direction = render_vect3_create(0.0f, 1.0f, 0.0f); // Default forward direction
    }
    
    // Debug: print raycast origin for verification
    static int debugCounter = 0;
    if (++debugCounter % 60 == 0) { // Print every 60 frames (1 second)
        printf("Raycast origin: (%.2f,%.2f,%.2f) direction: (%.2f,%.2f,%.2f)\n", 
               origin.x, origin.y, origin.z, direction.x, direction.y, direction.z);
    }
    
    // DDA (Digital Differential Analyzer) algorithm for voxel traversal
    // Start from the exact camera position for maximum precision
    float x = origin.x;
    float y = origin.y;
    float z = origin.z;
    
    // Current voxel position
    int voxelX = (int)floorf(x);
    int voxelY = (int)floorf(y);
    int voxelZ = (int)floorf(z);
    
    // Check if we start inside a solid block
    if (is_block_solid_at(manager, voxelX, voxelY, voxelZ)) {
        // If we start inside a block, we need to find the exit face
        // This happens when the camera is inside a block or very close to one
        printf("Raycast starts inside block at (%d,%d,%d)\n", voxelX, voxelY, voxelZ);
        
        // For now, just return the block we're inside
        selection.hit = TRUE;
        selection.blockX = voxelX;
        selection.blockY = voxelY;
        selection.blockZ = voxelZ;
        selection.faceX = 0;
        selection.faceY = 0;
        selection.faceZ = 1; // Default to top face
        selection.distance = 0.0f;
        
        // Get block type
        int chunkX = voxelX / 16;
        int chunkY = voxelY / 16;
        int chunkZ = voxelZ / 16;
        int localX = voxelX % 16; if (localX < 0) { localX += 16; chunkX--; }
        int localY = voxelY % 16; if (localY < 0) { localY += 16; chunkY--; }
        int localZ = voxelZ % 16; if (localZ < 0) { localZ += 16; chunkZ--; }
        
        for (int j = 0; j < manager->loadedChunks; j++) {
            VoxelChunk* chunk = manager->chunks[j];
            if (chunk && chunk->chunkX == chunkX && chunk->chunkY == chunkY && chunk->chunkZ == chunkZ) {
                selection.blockType = chunk->blocks[localX][localY][localZ].type;
                break;
            }
        }
        
        return selection;
    }
    
    // Direction signs
    int stepX = direction.x > 0 ? 1 : -1;
    int stepY = direction.y > 0 ? 1 : -1;
    int stepZ = direction.z > 0 ? 1 : -1;
    
    // Calculate initial t-values for next voxel boundaries
    float tMaxX = (direction.x != 0) ? (floorf(x + (stepX > 0 ? 1 : 0)) - x) / direction.x : INFINITY;
    float tMaxY = (direction.y != 0) ? (floorf(y + (stepY > 0 ? 1 : 0)) - y) / direction.y : INFINITY;
    float tMaxZ = (direction.z != 0) ? (floorf(z + (stepZ > 0 ? 1 : 0)) - z) / direction.z : INFINITY;
    
    // Delta t values (how much t increases per step)
    float tDeltaX = (direction.x != 0) ? stepX / direction.x : INFINITY;
    float tDeltaY = (direction.y != 0) ? stepY / direction.y : INFINITY;
    float tDeltaZ = (direction.z != 0) ? stepZ / direction.z : INFINITY;
    
    // Last face normal (for placing blocks)
    int faceX = 0, faceY = 0, faceZ = 0;
    
    // Traverse ray through voxels with precise intersection detection
    float t = 0;
    for (int i = 0; i < 200 && t < maxDistance; i++) {
        // Check if current voxel is solid
        if (is_block_solid_at(manager, voxelX, voxelY, voxelZ)) {
            selection.hit = TRUE;
            selection.blockX = voxelX;
            selection.blockY = voxelY;
            selection.blockZ = voxelZ;
            selection.faceX = faceX;
            selection.faceY = faceY;
            selection.faceZ = faceZ;
            selection.distance = t;
            
            // Debug: print hit info (only occasionally to reduce spam)
            static int hitCounter = 0;
            if (++hitCounter % 10 == 0) { // Print every 10th hit
                printf("Raycast HIT: block=(%d,%d,%d), face=(%d,%d,%d), distance=%.2f\n", 
                       voxelX, voxelY, voxelZ, faceX, faceY, faceZ, t);
            }
            
            // IMPORTANT: Stop immediately at first solid block hit
            // Don't continue traversing through blocks
            
            // Get block type
            int chunkX = voxelX / 16;
            int chunkY = voxelY / 16;
            int chunkZ = voxelZ / 16;
            int localX = voxelX % 16; if (localX < 0) { localX += 16; chunkX--; }
            int localY = voxelY % 16; if (localY < 0) { localY += 16; chunkY--; }
            int localZ = voxelZ % 16; if (localZ < 0) { localZ += 16; chunkZ--; }
            
            for (int j = 0; j < manager->loadedChunks; j++) {
                VoxelChunk* chunk = manager->chunks[j];
                if (chunk && chunk->chunkX == chunkX && chunk->chunkY == chunkY && chunk->chunkZ == chunkZ) {
                    selection.blockType = chunk->blocks[localX][localY][localZ].type;
                    break;
                }
            }
            
            return selection;
        }
        
        // Step to next voxel with precise face detection
        if (tMaxX < tMaxY && tMaxX < tMaxZ) {
            t = tMaxX;
            tMaxX += tDeltaX;
            voxelX += stepX;
            faceX = -stepX;
            faceY = 0;
            faceZ = 0;
        } else if (tMaxY < tMaxZ) {
            t = tMaxY;
            tMaxY += tDeltaY;
            voxelY += stepY;
            faceX = 0;
            faceY = -stepY;
            faceZ = 0;
        } else {
            t = tMaxZ;
            tMaxZ += tDeltaZ;
            voxelZ += stepZ;
            faceX = 0;
            faceY = 0;
            faceZ = -stepZ;
        }
    }
    
    return selection; // No hit
}

// Render block selection outline (wireframe)
void render_block_selection(BlockSelection* selection) {
    if (!selection || !selection->hit) return;
    
    glPushMatrix();
    // Align with block rendering coordinates (blocks are rendered at integer positions)
    glTranslatef((float)selection->blockX, (float)selection->blockY, (float)selection->blockZ);
    
    // Set color for selection outline
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f); // Black outline
    glLineWidth(3.0f);
    
    // Draw wireframe cube slightly larger than block (consistent with 1.0f block size)
    float size = 0.51f;  // Slightly larger than 0.5f block for visibility
    glBegin(GL_LINES);
    
    // Bottom face
    glVertex3f(-size, -size, -size); glVertex3f(size, -size, -size);
    glVertex3f(size, -size, -size); glVertex3f(size, size, -size);
    glVertex3f(size, size, -size); glVertex3f(-size, size, -size);
    glVertex3f(-size, size, -size); glVertex3f(-size, -size, -size);
    
    // Top face
    glVertex3f(-size, -size, size); glVertex3f(size, -size, size);
    glVertex3f(size, -size, size); glVertex3f(size, size, size);
    glVertex3f(size, size, size); glVertex3f(-size, size, size);
    glVertex3f(-size, size, size); glVertex3f(-size, -size, size);
    
    // Vertical edges
    glVertex3f(-size, -size, -size); glVertex3f(-size, -size, size);
    glVertex3f(size, -size, -size); glVertex3f(size, -size, size);
    glVertex3f(size, size, -size); glVertex3f(size, size, size);
    glVertex3f(-size, size, -size); glVertex3f(-size, size, size);
    
    glEnd();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glPopMatrix();
}

// Check if a block is surface/ground level
BOOL is_surface_block(ChunkManager* manager, int worldX, int worldY, int worldZ) {
    if (!manager) return FALSE;
    
    // Check if this is the topmost solid block at this X,Y position
    for (int z = worldZ + 1; z < 16; z++) {
        if (is_block_solid_at(manager, worldX, worldY, z)) {
            return FALSE; // There's a block above, so this isn't surface
        }
    }
    
    // Check if this block is solid and has air above it
    if (is_block_solid_at(manager, worldX, worldY, worldZ)) {
        return !is_block_solid_at(manager, worldX, worldY, worldZ + 1);
    }
    
    return FALSE;
}

// Re-render terrain below a broken surface block
void rerender_terrain_below(ChunkManager* manager, int worldX, int worldY, int worldZ) {
    if (!manager) return;
    
    printf("Re-rendering terrain below (%d,%d,%d)\n", worldX, worldY, worldZ);
    
    // Find the chunk
    int chunkX = worldX / 16;
    int chunkY = worldY / 16;
    int chunkZ = worldZ / 16;
    
    // Handle negative coordinates
    if (worldX < 0) chunkX--;
    if (worldY < 0) chunkY--;
    if (worldZ < 0) chunkZ--;
    
    for (int i = 0; i < manager->loadedChunks; i++) {
        VoxelChunk* chunk = manager->chunks[i];
        if (chunk && chunk->chunkX == chunkX && chunk->chunkY == chunkY && chunk->chunkZ == chunkZ) {
            int localX = worldX % 16; if (localX < 0) localX += 16;
            int localY = worldY % 16; if (localY < 0) localY += 16;
            int localZ = worldZ % 16; if (localZ < 0) localZ += 16;
            
            // Re-render blocks below the broken surface block
            for (int z = localZ - 1; z >= 0; z--) {
                VoxelBlock* block = &chunk->blocks[localX][localY][z];
                
                // If this block is air, we need to generate terrain below
                if (block->type == VOXEL_AIR) {
                    // Generate appropriate block type based on depth
                    if (z >= 12) {
                        // Surface layer - grass
                        block->type = VOXEL_GRASS;
                    } else if (z >= 8) {
                        // Middle layer - dirt/stone mix
                        block->type = VOXEL_WOOD; // Using wood as placeholder for dirt
                    } else {
                        // Deep layer - stone
                        block->type = VOXEL_LEAVES; // Using leaves as placeholder for stone
                    }
                    
                    // Initialize block properties
                    BlockBlueprint* blueprints = create_block_blueprints();
                    if (blueprints) {
                        BlockBlueprint* blueprint = get_block_blueprint(blueprints, block->type);
                        if (blueprint) {
                            initialize_block_from_blueprint(block, blueprint);
                        }
                    }
                    
                    // Recalculate face visibility
                    calculate_block_faces(chunk, localX, localY, z);
                }
            }
            
            // Recalculate faces for all affected blocks
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        int adjX = localX + dx;
                        int adjY = localY + dy;
                        int adjZ = localZ + dz;
                        
                        if (adjX >= 0 && adjX < 16 && adjY >= 0 && adjY < 16 && adjZ >= 0 && adjZ < 16) {
                            calculate_block_faces(chunk, adjX, adjY, adjZ);
                        }
                    }
                }
            }
            
            printf("Terrain re-rendered below surface block\n");
            return;
        }
    }
}

// Minecraft-style block interaction functions
void break_block_at(ChunkManager* manager, int worldX, int worldY, int worldZ) {
    if (!manager) return;
    
    // Calculate chunk coordinates
    int chunkX = worldX / 16;
    int chunkY = worldY / 16;
    int chunkZ = worldZ / 16;
    
    // Calculate local block coordinates within chunk
    int localX = worldX % 16;
    int localY = worldY % 16;
    int localZ = worldZ % 16;
    
    // Handle negative coordinates correctly
    if (localX < 0) { localX += 16; chunkX--; }
    if (localY < 0) { localY += 16; chunkY--; }
    if (localZ < 0) { localZ += 16; chunkZ--; }
    
    // Find the chunk
    for (int i = 0; i < manager->loadedChunks; i++) {
        VoxelChunk* chunk = manager->chunks[i];
        if (chunk && chunk->chunkX == chunkX && chunk->chunkY == chunkY && chunk->chunkZ == chunkZ) {
            VoxelBlock* block = &chunk->blocks[localX][localY][localZ];
            
            // Only break non-air blocks
            if (block->type != VOXEL_AIR) {
                printf("Breaking block at (%d,%d,%d) type=%d\n", worldX, worldY, worldZ, block->type);
                
                // Check if this is a surface block
                BOOL wasSurface = is_surface_block(manager, worldX, worldY, worldZ);
                printf("Block is surface: %s\n", wasSurface ? "YES" : "NO");
                
                // Set block to air
                block->type = VOXEL_AIR;
                block->isVisible = FALSE;
                
                // If this was a surface block, re-render terrain below
                if (wasSurface) {
                    printf("Re-rendering terrain below surface block\n");
                    rerender_terrain_below(manager, worldX, worldY, worldZ);
                } else {
                    printf("Block is not surface - no terrain re-rendering needed\n");
                }
                
                // Mark chunk for remeshing
                chunk->needsRemesh = TRUE;
                
                // Recalculate face visibility for this block and adjacent blocks
                calculate_block_faces(chunk, localX, localY, localZ);
                
                // Recalculate faces for adjacent blocks (handle chunk rebases)
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dz = -1; dz <= 1; dz++) {
                            if (dx == 0 && dy == 0 && dz == 0) continue;
                            
                            int adjX = localX + dx;
                            int adjY = localY + dy;
                            int adjZ = localZ + dz;
                            
                            // Handle chunk rebases for adjacent blocks
                            int adjChunkX = chunkX;
                            int adjChunkY = chunkY;
                            int adjChunkZ = chunkZ;
                            
                            if (adjX < 0) { adjX += 16; adjChunkX--; }
                            else if (adjX >= 16) { adjX -= 16; adjChunkX++; }
                            if (adjY < 0) { adjY += 16; adjChunkY--; }
                            else if (adjY >= 16) { adjY -= 16; adjChunkY++; }
                            if (adjZ < 0) { adjZ += 16; adjChunkZ--; }
                            else if (adjZ >= 16) { adjZ -= 16; adjChunkZ++; }
                            
                            // Find adjacent chunk and mark for remeshing
                            for (int j = 0; j < manager->loadedChunks; j++) {
                                VoxelChunk* adjChunk = manager->chunks[j];
                                if (adjChunk && adjChunk->chunkX == adjChunkX && 
                                    adjChunk->chunkY == adjChunkY && adjChunk->chunkZ == adjChunkZ) {
                                    adjChunk->needsRemesh = TRUE;
                                    calculate_block_faces(adjChunk, adjX, adjY, adjZ);
                                    break;
                                }
                            }
                            
                            if (adjX >= 0 && adjX < 16 && adjY >= 0 && adjY < 16 && adjZ >= 0 && adjZ < 16) {
                                calculate_block_faces(chunk, adjX, adjY, adjZ);
                            }
                        }
                    }
                }
                
                printf("Block broken successfully\n");
                return;
            }
        }
    }
    
    printf("Block not found or already air at (%d,%d,%d)\n", worldX, worldY, worldZ);
}

void place_block_at(ChunkManager* manager, int worldX, int worldY, int worldZ, VoxelType blockType) {
    if (!manager) return;
    
    // Calculate chunk coordinates
    int chunkX = worldX / 16;
    int chunkY = worldY / 16;
    int chunkZ = worldZ / 16;
    
    // Calculate local block coordinates within chunk
    int localX = worldX % 16;
    int localY = worldY % 16;
    int localZ = worldZ % 16;
    
    // Handle negative coordinates correctly
    if (localX < 0) { localX += 16; chunkX--; }
    if (localY < 0) { localY += 16; chunkY--; }
    if (localZ < 0) { localZ += 16; chunkZ--; }
    
    // Find the chunk
    for (int i = 0; i < manager->loadedChunks; i++) {
        VoxelChunk* chunk = manager->chunks[i];
        if (chunk && chunk->chunkX == chunkX && chunk->chunkY == chunkY && chunk->chunkZ == chunkZ) {
            VoxelBlock* block = &chunk->blocks[localX][localY][localZ];
            
            // Only place in air blocks
            if (block->type == VOXEL_AIR) {
                printf("Placing block at (%d,%d,%d) type=%d\n", worldX, worldY, worldZ, blockType);
                
                // Get block blueprint
                BlockBlueprint* blueprints = create_block_blueprints();
                if (blueprints) {
                    BlockBlueprint* blueprint = get_block_blueprint(blueprints, blockType);
                    if (blueprint) {
                        initialize_block_from_blueprint(block, blueprint);
                        
                        // Mark chunk for remeshing
                        chunk->needsRemesh = TRUE;
                        
                        // Recalculate face visibility for this block and adjacent blocks
                        calculate_block_faces(chunk, localX, localY, localZ);
                        
                        // Recalculate faces for adjacent blocks (handle chunk rebases)
                        for (int dx = -1; dx <= 1; dx++) {
                            for (int dy = -1; dy <= 1; dy++) {
                                for (int dz = -1; dz <= 1; dz++) {
                                    if (dx == 0 && dy == 0 && dz == 0) continue;
                                    
                                    int adjX = localX + dx;
                                    int adjY = localY + dy;
                                    int adjZ = localZ + dz;
                                    
                                    // Handle chunk rebases for adjacent blocks
                                    int adjChunkX = chunkX;
                                    int adjChunkY = chunkY;
                                    int adjChunkZ = chunkZ;
                                    
                                    if (adjX < 0) { adjX += 16; adjChunkX--; }
                                    else if (adjX >= 16) { adjX -= 16; adjChunkX++; }
                                    if (adjY < 0) { adjY += 16; adjChunkY--; }
                                    else if (adjY >= 16) { adjY -= 16; adjChunkY++; }
                                    if (adjZ < 0) { adjZ += 16; adjChunkZ--; }
                                    else if (adjZ >= 16) { adjZ -= 16; adjChunkZ++; }
                                    
                                    // Find adjacent chunk and mark for remeshing
                                    for (int j = 0; j < manager->loadedChunks; j++) {
                                        VoxelChunk* adjChunk = manager->chunks[j];
                                        if (adjChunk && adjChunk->chunkX == adjChunkX && 
                                            adjChunk->chunkY == adjChunkY && adjChunk->chunkZ == adjChunkZ) {
                                            adjChunk->needsRemesh = TRUE;
                                            calculate_block_faces(adjChunk, adjX, adjY, adjZ);
                                            break;
                                        }
                                    }
                                    
                                    if (adjX >= 0 && adjX < 16 && adjY >= 0 && adjY < 16 && adjZ >= 0 && adjZ < 16) {
                                        calculate_block_faces(chunk, adjX, adjY, adjZ);
                                    }
                                }
                            }
                        }
                        
                        printf("Block placed successfully\n");
                        return;
                    }
                }
            }
        }
    }
    
    printf("Cannot place block at (%d,%d,%d) - not air or chunk not found\n", worldX, worldY, worldZ);
}

// Minecraft-style block interaction with edge detection and cooldown
void handle_block_interaction(ChunkManager* manager, BlockSelection* selection, BOOL leftClick, BOOL rightClick) {
    if (!manager || !selection || !selection->hit) {
        printf("Block interaction: No valid selection\n");
        return;
    }
    
    // Check cooldown
    Player* player = get_player();
    if (!player || !can_perform_block_action(player)) {
        printf("Block interaction: Cooldown active\n");
        return;
    }
    
    // Check reach distance
    if (selection->distance > player->blockReach) {
        printf("Block interaction: Out of reach (distance: %.2f, max: %.2f)\n", 
               selection->distance, player->blockReach);
        return;
    }
    
    if (leftClick) {
        // Break block (edge detection - only on button press, not hold)
        printf("LEFT CLICK: Breaking block at (%d,%d,%d)\n", 
               selection->blockX, selection->blockY, selection->blockZ);
        break_block_at(manager, selection->blockX, selection->blockY, selection->blockZ);
        set_block_action_cooldown(player);
    } else if (rightClick) {
        // Place block at the face position (magnetic to the face being looked at)
        int placeX = selection->blockX + selection->faceX;
        int placeY = selection->blockY + selection->faceY;
        int placeZ = selection->blockZ + selection->faceZ;
        
        VoxelType blockType = player_get_current_block_type(player);
        
        printf("RIGHT CLICK: Placing block type %d at (%d,%d,%d) face=(%d,%d,%d)\n", 
               blockType, placeX, placeY, placeZ, selection->faceX, selection->faceY, selection->faceZ);
        
        // Determine which face we're looking at for better debugging
        const char* faceName = "unknown";
        if (selection->faceX == 1) faceName = "right";
        else if (selection->faceX == -1) faceName = "left";
        else if (selection->faceY == 1) faceName = "front";
        else if (selection->faceY == -1) faceName = "back";
        else if (selection->faceZ == 1) faceName = "top";
        else if (selection->faceZ == -1) faceName = "bottom";
        
        printf("Block will be placed on the %s face of block (%d,%d,%d)\n", 
               faceName, selection->blockX, selection->blockY, selection->blockZ);
        
        // Place block with current type
        place_block_at(manager, placeX, placeY, placeZ, blockType);
        set_block_action_cooldown(player);
    }
}

// Block interaction extras
BOOL can_perform_block_action(Player* player) {
    if (!player) return FALSE;
    
    DWORD currentTime = GetTickCount();
    return (currentTime - player->lastBlockAction) >= player->blockActionCooldown;
}

void set_block_action_cooldown(Player* player) {
    if (!player) return;
    
    player->lastBlockAction = GetTickCount();
}

void player_change_block_type(Player* player, VoxelType newType) {
    if (!player) return;
    
    player->currentBlockType = newType;
    printf("Block type changed to: %d\n", newType);
}

VoxelType player_get_current_block_type(Player* player) {
    if (!player) return VOXEL_GRASS;
    
    return player->currentBlockType;
}

void render_player_hitbox(Player* player) {
    if (!player) return;
    
    glPushMatrix();
    glTranslatef(player->position.x, player->position.y, player->position.z);
    
    // Set transparent material
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.5f, 1.0f, 0.3f); // Blue transparent
    
    // Draw wireframe cube (no faces, just lines)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    
    float w = player->width / 2.0f;
    float h = player->height / 2.0f;
    float d = player->depth / 2.0f;
    
    glBegin(GL_QUADS);
    
    // Front face
    glVertex3f(-w, -h, d);
    glVertex3f(w, -h, d);
    glVertex3f(w, h, d);
    glVertex3f(-w, h, d);
    
    // Back face
    glVertex3f(-w, -h, -d);
    glVertex3f(-w, h, -d);
    glVertex3f(w, h, -d);
    glVertex3f(w, -h, -d);
    
    // Top face
    glVertex3f(-w, h, -d);
    glVertex3f(-w, h, d);
    glVertex3f(w, h, d);
    glVertex3f(w, h, -d);
    
    // Bottom face
    glVertex3f(-w, -h, -d);
    glVertex3f(w, -h, -d);
    glVertex3f(w, -h, d);
    glVertex3f(-w, -h, d);
    
    // Right face
    glVertex3f(w, -h, -d);
    glVertex3f(w, h, -d);
    glVertex3f(w, h, d);
    glVertex3f(w, -h, d);
    
    // Left face
    glVertex3f(-w, -h, -d);
    glVertex3f(-w, -h, d);
    glVertex3f(-w, h, d);
    glVertex3f(-w, h, -d);
    
    glEnd();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void camera_follow_player(RenderCamera* camera, Player* player) {
    if (!camera || !player) return;
    
    // Position camera at player position + eye height
    camera->position = render_vect3_add(player->position, render_vect3_create(0, 0, camera->eye_height));
    camera->target = render_vect3_add(camera->position, camera->forward);
}

// Performance monitoring functions
void update_fps_counter() {
    g_frame_count++;
    
    // Get current time
    static DWORD last_time = 0;
    DWORD current_time = GetTickCount();
    
    if (last_time == 0) {
        last_time = current_time;
        return;
    }
    
    g_delta_time = (current_time - last_time) / 1000.0f;
    last_time = current_time;
    
    // Update FPS every second
    g_last_fps_time += g_delta_time;
    if (g_last_fps_time >= 1.0f) {
        g_current_fps = g_frame_count / g_last_fps_time;
        g_frame_count = 0;
        g_last_fps_time = 0.0f;
    }
}

float get_current_fps() {
    return g_current_fps;
}

float get_delta_time() {
    return g_delta_time;
}

// Get memory usage in MB
float get_memory_usage_mb() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / (1024.0f * 1024.0f);
    }
    return 0.0f;
}

// Render FPS and memory info on screen
void render_performance_info() {
    // Save current OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    
    // Set 2D projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g_renderer_context.width, g_renderer_context.height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Disable depth testing and lighting for 2D text
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    
    // Set text color (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // Render FPS as simple colored rectangles (visual representation)
    glColor3f(0.0f, 1.0f, 0.0f); // Green for FPS
    glBegin(GL_QUADS);
    glVertex2f(10, 10);
    glVertex2f(10 + (g_current_fps / 60.0f) * 100, 10);
    glVertex2f(10 + (g_current_fps / 60.0f) * 100, 25);
    glVertex2f(10, 25);
    glEnd();
    
    // Render memory usage as colored rectangles
    glColor3f(1.0f, 0.0f, 0.0f); // Red for memory
    float mem_usage = get_memory_usage_mb() / 100.0f; // Scale to 100MB
    if (mem_usage > 1.0f) mem_usage = 1.0f;
    
    glBegin(GL_QUADS);
    glVertex2f(10, 30);
    glVertex2f(10 + mem_usage * 100, 30);
    glVertex2f(10 + mem_usage * 100, 45);
    glVertex2f(10, 45);
    glEnd();
    
    // Render delta time as colored rectangles
    glColor3f(0.0f, 0.0f, 1.0f); // Blue for delta time
    float dt_usage = (g_delta_time * 1000.0f) / 16.67f; // Scale to 16.67ms (60 FPS)
    if (dt_usage > 1.0f) dt_usage = 1.0f;
    
    glBegin(GL_QUADS);
    glVertex2f(10, 50);
    glVertex2f(10 + dt_usage * 100, 50);
    glVertex2f(10 + dt_usage * 100, 65);
    glVertex2f(10, 65);
    glEnd();
    
    // Restore OpenGL state
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}

// Global access functions
OpenGLContext* get_renderer_context() { return &g_renderer_context; }
RenderCamera* get_render_camera() { return &g_render_camera; }
Player* get_player() { return g_player; }
RenderLight* get_render_lights() { return g_render_lights; }
int get_render_light_count() { return g_render_light_count; }
RenderFog* get_render_fog() { return &g_render_fog; }
VolumetricSystem* get_volumetric_system() { return g_volumetric_system; }
AdvancedShadowSystem* get_shadow_system() { return g_shadow_system; }
