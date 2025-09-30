#ifndef RENDERER_H
#define RENDERER_H

#include "core/types.h"
#include "core/math3d.h"
#include <windows.h>

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

// Camera functions
void camera_follow_player(RenderCamera* camera, Player* player);
void camera_set_movement_input(RenderCamera* camera, float forward, float right, float up);
void camera_update_smooth_movement(RenderCamera* camera, float deltaTime);
void camera_apply_damping(RenderCamera* camera, float deltaTime);

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

#endif // RENDERER_H
