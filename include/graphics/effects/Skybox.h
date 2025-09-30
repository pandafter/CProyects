#ifndef SKYBOX_H
#define SKYBOX_H

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

// Skybox structure
typedef struct {
    GLuint vao, vbo;
    GLuint prog;
    GLuint cubemap;
    BOOL initialized;
} Skybox;

// Skybox functions
void Skybox_Init(Skybox* sb);
void Skybox_LoadCubemap(Skybox* sb,
    const char* right,  const char* left,
    const char* top,    const char* bottom,
    const char* front,  const char* back);
void Skybox_Draw(Skybox* sb, int w, int h,
                 const float* view /*4x4*/,
                 const float* proj /*4x4*/);
void Skybox_Destroy(Skybox* sb);

// Procedural skybox (without textures)
void Skybox_CreateProcedural(Skybox* sb);

#endif // SKYBOX_H
