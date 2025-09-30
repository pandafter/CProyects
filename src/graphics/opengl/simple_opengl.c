#include "graphics/opengl/simple_opengl.h"
#include "graphics/shaders/shaders.h"
#include "graphics/effects/Skybox.h"
#include "world/chunk_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Global OpenGL context
static SimpleOpenGLContext g_context;
static SimpleOpenGLCamera g_camera;
static SimpleOpenGLLight g_lights[4];
static SimpleOpenGLFog g_fog;
static SimpleOpenGLCube g_cubes[4];
static int g_cubeCount = 4;

// Voxel terrain
ChunkManager* g_chunkManager = NULL;

// Shader system
static ShaderProgram g_litShader = {0};
static ShaderProgram g_fogShader = {0};
static ShaderUniforms g_litUniforms = {0};
static ShaderUniforms g_fogUniforms = {0};
static ShaderLighting g_shaderLighting = {0};
static BOOL g_shadersInitialized = FALSE;

// Skybox system
static Skybox g_skybox = {0};
static BOOL g_skyboxInitialized = FALSE;

// Physics constants (currently unused but kept for future implementation)
// static float g_gravity = 9.81f;  // Gravedad realista
// static float g_deltaTime = 0.016f;  // 60 FPS
// static DWORD g_lastPhysicsTime = 0;

// Initialize Simple OpenGL
BOOL initialize_simple_opengl(HDC hdc, int width, int height) {
    g_context.hdc = hdc;
    g_context.width = width;
    g_context.height = height;
    
    // Set pixel format
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (!pixelFormat) return FALSE;
    
    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) return FALSE;
    
    // Create OpenGL context
    g_context.hrc = wglCreateContext(hdc);
    if (!g_context.hrc) return FALSE;
    
    if (!wglMakeCurrent(hdc, g_context.hrc)) return FALSE;
    
    // Set viewport
    glViewport(0, 0, width, height);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Enable sRGB framebuffer for correct color space
    enable_srgb_framebuffer();
    
    // Initialize camera (CORREGIDO: posicionada para ver chunks acostados horizontalmente)
    g_camera = create_simple_opengl_camera(simple_opengl_vect3_create(0, 0, 20), 0, -0.3f, 45.0f);
    
    // Initialize lights
    g_lights[0] = (SimpleOpenGLLight){
        simple_opengl_vect3_create(0, 0, 0),  // position
        simple_opengl_vect3_create(-0.7f, -0.7f, -0.2f),  // direction
        (Color){255, 248, 220},  // color
        1.2f,  // intensity
        0,  // range (directional)
        0   // type (directional)
    };
    
    g_lights[1] = (SimpleOpenGLLight){
        simple_opengl_vect3_create(2, 2, 8),  // position
        simple_opengl_vect3_create(0, 0, 0),  // direction
        (Color){255, 255, 255},  // color
        1.5f,  // intensity
        20.0f,  // range
        1   // type (point)
    };
    
    // Initialize fog for realistic atmosphere
    g_fog = (SimpleOpenGLFog){
        TRUE,  // enabled
        simple_opengl_vect3_create(0, 0, 0),  // center
        60.0f,  // radius
        (Color){200, 220, 255},  // light blue color
        0.02f,  // density
        1.2f   // falloff
    };
    
    // Initialize shaders
    g_litShader = create_lit_shader_program();
    g_fogShader = create_fog_shader_program();
    
    if (g_litShader.isLinked && g_fogShader.isLinked) {
        g_litUniforms = get_shader_uniforms(g_litShader);
        g_fogUniforms = get_shader_uniforms(g_fogShader);
        g_shadersInitialized = TRUE;
        
        // Initialize shader lighting for realistic sunny day
        g_shaderLighting.lightDirection = simple_opengl_vect3_create(0, -1, 0); // Sun direction (from above)
        g_shaderLighting.lightColor = (Color){255, 255, 255};
        g_shaderLighting.ambient = 0.4f; // Higher ambient for bright sunny day
        g_shaderLighting.sunColor = (Color){255, 240, 180}; // Bright warm sun color
        g_shaderLighting.fogDensity = 0.02f; // Light fog for atmosphere
        g_shaderLighting.fogColor = (Color){200, 220, 255}; // Light blue fog
        g_shaderLighting.fogIntensity = 0.6f;
        
        printf("Shader system initialized successfully\n");
    } else {
        printf("ERROR: Failed to initialize shader system\n");
    }
    
    // Initialize skybox
    Skybox_CreateProcedural(&g_skybox);
    if (g_skybox.initialized) {
        g_skyboxInitialized = TRUE;
        printf("Skybox system initialized successfully\n");
    } else {
        printf("ERROR: Failed to initialize skybox system\n");
    }
    
    // No cubes - only voxel terrain (Minecraft-style)
    
    // Initialize voxel terrain system
    g_chunkManager = create_chunk_manager(25, 4);  // 25 chunks max, 4 chunk render distance (REDUCIDO PARA SEGURIDAD)
    TerrainGenerator terrainGenerator = create_terrain_generator(12345);  // Seed for terrain generation
    
    // Generate initial chunks around origin (Minecraft-style floor)
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            VoxelChunk* chunk = get_or_create_chunk(g_chunkManager, x, y, 0);
            if (chunk && !chunk->isGenerated) {
                generate_chunk_terrain(chunk, &terrainGenerator);
            }
        }
    }
    
    return TRUE;
}

