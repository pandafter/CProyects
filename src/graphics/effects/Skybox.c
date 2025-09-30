#include "graphics/effects/Skybox.h"
#include "core/math3d.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 36 vértices (cubo unitario)
static const float SKYBOX_VERTS[] = {
    // back face
    -1,-1,-1,  1,-1,-1,  1, 1,-1,
     1, 1,-1, -1, 1,-1, -1,-1,-1,
    // left
    -1,-1, 1, -1,-1,-1, -1, 1,-1,
    -1, 1,-1, -1, 1, 1, -1,-1, 1,
    // right
     1,-1,-1,  1,-1, 1,  1, 1, 1,
     1, 1, 1,  1, 1,-1,  1,-1,-1,
    // front
    -1,-1, 1, -1, 1, 1,  1, 1, 1,
     1, 1, 1,  1,-1, 1, -1,-1, 1,
    // top
    -1, 1,-1,  1, 1,-1,  1, 1, 1,
     1, 1, 1, -1, 1, 1, -1, 1,-1,
    // bottom
    -1,-1,-1, -1,-1, 1,  1,-1, 1,
     1,-1, 1,  1,-1,-1, -1,-1,-1
};

// Function pointer declarations for OpenGL extensions (only for functions not in core OpenGL)
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
static PFNGLUNIFORM3FPROC glUniform3f = NULL;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
static PFNGLGENBUFFERSPROC glGenBuffers = NULL;
static PFNGLBINDBUFFERPROC glBindBuffer = NULL;
static PFNGLBUFFERDATAPROC glBufferData = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
static PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
static PFNGLUNIFORM1IPROC glUniform1i = NULL;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;

// Initialize OpenGL function pointers
static BOOL init_opengl_functions() {
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
    glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
    glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    
    return (glCreateShader && glCreateProgram && glShaderSource && glCompileShader &&
            glGetShaderiv && glGetShaderInfoLog && glAttachShader && glLinkProgram &&
            glGetProgramiv && glGetProgramInfoLog && glDeleteShader && glDeleteProgram &&
            glUseProgram && glGetUniformLocation && glUniformMatrix4fv && glUniform3f &&
            glGenVertexArrays && glBindVertexArray && glGenBuffers && glBindBuffer &&
            glBufferData && glEnableVertexAttribArray && glVertexAttribPointer &&
            glActiveTexture && glUniform1i && glDeleteVertexArrays && glDeleteBuffers);
}

static GLuint compile_shader(GLenum type, const char* src) {
    if (!glCreateShader) return 0;
    
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(s, 2048, NULL, log);
        printf("Shader compilation error: %s\n", log);
    }
    
    return s;
}

static GLuint link_program(GLuint vs, GLuint fs) {
    if (!glCreateProgram) return 0;
    
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(p, 2048, NULL, log);
        printf("Shader linking error: %s\n", log);
    }
    
    // Note: glDetachShader is not available in older OpenGL versions
    // glDetachShader(p, vs);
    // glDetachShader(p, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return p;
}

