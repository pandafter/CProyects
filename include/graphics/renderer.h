#ifndef RENDERER_H
#define RENDERER_H

#include "core/types.h"
#include "core/math3d.h"
#include <windows.h>
#include "graphics/effects/Volumetrics.h"
#include "graphics/effects/Shadow.h"

// Note: chunk_system.h must be included before this file in .c files
// to avoid circular dependency

// OpenGL context
typedef struct {
    HDC hdc;
    HGLRC hrc;
    int width;
    int height;
} OpenGLContext;

// Player structure with hitbox and physics
typedef struct {
    Vect3 position;           // Posición del jugador
    Vect3 velocity;           // Velocidad del jugador
    Vect3 target_velocity;    // Velocidad objetivo
    float width;              // Ancho del hitbox (2.5m)
    float height;             // Altura del hitbox (2.5m)
    float depth;              // Profundidad del hitbox (2.5m)
    float move_speed;         // Velocidad base de movimiento
    float damping;            // Factor de amortiguación
    float acceleration;       // Aceleración hacia la velocidad objetivo
    float gravity;            // Fuerza de gravedad
    BOOL isGrounded;          // Si está en el suelo
    float jump_force;         // Fuerza de salto
    
    // Camera control integrated into player
    int camera_id;            // ID de referencia para el movimiento de mouse
    float camera_yaw;         // Rotación horizontal de la cámara
    float camera_pitch;       // Rotación vertical de la cámara
    float camera_sensitivity; // Sensibilidad del mouse para la cámara
    
    // Flight system
    BOOL isFlying;            // Si está volando
    float flight_speed;       // Velocidad de vuelo
    DWORD lastSpacePress;     // Timestamp del último press de espacio
    int spacePressCount;      // Contador de presses de espacio
    
    // Variable jump system
    BOOL isSpacePressed;      // Si la tecla espacio está presionada
    DWORD spacePressStart;    // Timestamp cuando se presionó espacio
    float maxJumpForce;       // Fuerza máxima de salto
    float minJumpForce;       // Fuerza mínima de salto
    float jumpChargeTime;     // Tiempo máximo para cargar el salto
    BOOL canJump;             // Si puede saltar (en el suelo)
    DWORD lastJumpTime;       // Timestamp del último salto
    DWORD jumpCooldown;       // Cooldown entre saltos (ms)
    
    // Block interaction system
    float blockReach;         // Maximum reach distance for block interaction
    DWORD lastBlockAction;    // Timestamp of last block action
    DWORD blockActionCooldown; // Cooldown between block actions (ms)
    VoxelType currentBlockType; // Currently selected block type for placing
} Player;

// Camera for rendering with smooth movement
typedef struct {
    Vect3 position;
    Vect3 target;
    Vect3 forward;
    Vect3 right;
    Vect3 up;
    float fov;
    float nearPlane;
    float farPlane;
    float yaw;
    float pitch;
    
    // Camera offset from player
    float eye_height;         // Altura de los ojos desde el suelo del jugador
} RenderCamera;

// Block selection result (raycast)
typedef struct {
    BOOL hit;                 // Si se encontró un bloque
    int blockX, blockY, blockZ; // Coordenadas del bloque seleccionado
    int faceX, faceY, faceZ;  // Normal de la cara (para colocar bloques)
    float distance;           // Distancia al bloque
    int blockType;            // Tipo de bloque seleccionado (usamos int para evitar circular dependency)
} BlockSelection;

// Light structure
typedef struct {
    Vect3 position;
    Vect3 direction;
    Color color;
    float intensity;
    float range;
    int type; // 0 = directional, 1 = point
} RenderLight;

// Fog structure
typedef struct {
    BOOL enabled;
    Vect3 center;
    float radius;
    Color color;
    float density;
    float falloff;
} RenderFog;

// Function declarations
BOOL initialize_renderer(HDC hdc, int width, int height);
void cleanup_renderer(OpenGLContext* context);
void resize_renderer(int width, int height);

// Camera functions
RenderCamera create_render_camera(Vect3 position, float yaw, float pitch, float fov);
void update_camera_vectors(RenderCamera* camera);
void camera_rotate(RenderCamera* camera, float deltaYaw, float deltaPitch);
void camera_move_forward(RenderCamera* camera, float distance);
void camera_move_right(RenderCamera* camera, float distance);
void camera_move_up(RenderCamera* camera, float distance);

// Player functions
Player* create_player(Vect3 position);
void destroy_player(Player* player);
void player_set_movement_input(Player* player, float forward, float right, float up);
void player_update_physics(Player* player, float deltaTime);
void player_jump(Player* player);
void render_player_hitbox(Player* player);

// Variable jump system
void player_handle_space_input(Player* player);
void player_handle_space_release(Player* player);
void player_update_jump_system(Player* player, float deltaTime);
void player_toggle_flight(Player* player);

// Block selection and raycast functions
BlockSelection raycast_from_camera(RenderCamera* camera, ChunkManager* manager, float maxDistance);
void render_block_selection(BlockSelection* selection);

// Minecraft-style block interaction functions
void break_block_at(ChunkManager* manager, int worldX, int worldY, int worldZ);
void place_block_at(ChunkManager* manager, int worldX, int worldY, int worldZ, VoxelType blockType);
void handle_block_interaction(ChunkManager* manager, BlockSelection* selection, BOOL leftClick, BOOL rightClick);

// Block interaction extras
BOOL can_perform_block_action(Player* player);
void set_block_action_cooldown(Player* player);
void player_change_block_type(Player* player, VoxelType newType);
VoxelType player_get_current_block_type(Player* player);

// Surface detection and terrain re-rendering
BOOL is_surface_block(ChunkManager* manager, int worldX, int worldY, int worldZ);
void rerender_terrain_below(ChunkManager* manager, int worldX, int worldY, int worldZ);

// Player camera control functions
void player_update_camera_from_mouse(Player* player, int deltaX, int deltaY);

// Camera functions
void camera_follow_player(RenderCamera* camera, Player* player);
void camera_set_movement_input(RenderCamera* camera, float forward, float right, float up);
void camera_update_smooth_movement(RenderCamera* camera, float deltaTime);
void camera_apply_damping(RenderCamera* camera, float deltaTime);
void Camera_UpdateFromMouse(RenderCamera* camera, int deltaX, int deltaY, float sensitivity);

// Rendering functions
void begin_frame();
void end_frame(HDC hdc);
void render_scene(RenderCamera camera, RenderLight* lights, int lightCount, RenderFog fog);
void render_test_environment(void);

// Utility functions
Vect3 render_vect3_create(float x, float y, float z);
Vect3 render_vect3_add(Vect3 a, Vect3 b);
Vect3 render_vect3_subtract(Vect3 a, Vect3 b);
Vect3 render_vect3_scale(Vect3 v, float scalar);
Vect3 render_vect3_normalize(Vect3 v);
float render_vect3_dot(Vect3 a, Vect3 b);
Vect3 render_vect3_cross(Vect3 a, Vect3 b);

// Performance monitoring functions
void update_fps_counter();
float get_current_fps();
float get_delta_time();
float get_memory_usage_mb();
void render_performance_info();

// Global access functions
OpenGLContext* get_renderer_context();
RenderCamera* get_render_camera();
Player* get_player();
RenderLight* get_render_lights();
int get_render_light_count();
RenderFog* get_render_fog();
VolumetricSystem* get_volumetric_system();
AdvancedShadowSystem* get_shadow_system();

#endif // RENDERER_H
