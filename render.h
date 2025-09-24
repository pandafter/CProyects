#ifndef RENDER_H
#define RENDER_H

#include <windows.h>
#include "math2d.h"
#include "math3d.h"

// Declaraciones de funciones de renderizado
void render_2d_test(HDC hdc, int width, int height, int mouseX, int mouseY);
void render_3d_test(HDC hdc, int width, int height, int mouseX, int mouseY);
void draw_cube_wireframe(HDC hdc, Cube3D cube, Camera camera, int screenWidth, int screenHeight);
void render_3d_wireframe(HDC hdc, int width, int height, int mouseX, int mouseY);
void draw_cube_wire(HDC hdc, int W, int H, float fov_deg, Cube3D cube, float yaw, float pitch, float roll);

#endif
