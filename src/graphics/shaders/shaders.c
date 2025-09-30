#include "graphics/shaders/shaders.h"
#include "core/math3d.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Define M_PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Vertex shader source for world space lighting
static const char* LIT_VERTEX_SHADER_SOURCE = 
"#version 120\n"
"attribute vec3 aPosition;\n"
"attribute vec3 aNormal;\n"
"attribute vec2 aTexCoord;\n"
"\n"
"uniform mat4 uModel;\n"
"uniform mat4 uView;\n"
"uniform mat4 uProjection;\n"
"uniform mat3 uNormalMatrix;\n"
"\n"
"varying vec3 vWorldPos;\n"
"varying vec3 vWorldNrm;\n"
"varying vec2 vTexCoord;\n"
"\n"
"void main() {\n"
"    vec4 worldPos = uModel * vec4(aPosition, 1.0);\n"
"    vWorldPos = worldPos.xyz;\n"
"    vWorldNrm = normalize(uNormalMatrix * aNormal);\n"
"    vTexCoord = aTexCoord;\n"
"    \n"
"    gl_Position = uProjection * uView * worldPos;\n"
"}\n";

// Fragment shader source for Lambert + Blinn-Phong lighting
static const char* LIT_FRAGMENT_SHADER_SOURCE = 
"#version 120\n"
"varying vec3 vWorldPos;\n"
"varying vec3 vWorldNrm;\n"
"varying vec2 vTexCoord;\n"
"\n"
"uniform vec3 uLightDir;\n"
"uniform vec3 uLightColor;\n"
"uniform float uAmbient;\n"
"uniform vec3 uSunColor;\n"
"uniform float uFogDensity;\n"
"uniform vec3 uFogColor;\n"
"uniform float uFogIntensity;\n"
"\n"
"void main() {\n"
"    // Base material color (bright warm tones for hot day)\n"
"    vec3 albedo = vec3(1.0, 0.9, 0.7);\n"
"    \n"
"    // Normalize normal\n"
"    vec3 normal = normalize(vWorldNrm);\n"
"    \n"
"    // Lambert diffuse lighting (enhanced for bright day)\n"
"    float NdotL = max(dot(normal, -uLightDir), 0.0);\n"
"    vec3 diffuse = albedo * uSunColor * NdotL * 1.5;\n"
"    \n"
"    // Simple specular (Blinn-Phong) - bright for hot day\n"
"    vec3 viewDir = normalize(-vWorldPos);\n"
"    vec3 halfDir = normalize(-uLightDir + viewDir);\n"
"    float NdotH = max(dot(normal, halfDir), 0.0);\n"
"    float specular = pow(NdotH, 8.0);\n"
"    vec3 specularColor = uSunColor * specular * 0.8;\n"
"    \n"
"    // Ambient lighting\n"
"    vec3 ambient = albedo * uAmbient;\n"
"    \n"
"    // Combine lighting\n"
"    vec3 finalColor = ambient + diffuse + specularColor;\n"
"    \n"
"    // Apply fog if density > 0\n"
"    if (uFogDensity > 0.0) {\n"
"        float distance = length(vWorldPos);\n"
"        float fogFactor = exp(-uFogDensity * distance);\n"
"        finalColor = mix(uFogColor * uFogIntensity, finalColor, fogFactor);\n"
"    }\n"
"    \n"
"    gl_FragColor = vec4(finalColor, 1.0);\n"
"}\n";

// Fog shader source
static const char* FOG_FRAGMENT_SHADER_SOURCE = 
"#version 120\n"
"varying vec3 vWorldPos;\n"
"varying vec3 vWorldNrm;\n"
"varying vec2 vTexCoord;\n"
"\n"
"uniform float uFogDensity;\n"
"uniform vec3 uFogColor;\n"
"uniform float uFogIntensity;\n"
"\n"
"void main() {\n"
"    // Early out if fog is disabled\n"
"    if (uFogDensity <= 0.0) {\n"
"        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
"        return;\n"
"    }\n"
"    \n"
"    // Calculate fog\n"
"    float distance = length(vWorldPos);\n"
"    float fogFactor = 1.0 - exp(-uFogDensity * distance);\n"
"    \n"
"    vec3 fogColor = uFogColor * uFogIntensity * fogFactor;\n"
"    gl_FragColor = vec4(fogColor, fogFactor);\n"
"}\n";

// Function pointer declarations for OpenGL extensions
static PFNGLCREATESHADERPROC glCreateShader = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
static PFNGLATTACHSHADERPROC glAttachShader = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
static PFNGLDELETESHADERPROC glDeleteShader = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
static PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = NULL;
static PFNGLUNIFORM3FPROC glUniform3f = NULL;
static PFNGLUNIFORM1FPROC glUniform1f = NULL;

// Initialize OpenGL function pointers
static BOOL init_opengl_functions() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
    glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
    glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
