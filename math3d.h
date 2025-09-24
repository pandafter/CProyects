#ifndef MATH3D_H
#define MATH3D_H

#include <windows.h>
#include <math.h>
#include "math2d.h"

// VECTOR 3D
typedef struct {
    float x, y, z;
} Vect3;

// CUBO 3D
typedef struct {
    Vect3 position;
    float size;
    Color color;
} Cube3D;

// ESFERA 3D
typedef struct {
    Vect3 position;
    float size;
    Color color;
} Sphere3D;

typedef struct {
    Vect3 position;
    Vect3 target;
    Vect3 up;
    float fov;
    float nearPlane;
    float farPlane;
} Camera;

// Declaraciones de funciones 3D
Vect3 vect3_create(float x, float y, float z);
Vect3 vect3_add(Vect3 a, Vect3 b);
Vect3 vect3_scale(Vect3 v, float scalar);
float vect3_dot(Vect3 a, Vect3 b);
float vect3_magnitude(Vect3 v);
Vect3 vect3_normalize(Vect3 v);
Vect3 vect3_subtract(Vect3 a, Vect3 b);
Vect3 vect3_cross(Vect3 a, Vect3 b);
Camera create_camera(Vect3 position, Vect3 target, float fov);
Vect3 camera_transform_point(Camera cam, Vect3 point);

// Funciones de rotación
Vect3 v3_rotX(Vect3 p, float a);
Vect3 v3_rotY(Vect3 p, float a);
Vect3 v3_rotZ(Vect3 p, float a);

// Funciones de objetos 3D
Cube3D create_cube(Vect3 position, float size, Color color);

// Funciones de proyección
Point2D project_3d_to_2d(Vect3 point3d, int screenWidth, int screenHeight, float fov);
int project_persp(Vect3 pc, int W, int H, float fov_deg, int* sx, int* sy);

#endif
