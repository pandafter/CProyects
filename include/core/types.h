#ifndef TYPES_H
#define TYPES_H

#include <windows.h>
#include <stdbool.h>

// Basic types
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

// Boolean type
#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

// Color structure
typedef struct {
    uint8 r, g, b;
} Color;

// 2D Point
typedef struct {
    int x, y;
} Point2D;

// 3D Vector
typedef struct {
    float x, y, z;
} Vect3;

// 3D Vertex
typedef struct {
    Vect3 position;
    Vect3 normal;
    float u, v;  // Texture coordinates
    Color color;
} Vertex3D;

// 3D Triangle
typedef struct {
    Vertex3D v0, v1, v2;
    Color color;
} Triangle3D;

// Material structure
typedef struct {
    Color albedo;
    float metallic;
    float roughness;
    float emissive;
} Material;

// Texture structure
typedef struct {
    Color* pixels;
    int width, height;
} Texture;

#endif // TYPES_H