#pragma GCC diagnostic pop
    
    return (glCreateShader && glCreateProgram && glShaderSource && glCompileShader &&
            glGetShaderiv && glGetShaderInfoLog && glAttachShader && glLinkProgram &&
            glGetProgramiv && glGetProgramInfoLog && glDeleteShader && glDeleteProgram &&
            glUseProgram && glGetUniformLocation && glUniformMatrix4fv && glUniformMatrix3fv &&
            glUniform3f && glUniform1f);
}

// Create shader program
ShaderProgram create_shader_program(const char* vertexSource, const char* fragmentSource) {
    ShaderProgram program = {0};
    
    // Initialize OpenGL functions
    if (!init_opengl_functions()) {
        printf("ERROR: OpenGL shader functions not available\n");
        return program;
    }
    
    // Create shaders
    program.vertexShader = glCreateShader(GL_VERTEX_SHADER);
    program.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    if (!program.vertexShader || !program.fragmentShader) {
        printf("ERROR: Failed to create shaders\n");
        return program;
    }
    
    // Compile vertex shader
    if (!compile_shader(program.vertexShader, vertexSource)) {
        printf("ERROR: Failed to compile vertex shader\n");
        glDeleteShader(program.vertexShader);
        glDeleteShader(program.fragmentShader);
        return program;
    }
    
    // Compile fragment shader
    if (!compile_shader(program.fragmentShader, fragmentSource)) {
        printf("ERROR: Failed to compile fragment shader\n");
        glDeleteShader(program.vertexShader);
        glDeleteShader(program.fragmentShader);
        return program;
    }
    
    // Create program
    program.program = glCreateProgram();
    if (!program.program) {
        printf("ERROR: Failed to create shader program\n");
        glDeleteShader(program.vertexShader);
        glDeleteShader(program.fragmentShader);
        return program;
    }
    
    // Attach shaders
    glAttachShader(program.program, program.vertexShader);
    glAttachShader(program.program, program.fragmentShader);
    
    // Link program
    if (!link_shader_program(&program)) {
        printf("ERROR: Failed to link shader program\n");
        glDeleteProgram(program.program);
        glDeleteShader(program.vertexShader);
        glDeleteShader(program.fragmentShader);
        program.program = 0;
        return program;
    }
    
    program.isLinked = TRUE;
    printf("Shader program created successfully\n");
    return program;
}

// Compile shader
BOOL compile_shader(GLuint shader, const char* source) {
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
        return FALSE;
    }
    
    return TRUE;
}

// Link shader program
BOOL link_shader_program(ShaderProgram* program) {
    glLinkProgram(program->program);
    
    GLint success;
    glGetProgramiv(program->program, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(program->program, 1024, NULL, infoLog);
        printf("Shader linking error: %s\n", infoLog);
        return FALSE;
    }
    
    return TRUE;
}

// Destroy shader program
void destroy_shader_program(ShaderProgram* program) {
    if (program->program) {
        glDeleteProgram(program->program);
        program->program = 0;
    }
    if (program->vertexShader) {
        glDeleteShader(program->vertexShader);
        program->vertexShader = 0;
    }
    if (program->fragmentShader) {
        glDeleteShader(program->fragmentShader);
        program->fragmentShader = 0;
    }
    program->isLinked = FALSE;
}

// Create lit shader program
ShaderProgram create_lit_shader_program() {
    return create_shader_program(LIT_VERTEX_SHADER_SOURCE, LIT_FRAGMENT_SHADER_SOURCE);
}

// Create fog shader program
ShaderProgram create_fog_shader_program() {
    return create_shader_program(LIT_VERTEX_SHADER_SOURCE, FOG_FRAGMENT_SHADER_SOURCE);
}

// Get shader uniforms
ShaderUniforms get_shader_uniforms(ShaderProgram program) {
    ShaderUniforms uniforms = {0};
    
    if (!program.isLinked) {
        printf("ERROR: Shader program not linked\n");
        return uniforms;
    }
    
    glUseProgram(program.program);
    
    uniforms.uModel = glGetUniformLocation(program.program, "uModel");
    uniforms.uView = glGetUniformLocation(program.program, "uView");
    uniforms.uProjection = glGetUniformLocation(program.program, "uProjection");
    uniforms.uNormalMatrix = glGetUniformLocation(program.program, "uNormalMatrix");
    uniforms.uLightDir = glGetUniformLocation(program.program, "uLightDir");
    uniforms.uLightColor = glGetUniformLocation(program.program, "uLightColor");
    uniforms.uAmbient = glGetUniformLocation(program.program, "uAmbient");
    uniforms.uSunColor = glGetUniformLocation(program.program, "uSunColor");
    uniforms.uFogDensity = glGetUniformLocation(program.program, "uFogDensity");
    uniforms.uFogColor = glGetUniformLocation(program.program, "uFogColor");
    uniforms.uFogIntensity = glGetUniformLocation(program.program, "uFogIntensity");
    
    printf("Shader uniforms loaded\n");
    return uniforms;
}

