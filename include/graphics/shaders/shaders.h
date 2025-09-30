#ifndef SHADERS_H
#define SHADERS_H

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include "core/math3d.h"

// Define missing constants for older OpenGL
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_FRAMEBUFFER_SRGB
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif

// Define GLchar if not available
#ifndef GLchar
typedef char GLchar;
#endif

// Shader program structure
typedef struct {
    GLuint program;
    GLuint vertexShader;
    GLuint fragmentShader;
    BOOL isLinked;
} ShaderProgram;

// Shader uniforms structure
typedef struct {
    GLint uModel;
    GLint uView;
    GLint uProjection;
    GLint uNormalMatrix;
    GLint uLightDir;
    GLint uLightColor;
    GLint uAmbient;
    GLint uSunColor;
    GLint uFogDensity;
    GLint uFogColor;
    GLint uFogIntensity;
    GLint uWorldPos;
    GLint uWorldNrm;
} ShaderUniforms;

// Lighting system for shaders
typedef struct {
    Vect3 lightDirection;    // World space, normalized
    Color lightColor;        // HDR color (3-8 range)
    float ambient;           // Low ambient (≤0.03)
    Color sunColor;          // HDR sun color
    float fogDensity;        // Fog density
    Color fogColor;          // Fog color
    float fogIntensity;      // Fog intensity
} ShaderLighting;

// Function declarations
ShaderProgram create_shader_program(const char* vertexSource, const char* fragmentSource);
void destroy_shader_program(ShaderProgram* program);
BOOL compile_shader(GLuint shader, const char* source);
BOOL link_shader_program(ShaderProgram* program);

// Shader creation functions
ShaderProgram create_lit_shader_program();
ShaderProgram create_fog_shader_program();

// Shader uniform functions
ShaderUniforms get_shader_uniforms(ShaderProgram program);
void set_shader_matrices(ShaderUniforms uniforms, float* model, float* view, float* projection);
void set_shader_lighting(ShaderUniforms uniforms, ShaderLighting lighting);
void set_shader_fog(ShaderUniforms uniforms, float density, Color color, float intensity);

// Matrix utility functions
void create_model_matrix(float* matrix, Vect3 position, Vect3 scale, Vect3 rotation);
void create_view_matrix(float* matrix, Vect3 position, Vect3 target, Vect3 up);
void create_projection_matrix(float* matrix, float fov, float aspect, float nearPlane, float farPlane);
void create_normal_matrix(float* normalMatrix, float* modelMatrix);

// OpenGL state management
void enable_srgb_framebuffer();
void disable_srgb_framebuffer();
void set_clear_color_test(float r, float g, float b, float a);

#endif // SHADERS_H
