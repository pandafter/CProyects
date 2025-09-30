#ifndef SHADOW_H
#define SHADOW_H

#include <windows.h>
#include "core/math3d.h"

// Shadow map configuration
#define SHADOW_RES 2048
#define SHADOW_PCF_SIZE 3
#define SHADOW_BIAS 0.005f

// Shadow map system (simplified for Windows GDI)
typedef struct AdvancedShadowSystem {
    unsigned int fbo;           // Framebuffer object (placeholder)
    unsigned int depthTexture;  // Depth texture (placeholder)
    unsigned int width, height; // Shadow map resolution
    
    // Light projection matrix (orthographic)
    float lightProjection[16];
    float lightView[16];
    float lightVP[16];          // Combined view-projection
    
    // Light properties
    Vect3 lightDirection;       // Directional light direction
    Vect3 lightPosition;        // Light position (for orthographic projection)
    float lightDistance;        // Distance from scene center
    
    // Shadow properties
    float bias;                 // Shadow bias
    int pcfSize;                // PCF filter size (3x3 or 5x5)
    BOOL enableShadows;         // Enable/disable shadows
    
    // Performance tracking
    int shadowPasses;           // Number of shadow passes rendered
    float shadowRenderTime;     // Time spent rendering shadows
} AdvancedShadowSystem;

// Shadow map functions (simplified for Windows GDI)
AdvancedShadowSystem* InitShadow(int width, int height);
void DestroyShadow(AdvancedShadowSystem* shadow);
void RenderShadowPass(AdvancedShadowSystem* shadow, Vect3 lightDir, Vect3 sceneCenter, float sceneRadius);
void UpdateShadowMatrices(AdvancedShadowSystem* shadow, Vect3 lightDir, Vect3 sceneCenter, float sceneRadius);
void BindShadowMap(AdvancedShadowSystem* shadow, unsigned int textureUnit);
void UnbindShadowMap();

// Shadow sampling functions
float SampleShadowMap(AdvancedShadowSystem* shadow, Vect3 worldPos, Vect3 lightDir);
float SampleShadowMapPCF(AdvancedShadowSystem* shadow, Vect3 worldPos, Vect3 lightDir, int pcfSize);
float CalculateShadowFactor(AdvancedShadowSystem* shadow, Vect3 worldPos, Vect3 lightDir);

// Matrix utilities
void CreateOrthographicMatrix(float* matrix, float left, float right, float bottom, float top, float near_val, float far);
void CreateLookAtMatrix(float* matrix, Vect3 eye, Vect3 target, Vect3 up);
void MultiplyMatrices(float* result, const float* a, const float* b);

// Runtime setters
void SetShadowBias(AdvancedShadowSystem* shadow, float bias);
void SetPCFSize(AdvancedShadowSystem* shadow, int size);
void SetShadowEnabled(AdvancedShadowSystem* shadow, BOOL enabled);

#endif // SHADOW_H