// Update physics for all cubes (disabled - only voxel terrain)
void update_physics() {
    // No physics needed for voxel terrain only
    return;
}

// Create Simple OpenGL cube
SimpleOpenGLCube create_simple_opengl_cube(Vect3 position, Vect3 scale, Color color, float mass, 
                                          BOOL isCollidable, BOOL isStatic, BOOL affectedByGravity) {
    SimpleOpenGLCube cube = {0};
    cube.position = position;
    cube.scale = scale;
    cube.material = color;
    cube.velocity = simple_opengl_vect3_create(0, 0, 0);
    cube.acceleration = simple_opengl_vect3_create(0, 0, 0);
    cube.mass = mass;
    cube.isCollidable = isCollidable;
    cube.isStatic = isStatic;
    cube.affectedByGravity = affectedByGravity;
    cube.isFalling = TRUE;
    return cube;
}

// Create Simple OpenGL camera
SimpleOpenGLCamera create_simple_opengl_camera(Vect3 position, float yaw, float pitch, float fov) {
    SimpleOpenGLCamera camera = {0};
    camera.position = position;
    camera.yaw = yaw;
    camera.pitch = pitch;
    camera.fov = fov;
    camera.nearPlane = 0.1f;
    camera.farPlane = 1000.0f;
    camera.up = simple_opengl_vect3_create(0, 0, 1);
    update_simple_opengl_camera_vectors(&camera);
    return camera;
}

// Update camera vectors
void update_simple_opengl_camera_vectors(SimpleOpenGLCamera* camera) {
    camera->forward = simple_opengl_vect3_create(
        sinf(camera->yaw) * cosf(camera->pitch),
        -cosf(camera->yaw) * cosf(camera->pitch),
        sinf(camera->pitch)
    );
    camera->forward = simple_opengl_vect3_normalize(camera->forward);
    
    camera->right = simple_opengl_vect3_cross(camera->forward, simple_opengl_vect3_create(0, 0, 1));
    camera->right = simple_opengl_vect3_normalize(camera->right);
    
    camera->up_calculated = simple_opengl_vect3_cross(camera->right, camera->forward);
    camera->up_calculated = simple_opengl_vect3_normalize(camera->up_calculated);
    
    camera->target = simple_opengl_vect3_add(camera->position, camera->forward);
}

// Camera movement functions
void simple_opengl_camera_rotate(SimpleOpenGLCamera* camera, float deltaYaw, float deltaPitch) {
    camera->yaw += deltaYaw;
    camera->pitch += deltaPitch;
    
    // Limit pitch (vertical rotation)
    if (camera->pitch > 1.57f) camera->pitch = 1.57f;
    if (camera->pitch < -1.57f) camera->pitch = -1.57f;
    
    // INFINITE YAW ROTATION - Allow unlimited horizontal rotation
    // No need to normalize yaw - let it accumulate for infinite rotation
    // This allows the camera to rotate 360° continuously in the horizontal plane
    
    update_simple_opengl_camera_vectors(camera);
}

void simple_opengl_camera_move_forward(SimpleOpenGLCamera* camera, float distance) {
    Vect3 movement = simple_opengl_vect3_scale(camera->forward, distance);
    camera->position = simple_opengl_vect3_add(camera->position, movement);
    camera->target = simple_opengl_vect3_add(camera->position, camera->forward);
}

