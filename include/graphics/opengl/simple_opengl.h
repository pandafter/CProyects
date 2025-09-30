#ifndef SIMPLE_OPENGL_H
#define SIMPLE_OPENGL_H

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "core/math3d.h"

// Simple OpenGL Context
typedef struct {
    HDC hdc;
    HGLRC hrc;
    int width;
    int height;
} SimpleOpenGLContext;

// Simple OpenGL Camera
typedef struct {
    Vect3 position;
    Vect3 target;
    Vect3 up;
    float fov;
    float nearPlane;
    float farPlane;
    float yaw;
    float pitch;
    Vect3 forward;
    Vect3 right;
    Vect3 up_calculated;
} SimpleOpenGLCamera;

// Simple OpenGL Light
typedef struct {
    Vect3 position;
    Vect3 direction;
    Color color;
    float intensity;
    float range;
    int type; // 0=directional, 1=point, 2=spot
} SimpleOpenGLLight;

// Simple OpenGL Fog
typedef struct {
    BOOL enabled;
    Vect3 center;
    float radius;
    Color color;
    float density;
    float falloff;
} SimpleOpenGLFog;

// Simple OpenGL Cube
typedef struct {
    Vect3 position;
    Vect3 scale;
    Color material;
    Vect3 velocity;
    Vect3 acceleration;
    float mass;
    BOOL isCollidable;
    BOOL isStatic;
    BOOL affectedByGravity;
    BOOL isFalling;
} SimpleOpenGLCube;

// Function declarations
BOOL initialize_simple_opengl(HDC hdc, int width, int height);
void cleanup_simple_opengl(SimpleOpenGLContext* context);
void resize_simple_opengl(int width, int height);

// Camera functions
SimpleOpenGLCamera create_simple_opengl_camera(Vect3 position, float yaw, float pitch, float fov);
void update_simple_opengl_camera_vectors(SimpleOpenGLCamera* camera);
void simple_opengl_camera_rotate(SimpleOpenGLCamera* camera, float deltaYaw, float deltaPitch);
void simple_opengl_camera_move_forward(SimpleOpenGLCamera* camera, float distance);
void simple_opengl_camera_move_right(SimpleOpenGLCamera* camera, float distance);
void simple_opengl_camera_move_up(SimpleOpenGLCamera* camera, float distance);

// Rendering functions
void render_simple_opengl_scene(SimpleOpenGLCamera camera, SimpleOpenGLCube* cubes, int cubeCount, 
                               SimpleOpenGLLight* lights, int lightCount, SimpleOpenGLFog fog);
void render_simple_cube(SimpleOpenGLCube cube, SimpleOpenGLCamera camera, SimpleOpenGLLight* lights, int lightCount, SimpleOpenGLFog fog);

// Utility functions
Vect3 simple_opengl_vect3_create(float x, float y, float z);
Vect3 simple_opengl_vect3_add(Vect3 a, Vect3 b);
Vect3 simple_opengl_vect3_subtract(Vect3 a, Vect3 b);
Vect3 simple_opengl_vect3_scale(Vect3 v, float scalar);
Vect3 simple_opengl_vect3_normalize(Vect3 v);
float simple_opengl_vect3_dot(Vect3 a, Vect3 b);
Vect3 simple_opengl_vect3_cross(Vect3 a, Vect3 b);

// Global access functions
SimpleOpenGLCamera* get_simple_opengl_camera();
SimpleOpenGLLight* get_simple_opengl_lights();
SimpleOpenGLCube* get_simple_opengl_cubes();
int get_simple_opengl_cube_count();
SimpleOpenGLFog* get_simple_opengl_fog();

// Physics functions
void update_physics();

// Voxel terrain functions (declared in voxel_chunk.h)
// Forward declaration only - actual types defined in voxel_chunk.h
void* get_chunk_manager();

// Cube creation
SimpleOpenGLCube create_simple_opengl_cube(Vect3 position, Vect3 scale, Color color, float mass, 
                                          BOOL isCollidable, BOOL isStatic, BOOL affectedByGravity);

#endif
