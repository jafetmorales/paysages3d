#ifndef _PAYSAGES_TERRAIN_H_
#define _PAYSAGES_TERRAIN_H_

#include "shared/types.h"
#include "modifiers.h"
#include "noise.h"
#include "lighting.h"
#include "pack.h"
#include "layers.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TERRAIN_MAX_MODIFIERS 50

typedef struct
{
    NoiseGenerator* height_noise;
    double height_factor;
    double scaling;
    Layers* canvases;
    double shadow_smoothing;

    double _max_height;
} TerrainDefinition;

void terrainSave(PackStream* stream, TerrainDefinition* definition);
void terrainLoad(PackStream* stream, TerrainDefinition* definition);

TerrainDefinition terrainCreateDefinition();
void terrainDeleteDefinition(TerrainDefinition* definition);
void terrainCopyDefinition(TerrainDefinition* source, TerrainDefinition* destination);
void terrainValidateDefinition(TerrainDefinition* definition);

Color terrainLightFilter(TerrainDefinition* definition, Renderer* renderer, Color light, Vector3 location, Vector3 light_location, Vector3 direction_to_light);
int terrainProjectRay(TerrainDefinition* definition, Renderer* renderer, Vector3 start, Vector3 direction, Vector3* hit_point, Color* hit_color);
double terrainGetHeight(TerrainDefinition* definition, double x, double z);
double terrainGetHeightNormalized(TerrainDefinition* definition, double x, double z);
Color terrainGetColor(TerrainDefinition* definition, Renderer* renderer, double x, double z, double detail);
void terrainRender(TerrainDefinition* definition, Renderer* renderer);

#ifdef __cplusplus
}
#endif

#endif