void simple_opengl_camera_move_right(SimpleOpenGLCamera* camera, float distance) {
    Vect3 movement = simple_opengl_vect3_scale(camera->right, distance);
    camera->position = simple_opengl_vect3_add(camera->position, movement);
    camera->target = simple_opengl_vect3_add(camera->position, camera->forward);
}

void simple_opengl_camera_move_up(SimpleOpenGLCamera* camera, float distance) {
    Vect3 movement = simple_opengl_vect3_scale(camera->up_calculated, distance);
    camera->position = simple_opengl_vect3_add(camera->position, movement);
    camera->target = simple_opengl_vect3_add(camera->position, camera->forward);
}

// Calculate lighting for a face (ILUMINACIÓN DIFUSA CON NORMALES POR CARA)
Color calculate_simple_lighting(Color baseColor, Vect3 normal, Vect3 viewDir, Vect3 objectPosition, SimpleOpenGLLight* lights, int lightCount) {
    (void)viewDir; // Suppress unused parameter warning
    // Normalizar normal de la cara
    normal = simple_opengl_vect3_normalize(normal);
    
    // Iluminación ambiente (más alta para día soleado)
    Color ambient = {
        (int)(baseColor.r * 0.3f),
        (int)(baseColor.g * 0.3f),
        (int)(baseColor.b * 0.3f)
    };
    
    // Iluminación difusa (LEY DE LAMBERT - la luz rebota en la superficie)
    Color diffuse = {0, 0, 0};
    for (int i = 0; i < lightCount; i++) {
        Vect3 lightDir;
        if (lights[i].type == 0) { // Luz direccional (sol)
            lightDir = simple_opengl_vect3_normalize(lights[i].direction);
        } else { // Luz puntual
            lightDir = simple_opengl_vect3_subtract(lights[i].position, objectPosition);
            lightDir = simple_opengl_vect3_normalize(lightDir);
        }
        
        // Producto punto entre normal y dirección de luz (LEY DE LAMBERT)
        float diff = simple_opengl_vect3_dot(normal, lightDir);
        diff = fmax(0.0f, diff); // La luz rebota, no atraviesa (no iluminación negativa)
        
        // Aplicar intensidad de la luz y color del material (más intenso para día soleado)
        float sunnyDayMultiplier = 1.5f; // Multiplicador para día soleado
        diffuse.r += (int)(lights[i].color.r * lights[i].intensity * diff * baseColor.r / 255.0f * sunnyDayMultiplier);
        diffuse.g += (int)(lights[i].color.g * lights[i].intensity * diff * baseColor.g / 255.0f * sunnyDayMultiplier);
        diffuse.b += (int)(lights[i].color.b * lights[i].intensity * diff * baseColor.b / 255.0f * sunnyDayMultiplier);
    }
    
    // Combinar iluminación ambiente y difusa
    Color result = {
        fmin(255, ambient.r + diffuse.r),
        fmin(255, ambient.g + diffuse.g),
        fmin(255, ambient.b + diffuse.b)
    };
    
    return result;
}

// Apply fog to color (FOG VOLUMÉTRICO ESFÉRICO MEJORADO)
Color apply_simple_fog(Color originalColor, Vect3 objectPosition, Vect3 cameraPosition, SimpleOpenGLFog fog) {
    if (!fog.enabled) return originalColor;
    
    // Calcular distancia desde la cámara al objeto
    Vect3 cameraToObject = simple_opengl_vect3_subtract(objectPosition, cameraPosition);
    float distance = sqrtf(
        cameraToObject.x * cameraToObject.x + 
        cameraToObject.y * cameraToObject.y + 
        cameraToObject.z * cameraToObject.z
    );
    
    // Si el objeto está dentro del radio del fog esférico
    if (distance <= fog.radius) {
        // Calcular factor de fog (0.0 = sin fog, 1.0 = fog completo)
        float fogFactor = 1.0f - (distance / fog.radius);
        
        // Aplicar falloff (curva de transición)
        fogFactor = powf(fogFactor, fog.falloff);
        
        // Aplicar densidad
        fogFactor *= fog.density;
        
        // Limitar factor de fog
        if (fogFactor > 1.0f) fogFactor = 1.0f;
        if (fogFactor < 0.0f) fogFactor = 0.0f;
        
        // Interpolar entre color original y color del fog
        Color foggedColor = {
            (int)(originalColor.r * (1.0f - fogFactor) + fog.color.r * fogFactor),
            (int)(originalColor.g * (1.0f - fogFactor) + fog.color.g * fogFactor),
            (int)(originalColor.b * (1.0f - fogFactor) + fog.color.b * fogFactor)
        };
        
        return foggedColor;
    }
    
    return originalColor;
}

