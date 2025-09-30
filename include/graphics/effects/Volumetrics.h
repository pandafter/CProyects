#ifndef VOLUMETRICS_H
#define VOLUMETRICS_H

#include <windows.h>
#include "core/math3d.h"
#include "graphics/effects/Shadow.h"

// Volumetric fog configuration
#define VOL_HALF_RES 1
#define VOL_MAX_STEPS 40
#define VOL_ANISOTROPY 0.7f
#define VOL_FOG_DENSITY 0.1f
#define VOL_FOG_MAX_DISTANCE 100.0f

// Volumetric fog system (simplified for Windows GDI)
typedef struct VolumetricSystem {
    // Framebuffer for half-resolution fog
    unsigned int fbo;
    unsigned int fogTexture;
    unsigned int depthTexture;
    int width, height;
    int halfWidth, halfHeight;
    
    // Fog parameters
    float fogDensity;
    float fogAlbedo;
    float fogMaxDistance;
    float fogIntensity;
    float anisotropy;           // Henyey-Greenstein parameter (g)
    
    // Sun parameters
    Vect3 sunDirection;
    Color sunColor;
    float sunIntensity;
    
    // Temporal reprojection
    unsigned int prevFogTexture;
    float temporalBlend;
    BOOL enableTemporal;
    
    // Performance tracking
    int rayMarchSteps;
    float renderTime;
    BOOL enableVolumetrics;
} VolumetricSystem;

// Volumetric fog functions
VolumetricSystem* InitVolumetrics(int width, int height);
void DestroyVolumetrics(VolumetricSystem* vol);
void RenderVolumetricsPass(VolumetricSystem* vol, AdvancedShadowSystem* shadow, Vect3 cameraPos, Vect3 cameraDir, float fov);
void CompositeVolumetrics(VolumetricSystem* vol, unsigned int sceneTexture, unsigned int depthTexture);
void UpdateVolumetricParameters(VolumetricSystem* vol, Vect3 sunDir, Color sunColor, float sunIntensity);

// Ray marching functions
Color RayMarchVolumetric(VolumetricSystem* vol, AdvancedShadowSystem* shadow, Vect3 rayStart, Vect3 rayDir, float maxDistance);
float BeerLambert(float density, float distance);
float HenyeyGreenstein(float cosTheta, float g);
Vect3 GetRayDirection(Vect3 cameraPos, Vect3 cameraDir, float fov, int x, int y, int width, int height);

// Temporal reprojection
void UpdateTemporalReprojection(VolumetricSystem* vol);
void ApplyTemporalBlend(VolumetricSystem* vol);

// Runtime setters
void SetFogDensity(VolumetricSystem* vol, float density);
void SetFogIntensity(VolumetricSystem* vol, float intensity);
void SetAnisotropy(VolumetricSystem* vol, float anisotropy);
void SetVolumetricsEnabled(VolumetricSystem* vol, BOOL enabled);

// Utility functions
void CreateHalfResFramebuffer(VolumetricSystem* vol);
void BindVolumetricFramebuffer(VolumetricSystem* vol);
void UnbindVolumetricFramebuffer();

#endif // VOLUMETRICS_H