void Skybox_Init(Skybox* sb) {
    if (!init_opengl_functions()) {
        printf("ERROR: OpenGL functions not available for skybox\n");
        return;
    }
    
    memset(sb, 0, sizeof(Skybox));
    
    // Generate VAO and VBO
    if (glGenVertexArrays && glBindVertexArray && glGenBuffers && glBindBuffer && 
        glBufferData && glEnableVertexAttribArray && glVertexAttribPointer) {
        glGenVertexArrays(1, &sb->vao);
        glGenBuffers(1, &sb->vbo);
        
        glBindVertexArray(sb->vao);
        glBindBuffer(GL_ARRAY_BUFFER, sb->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(SKYBOX_VERTS), SKYBOX_VERTS, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        
        glBindVertexArray(0);
    } else {
        printf("WARNING: VAO/VBO functions not available, using immediate mode\n");
        sb->vao = 0;
        sb->vbo = 0;
    }
    
    // Create procedural skybox shaders
    const char* vsrc =
        "#version 120\n"
        "attribute vec3 aPos;\n"
        "uniform mat4 uView;\n"
        "uniform mat4 uProj;\n"
        "varying vec3 vDir;\n"
        "void main(){\n"
        "  mat3 R = mat3(uView);\n"
        "  vec3 dir = R * aPos;\n"
        "  vDir = dir;\n"
        "  vec4 clip = uProj * vec4(dir, 1.0);\n"
        "  gl_Position = clip.xyww;\n"
        "}\n";
    
    const char* fsrc =
        "#version 120\n"
        "varying vec3 vDir;\n"
        "uniform vec3 uSunDir;\n"
        "void main(){\n"
        "  vec3 d = normalize(vDir);\n"
        "  \n"
        "  // Procedural sky colors for realistic sunny day\n"
        "  float sunDot = max(dot(d, -uSunDir), 0.0);\n"
        "  \n"
        "  // Realistic sky gradient (bright blue to white)\n"
        "  vec3 skyColor = mix(vec3(0.4, 0.6, 1.0), vec3(0.8, 0.9, 1.0), d.y * 0.7 + 0.3);\n"
        "  \n"
        "  // Sun disk with realistic glow\n"
        "  if (sunDot > 0.995) {\n"
        "    float sunIntensity = (sunDot - 0.995) * 200.0;\n"
        "    skyColor = mix(skyColor, vec3(1.0, 0.95, 0.8), sunIntensity);\n"
        "  }\n"
        "  \n"
        "  // Sun corona effect\n"
        "  if (sunDot > 0.98) {\n"
        "    float coronaIntensity = (sunDot - 0.98) * 50.0;\n"
        "    skyColor = mix(skyColor, vec3(1.0, 0.9, 0.7), coronaIntensity * 0.3);\n"
        "  }\n"
        "  \n"
        "  // Atmospheric perspective (darker at horizon)\n"
        "  float horizonFade = 1.0 - abs(d.y);\n"
        "  skyColor = mix(skyColor, vec3(0.3, 0.5, 0.8), horizonFade * 0.2);\n"
        "  \n"
        "  gl_FragColor = vec4(skyColor, 1.0);\n"
        "}\n";
    
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vsrc);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fsrc);
    sb->prog = link_program(vs, fs);
    
    sb->initialized = TRUE;
    printf("Skybox initialized successfully\n");
}

void Skybox_Draw(Skybox* sb, int w, int h, const float* view, const float* proj) {
    (void)w; // Suppress unused parameter warning
    (void)h; // Suppress unused parameter warning
    if (!sb->initialized || !glUseProgram) return;
    
    // Save current state
    GLboolean depthMask;
    GLint depthFunc;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    
    // Set skybox rendering state
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    
    glUseProgram(sb->prog);
    
    // Set uniforms
    GLint locV = glGetUniformLocation(sb->prog, "uView");
    GLint locP = glGetUniformLocation(sb->prog, "uProj");
    GLint locS = glGetUniformLocation(sb->prog, "uSunDir");
    
    if (locV >= 0) glUniformMatrix4fv(locV, 1, GL_FALSE, view);
    if (locP >= 0) glUniformMatrix4fv(locP, 1, GL_FALSE, proj);
    if (locS >= 0) glUniform3f(locS, 0.0, -1.0, 0.0); // Sun direction
    
    // Draw skybox
    if (sb->vao && glBindVertexArray) {
        glBindVertexArray(sb->vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    } else {
        // Fallback to immediate mode
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < 36; i++) {
            glVertex3f(SKYBOX_VERTS[i*3], SKYBOX_VERTS[i*3+1], SKYBOX_VERTS[i*3+2]);
        }
        glEnd();
    }
    
    // Restore state
    glDepthMask(depthMask);
    glDepthFunc(depthFunc);
    glEnable(GL_CULL_FACE);
}

void Skybox_Destroy(Skybox* sb) {
    if (!sb->initialized) return;
    
    if (sb->vao && glDeleteVertexArrays) {
        glDeleteVertexArrays(1, &sb->vao);
    }
    if (sb->vbo && glDeleteBuffers) {
        glDeleteBuffers(1, &sb->vbo);
    }
    if (sb->prog && glDeleteProgram) {
        glDeleteProgram(sb->prog);
    }
    if (sb->cubemap) {
        glDeleteTextures(1, &sb->cubemap);
    }
    
    memset(sb, 0, sizeof(Skybox));
    printf("Skybox destroyed\n");
}

void Skybox_CreateProcedural(Skybox* sb) {
    Skybox_Init(sb);
}