// Render cube using OpenGL 1.1
void render_simple_cube(SimpleOpenGLCube cube, SimpleOpenGLCamera camera, SimpleOpenGLLight* lights, int lightCount, SimpleOpenGLFog fog) {
    glPushMatrix();
    
    // Translate to position
    glTranslatef(cube.position.x, cube.position.y, cube.position.z);
    
    // Scale
    glScalef(cube.scale.x, cube.scale.y, cube.scale.z);
    
    // Cube faces with normals
    Vect3 faces[6][4] = {
        // Front face
        {{-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}},
        // Back face
        {{ 0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}},
        // Left face
        {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f}},
        // Right face
        {{ 0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f}},
        // Top face
        {{-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}},
        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f}}
    };
    
    Vect3 normals[6] = {
        { 0.0f,  0.0f,  1.0f}, // Front
        { 0.0f,  0.0f, -1.0f}, // Back
        {-1.0f,  0.0f,  0.0f}, // Left
        { 1.0f,  0.0f,  0.0f}, // Right
        { 0.0f,  1.0f,  0.0f}, // Top
        { 0.0f, -1.0f,  0.0f}  // Bottom
    };
    
    for (int face = 0; face < 6; face++) {
        // Calculate lighting for this face
        Vect3 faceCenter = {0, 0, 0};
        for (int i = 0; i < 4; i++) {
            faceCenter = simple_opengl_vect3_add(faceCenter, faces[face][i]);
        }
        faceCenter = simple_opengl_vect3_scale(faceCenter, 0.25f);
        faceCenter = simple_opengl_vect3_add(faceCenter, cube.position);
        
        Color litColor = calculate_simple_lighting(cube.material, normals[face], camera.forward, faceCenter, lights, lightCount);
        
        // Apply fog
        Color finalColor = apply_simple_fog(litColor, faceCenter, camera.position, fog);
        
        // Set color
        glColor3f(finalColor.r / 255.0f, finalColor.g / 255.0f, finalColor.b / 255.0f);
        
        // Draw face
        glBegin(GL_QUADS);
        glNormal3f(normals[face].x, normals[face].y, normals[face].z);
        for (int i = 0; i < 4; i++) {
            glVertex3f(faces[face][i].x, faces[face][i].y, faces[face][i].z);
        }
        glEnd();
    }
    
    glPopMatrix();
}

