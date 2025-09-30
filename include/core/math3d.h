#ifndef MATH3D_H
#define MATH3D_H

#include "types.h"
#include <math.h>

// Vector operations
Vect3 vect3_create(float x, float y, float z);
Vect3 vect3_add(Vect3 a, Vect3 b);
Vect3 vect3_subtract(Vect3 a, Vect3 b);
Vect3 vect3_scale(Vect3 v, float scalar);
Vect3 vect3_normalize(Vect3 v);
float vect3_dot(Vect3 a, Vect3 b);
Vect3 vect3_cross(Vect3 a, Vect3 b);
float vect3_length(Vect3 v);
float vect3_length_squared(Vect3 v);

// Matrix operations
typedef struct {
    float m[4][4];
} Matrix4x4;

Matrix4x4 matrix4x4_identity();
Matrix4x4 matrix4x4_multiply(Matrix4x4 a, Matrix4x4 b);
Matrix4x4 matrix4x4_translate(float x, float y, float z);
Matrix4x4 matrix4x4_scale(float x, float y, float z);
Matrix4x4 matrix4x4_rotate_x(float angle);
Matrix4x4 matrix4x4_rotate_y(float angle);
Matrix4x4 matrix4x4_rotate_z(float angle);
Vect3 matrix4x4_transform_point(Matrix4x4 m, Vect3 point);
Vect3 matrix4x4_transform_vector(Matrix4x4 m, Vect3 vector);

// Camera operations
typedef struct {
    Vect3 position;
    Vect3 target;
    Vect3 up;
    Vect3 forward;
    Vect3 right;
    Vect3 up_calculated;
    float fov;
    float nearPlane;
    float farPlane;
    float yaw;
    float pitch;
    float roll;
} Camera;

Camera create_camera(Vect3 position, Vect3 target, float fov);
Vect3 camera_transform_point(Camera camera, Vect3 point);
Point2D project_3d_to_2d(Vect3 point, int screenWidth, int screenHeight, float fov);

// Utility functions
float clamp(float value, float min, float max);
float lerp(float a, float b, float t);
float deg_to_rad(float degrees);
float rad_to_deg(float radians);

#endif // MATH3D_H
