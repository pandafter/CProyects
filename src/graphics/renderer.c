#include "graphics/renderer.h"
#include "graphics/window.h"
#include "world/chunk_system.h"
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
    
    // Initialize lights - reduced intensity
    g_render_lights[0] = (RenderLight){
        render_vect3_create(0, 0, 100),  // position - above the scene
        render_vect3_create(0, 0, -1),   // direction - pointing down
        (Color){100, 100, 100},           // color - reduced white light
        0.4f,                             // intensity - reduced
        0,                                // range (directional)
        0                                 // type (directional)
    };
    
    g_render_lights[1] = (RenderLight){
        render_vect3_create(0, 0, 30),   // position - ambient light
        render_vect3_create(0, 0, -1),   // direction
        (Color){50, 50, 60},              // color - very soft blue
        0.2f,                             // intensity - reduced
        100.0f,                           // range
        1                                 // type (point)
    };
    
    // Initialize fog - reduced for better visibility
    g_render_fog = (RenderFog){
        FALSE,  // enabled - disabled for test environment
        render_vect3_create(0, 0, 0),  // center
        50.0f,  // radius
        (Color){100, 100, 120},  // color
        0.1f,  // density - reduced
        0.05f   // falloff - reduced
    };
    
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
    
    // Player dimensions: 2.5m vs 1m block
    player->width = 2.5f;      // Ancho del hitbox
    player->height = 2.5f;     // Altura del hitbox
    player->depth = 2.5f;      // Profundidad del hitbox
    
    // Movement and physics
    player->move_speed = 5.0f;     // Velocidad base
    player->damping = 0.85f;       // Damping suave
    player->acceleration = 8.0f;   // Aceleración rápida
    player->gravity = -20.0f;      // Fuerza de gravedad
    player->isGrounded = FALSE;    // No está en el suelo inicialmente
    player->jump_force = 8.0f;     // Fuerza de salto
    
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
    // Clear buffers
    glClearColor(0.135f, 0.206f, 0.235f, 1.0f); // Sky blue
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
}

// End frame
void end_frame(HDC hdc) {
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
    printf("Renderizando entorno de prueba...\n");
    
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
            
            printf("Entorno de prueba renderizado.\n");
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
    // Vect3 up_dir = render_vect3_create(0, 0, 1); // Unused
    
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
    
    // Vertical movement (jumping)
    if (up != 0.0f && player->isGrounded) {
        player->velocity.z = player->jump_force;
        player->isGrounded = FALSE;
    }
    
    // Set horizontal target velocity
    player->target_velocity.x = target_vel.x;
    player->target_velocity.y = target_vel.y;
}

void player_update_physics(Player* player, float deltaTime) {
    if (!player) return;
    
    // Apply gravity
    if (!player->isGrounded) {
        player->velocity.z += player->gravity * deltaTime;
    }
    
    // Interpolate horizontal velocity towards target
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
    
    // Apply movement
    Vect3 movement = render_vect3_scale(player->velocity, deltaTime);
    player->position = render_vect3_add(player->position, movement);
    
    // Simple ground collision (z = 0)
    if (player->position.z <= 0) {
        player->position.z = 0;
        player->velocity.z = 0;
        player->isGrounded = TRUE;
    }
}

void player_jump(Player* player) {
    if (!player || !player->isGrounded) return;
    
    player->velocity.z = player->jump_force;
    player->isGrounded = FALSE;
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