// Render scene
void render_simple_opengl_scene(SimpleOpenGLCamera camera, SimpleOpenGLCube* cubes, int cubeCount, 
                               SimpleOpenGLLight* lights, int lightCount, SimpleOpenGLFog fog) {
    (void)cubes; // Suppress unused parameter warning
    (void)cubeCount; // Suppress unused parameter warning
    (void)fog; // Suppress unused parameter warning
    // Clear buffers with realistic sky blue
    glClearColor(0.4f, 0.6f, 1.0f, 1.0f); // Bright sky blue
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect = (float)g_context.width / (float)g_context.height;
    gluPerspective(camera.fov, aspect, camera.nearPlane, camera.farPlane);
    
    // Set up modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Set up camera
    gluLookAt(
        camera.position.x, camera.position.y, camera.position.z,
        camera.target.x, camera.target.y, camera.target.z,
        camera.up_calculated.x, camera.up_calculated.y, camera.up_calculated.z
    );
    
    // Draw skybox FIRST (before everything else)
    if (g_skyboxInitialized) {
        // Get current matrices for skybox
        float viewMatrix[16], projMatrix[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);
        glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);
        
        // Draw skybox
        Skybox_Draw(&g_skybox, g_context.width, g_context.height, viewMatrix, projMatrix);
    }
    
    // Use shader-based lighting if available
    if (g_shadersInitialized) {
        // Note: glUseProgram is loaded dynamically in shaders.c
        // For now, skip shader usage to avoid compilation issues
        // TODO: Fix shader system integration
        
        // Set up matrices
        float modelMatrix[16], viewMatrix[16], projectionMatrix[16];
        create_model_matrix(modelMatrix, simple_opengl_vect3_create(0, 0, 0), simple_opengl_vect3_create(1, 1, 1), simple_opengl_vect3_create(0, 0, 0));
        create_view_matrix(viewMatrix, camera.position, camera.target, camera.up_calculated);
        create_projection_matrix(projectionMatrix, camera.fov, (float)g_context.width / (float)g_context.height, camera.nearPlane, camera.farPlane);
        
        set_shader_matrices(g_litUniforms, modelMatrix, viewMatrix, projectionMatrix);
        set_shader_lighting(g_litUniforms, g_shaderLighting);
        set_shader_fog(g_litUniforms, g_shaderLighting.fogDensity, g_shaderLighting.fogColor, g_shaderLighting.fogIntensity);
    } else {
        // Fallback to fixed pipeline lighting
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        
        // Set up lights
        for (int i = 0; i < lightCount; i++) {
            GLenum light = (i == 0) ? GL_LIGHT0 : GL_LIGHT1;
        
            float lightPos[4];
            if (lights[i].type == 0) { // Directional
                lightPos[0] = -lights[i].direction.x;
                lightPos[1] = -lights[i].direction.y;
                lightPos[2] = -lights[i].direction.z;
                lightPos[3] = 0.0f;
            } else { // Point
                lightPos[0] = lights[i].position.x;
                lightPos[1] = lights[i].position.y;
                lightPos[2] = lights[i].position.z;
                lightPos[3] = 1.0f;
            }
            
            float lightColor[4] = {
                lights[i].color.r / 255.0f * lights[i].intensity,
                lights[i].color.g / 255.0f * lights[i].intensity,
                lights[i].color.b / 255.0f * lights[i].intensity,
                1.0f
            };
            
            glLightfv(light, GL_POSITION, lightPos);
            glLightfv(light, GL_DIFFUSE, lightColor);
            glLightfv(light, GL_SPECULAR, lightColor);
        }
    }
    
    // Update physics before rendering
    update_physics();
    
    // Render only voxel terrain (Minecraft-style floor)
    if (g_chunkManager) {
        render_chunk_manager(g_chunkManager, camera.position, camera.target);
    }
    
    // Disable shader program
    if (g_shadersInitialized) {
        // Note: glUseProgram is loaded dynamically in shaders.c
        // For now, skip shader usage to avoid compilation issues
        // TODO: Fix shader system integration
    }
    
    // Swap buffers
    SwapBuffers(g_context.hdc);
}

// Resize OpenGL viewport
void resize_simple_opengl(int width, int height) {
    g_context.width = width;
    g_context.height = height;
    glViewport(0, 0, width, height);
}

// Cleanup OpenGL
void cleanup_simple_opengl(SimpleOpenGLContext* context) {
    // Cleanup shaders
    if (g_shadersInitialized) {
        destroy_shader_program(&g_litShader);
        destroy_shader_program(&g_fogShader);
        g_shadersInitialized = FALSE;
        printf("Shader system cleaned up\n");
    }
    
    // Cleanup skybox
    if (g_skyboxInitialized) {
        Skybox_Destroy(&g_skybox);
        g_skyboxInitialized = FALSE;
        printf("Skybox system cleaned up\n");
    }
    
    if (context->hrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(context->hrc);
    }
}

// Vector utility functions
Vect3 simple_opengl_vect3_create(float x, float y, float z) {
    Vect3 v = {x, y, z};
    return v;
}

Vect3 simple_opengl_vect3_add(Vect3 a, Vect3 b) {
    return simple_opengl_vect3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vect3 simple_opengl_vect3_subtract(Vect3 a, Vect3 b) {
    return simple_opengl_vect3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vect3 simple_opengl_vect3_scale(Vect3 v, float scalar) {
    return simple_opengl_vect3_create(v.x * scalar, v.y * scalar, v.z * scalar);
}

Vect3 simple_opengl_vect3_normalize(Vect3 v) {
    float mag = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (mag == 0) return simple_opengl_vect3_create(0, 0, 0);
    return simple_opengl_vect3_create(v.x / mag, v.y / mag, v.z / mag);
}

float simple_opengl_vect3_dot(Vect3 a, Vect3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vect3 simple_opengl_vect3_cross(Vect3 a, Vect3 b) {
    return simple_opengl_vect3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

// Global functions for external access
SimpleOpenGLCamera* get_simple_opengl_camera() { return &g_camera; }
SimpleOpenGLLight* get_simple_opengl_lights() { return g_lights; }
SimpleOpenGLCube* get_simple_opengl_cubes() { return g_cubes; }
int get_simple_opengl_cube_count() { return g_cubeCount; }
SimpleOpenGLFog* get_simple_opengl_fog() { return &g_fog; }
void* get_chunk_manager() { return g_chunkManager; }
