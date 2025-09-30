#include "core/math3d.h"
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Vector operations
Vect3 vect3_create(float x, float y, float z) {
    Vect3 v = {x, y, z};
    return v;
}

Vect3 vect3_add(Vect3 a, Vect3 b) {
    return vect3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vect3 vect3_subtract(Vect3 a, Vect3 b) {
    return vect3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vect3 vect3_scale(Vect3 v, float scalar) {
    return vect3_create(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vect3 vect3_normalize(Vect3 v) {
    float length = vect3_length(v);
    if (length == 0.0f) return vect3_create(0, 0, 0);
    return vect3_scale(v, 1.0f / length);
}

float vect3_dot(Vect3 a, Vect3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vect3 vect3_cross(Vect3 a, Vect3 b) {
    return vect3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float vect3_length(Vect3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vect3_length_squared(Vect3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

// Matrix operations
Matrix4x4 matrix4x4_identity() {
    Matrix4x4 m = {0};
    m.m[0][0] = 1.0f; m.m[1][1] = 1.0f; m.m[2][2] = 1.0f; m.m[3][3] = 1.0f;
    return m;
}

Matrix4x4 matrix4x4_multiply(Matrix4x4 a, Matrix4x4 b) {
    Matrix4x4 result = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return result;
}

Matrix4x4 matrix4x4_translate(float x, float y, float z) {
    Matrix4x4 m = matrix4x4_identity();
    m.m[3][0] = x;
    m.m[3][1] = y;
    m.m[3][2] = z;
    return m;
}

Matrix4x4 matrix4x4_scale(float x, float y, float z) {
    Matrix4x4 m = matrix4x4_identity();
    m.m[0][0] = x;
    m.m[1][1] = y;
    m.m[2][2] = z;
    return m;
}

Matrix4x4 matrix4x4_rotate_x(float angle) {
    Matrix4x4 m = matrix4x4_identity();
    float c = cosf(angle);
    float s = sinf(angle);
    m.m[1][1] = c; m.m[1][2] = -s;
    m.m[2][1] = s; m.m[2][2] = c;
    return m;
}

Matrix4x4 matrix4x4_rotate_y(float angle) {
    Matrix4x4 m = matrix4x4_identity();
    float c = cosf(angle);
    float s = sinf(angle);
    m.m[0][0] = c; m.m[0][2] = s;
    m.m[2][0] = -s; m.m[2][2] = c;
    return m;
}

Matrix4x4 matrix4x4_rotate_z(float angle) {
    Matrix4x4 m = matrix4x4_identity();
    float c = cosf(angle);
    float s = sinf(angle);
    m.m[0][0] = c; m.m[0][1] = -s;
    m.m[1][0] = s; m.m[1][1] = c;
    return m;
}

Vect3 matrix4x4_transform_point(Matrix4x4 m, Vect3 point) {
    Vect3 result;
    result.x = m.m[0][0] * point.x + m.m[1][0] * point.y + m.m[2][0] * point.z + m.m[3][0];
    result.y = m.m[0][1] * point.x + m.m[1][1] * point.y + m.m[2][1] * point.z + m.m[3][1];
    result.z = m.m[0][2] * point.x + m.m[1][2] * point.y + m.m[2][2] * point.z + m.m[3][2];
    return result;
}

Vect3 matrix4x4_transform_vector(Matrix4x4 m, Vect3 vector) {
    Vect3 result;
    result.x = m.m[0][0] * vector.x + m.m[1][0] * vector.y + m.m[2][0] * vector.z;
    result.y = m.m[0][1] * vector.x + m.m[1][1] * vector.y + m.m[2][1] * vector.z;
    result.z = m.m[0][2] * vector.x + m.m[1][2] * vector.y + m.m[2][2] * vector.z;
    return result;
}

// Camera operations
Camera create_camera(Vect3 position, Vect3 target, float fov) {
    Camera camera = {0};
    camera.position = position;
    camera.target = target;
    camera.fov = fov;
    camera.nearPlane = 0.1f;
    camera.farPlane = 1000.0f;
    camera.up = vect3_create(0, 0, 1);
    
    // Calculate forward vector
    camera.forward = vect3_subtract(target, position);
    camera.forward = vect3_normalize(camera.forward);
    
    // Calculate right vector
    camera.right = vect3_cross(camera.forward, camera.up);
    camera.right = vect3_normalize(camera.right);
    
    // Calculate up vector
    camera.up_calculated = vect3_cross(camera.right, camera.forward);
    camera.up_calculated = vect3_normalize(camera.up_calculated);
    
    return camera;
}

Vect3 camera_transform_point(Camera camera, Vect3 point) {
    // Translate to camera space
    Vect3 translated = vect3_subtract(point, camera.position);
    
    // Create view matrix
    Matrix4x4 view = matrix4x4_identity();
    view.m[0][0] = camera.right.x; view.m[0][1] = camera.right.y; view.m[0][2] = camera.right.z;
    view.m[1][0] = camera.up_calculated.x; view.m[1][1] = camera.up_calculated.y; view.m[1][2] = camera.up_calculated.z;
    view.m[2][0] = -camera.forward.x; view.m[2][1] = -camera.forward.y; view.m[2][2] = -camera.forward.z;
    
    return matrix4x4_transform_vector(view, translated);
}

Point2D project_3d_to_2d(Vect3 point, int screenWidth, int screenHeight, float fov) {
    Point2D result = {-1, -1};
    
    // Check if point is behind camera
    if (point.z >= 0) return result;
    
    // Perspective projection
    float aspect = (float)screenWidth / (float)screenHeight;
    float fov_rad = deg_to_rad(fov);
    float f = 1.0f / tanf(fov_rad / 2.0f);
    
    float x = (point.x * f) / (-point.z * aspect);
    float y = (point.y * f) / (-point.z);
    
    // Convert to screen coordinates
    result.x = (int)((x + 1.0f) * 0.5f * screenWidth);
    result.y = (int)((1.0f - y) * 0.5f * screenHeight);
    
    return result;
}

// Utility functions
float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float deg_to_rad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

float rad_to_deg(float radians) {
    return radians * (180.0f / M_PI);
}
