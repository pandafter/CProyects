#ifndef MATH2D_H
#define MATH2D_H

#include <windows.h>
#include <math.h>

// COLOR RGB (para renderizado)
typedef struct {
    unsigned char r, g, b;
} Color;

// VECTOR 2D
typedef struct {
    float x, y;
} Vect2;

// PUNTO EN PANTALLA
typedef struct {
    int x, y;
} Point2D;

// Declaraciones de funciones 2D
Vect2 vect2_create(float x, float y);
Vect2 vect2_add(Vect2 a, Vect2 b);
Vect2 vect2_scale(Vect2 v, float scalar);
float vec2_distance(Vect2 a, Vect2 b);

// Funciones de renderizado 2D
void draw_point(HDC hdc, int x, int y, Color color);
void draw_line(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
void draw_circle(HDC hdc, int centerX, int centerY, int radius, Color color);

#endif