// Set shader matrices
void set_shader_matrices(ShaderUniforms uniforms, float* model, float* view, float* projection) {
    if (uniforms.uModel >= 0) glUniformMatrix4fv(uniforms.uModel, 1, GL_FALSE, model);
    if (uniforms.uView >= 0) glUniformMatrix4fv(uniforms.uView, 1, GL_FALSE, view);
    if (uniforms.uProjection >= 0) glUniformMatrix4fv(uniforms.uProjection, 1, GL_FALSE, projection);
    
    // Calculate normal matrix
    float normalMatrix[9];
    create_normal_matrix(normalMatrix, model);
    if (uniforms.uNormalMatrix >= 0) glUniformMatrix3fv(uniforms.uNormalMatrix, 1, GL_FALSE, normalMatrix);
}

// Set shader lighting
void set_shader_lighting(ShaderUniforms uniforms, ShaderLighting lighting) {
    if (uniforms.uLightDir >= 0) {
        glUniform3f(uniforms.uLightDir, lighting.lightDirection.x, lighting.lightDirection.y, lighting.lightDirection.z);
    }
    if (uniforms.uLightColor >= 0) {
        glUniform3f(uniforms.uLightColor, lighting.lightColor.r/255.0f, lighting.lightColor.g/255.0f, lighting.lightColor.b/255.0f);
    }
    if (uniforms.uAmbient >= 0) {
        glUniform1f(uniforms.uAmbient, lighting.ambient);
    }
    if (uniforms.uSunColor >= 0) {
        glUniform3f(uniforms.uSunColor, lighting.sunColor.r/255.0f, lighting.sunColor.g/255.0f, lighting.sunColor.b/255.0f);
    }
}

// Set shader fog
void set_shader_fog(ShaderUniforms uniforms, float density, Color color, float intensity) {
    if (uniforms.uFogDensity >= 0) {
        glUniform1f(uniforms.uFogDensity, density);
    }
    if (uniforms.uFogColor >= 0) {
        glUniform3f(uniforms.uFogColor, color.r/255.0f, color.g/255.0f, color.b/255.0f);
    }
    if (uniforms.uFogIntensity >= 0) {
        glUniform1f(uniforms.uFogIntensity, intensity);
    }
}

// Matrix utility functions
void create_model_matrix(float* matrix, Vect3 position, Vect3 scale, Vect3 rotation) {
    (void)rotation; // Suppress unused parameter warning
    // Simple identity matrix for now
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = scale.x;
    matrix[5] = scale.y;
    matrix[10] = scale.z;
    matrix[12] = position.x;
    matrix[13] = position.y;
    matrix[14] = position.z;
    matrix[15] = 1.0f;
}

void create_view_matrix(float* matrix, Vect3 position, Vect3 target, Vect3 up) {
    // Simple look-at matrix
    Vect3 forward = vect3_subtract(target, position);
    forward = vect3_normalize(forward);
    
    Vect3 right = vect3_cross(forward, up);
    right = vect3_normalize(right);
    
    Vect3 up_calc = vect3_cross(right, forward);
    
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = right.x;
    matrix[1] = up_calc.x;
    matrix[2] = -forward.x;
    matrix[4] = right.y;
    matrix[5] = up_calc.y;
    matrix[6] = -forward.y;
    matrix[8] = right.z;
    matrix[9] = up_calc.z;
    matrix[10] = -forward.z;
    matrix[12] = -vect3_dot(right, position);
    matrix[13] = -vect3_dot(up_calc, position);
    matrix[14] = vect3_dot(forward, position);
    matrix[15] = 1.0f;
}

void create_projection_matrix(float* matrix, float fov, float aspect, float nearPlane, float farPlane) {
    float f = 1.0f / tanf(fov * 0.5f * M_PI / 180.0f);
    
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = f / aspect;
    matrix[5] = f;
    matrix[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    matrix[11] = -1.0f;
    matrix[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
}

void create_normal_matrix(float* normalMatrix, float* modelMatrix) {
    (void)modelMatrix; // Suppress unused parameter warning
    // For now, use identity matrix
    memset(normalMatrix, 0, 9 * sizeof(float));
    normalMatrix[0] = 1.0f;
    normalMatrix[4] = 1.0f;
    normalMatrix[8] = 1.0f;
}

// OpenGL state management
void enable_srgb_framebuffer() {
    // Check if sRGB is supported
    GLboolean srgbSupported = glIsEnabled(GL_FRAMEBUFFER_SRGB);
    if (srgbSupported) {
        glEnable(GL_FRAMEBUFFER_SRGB);
        printf("sRGB framebuffer enabled\n");
    } else {
        printf("sRGB framebuffer not supported, using linear color space\n");
    }
}

void disable_srgb_framebuffer() {
    glDisable(GL_FRAMEBUFFER_SRGB);
    printf("sRGB framebuffer disabled\n");
}

void set_clear_color_test(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    printf("Clear color set to: %.2f, %.2f, %.2f, %.2f\n", r, g, b, a);
}
