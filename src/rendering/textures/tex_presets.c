#include "private.h"

void texturesAutoPreset(TexturesDefinition* definition, TexturesPreset preset)
{
    TexturesLayerDefinition* layer;
    layersClear(definition->layers);

    if (preset == TEXTURES_PRESET_FULL)
    {
        layer = layersGetLayer(definition->layers, layersAddLayer(definition->layers, NULL));
        texturesLayerAutoPreset(layer, TEXTURES_LAYER_PRESET_ROCK);
        layersSetName(definition->layers, 0, "Ground");

        layer = layersGetLayer(definition->layers, layersAddLayer(definition->layers, NULL));
        texturesLayerAutoPreset(layer, TEXTURES_LAYER_PRESET_GRASS);
        layersSetName(definition->layers, 1, "Grass");

        layer = layersGetLayer(definition->layers, layersAddLayer(definition->layers, NULL));
        texturesLayerAutoPreset(layer, TEXTURES_LAYER_PRESET_SAND);
        layersSetName(definition->layers, 2, "Sand");

        layer = layersGetLayer(definition->layers, layersAddLayer(definition->layers, NULL));
        texturesLayerAutoPreset(layer, TEXTURES_LAYER_PRESET_SNOW);
        layersSetName(definition->layers, 3, "Snow");
    }
    else if (preset == TEXTURES_PRESET_IRELAND)
    {
        layer = layersGetLayer(definition->layers, layersAddLayer(definition->layers, NULL));
        texturesLayerAutoPreset(layer, TEXTURES_LAYER_PRESET_ROCK);
        layersSetName(definition->layers, 0, "Ground");

        layer = layersGetLayer(definition->layers, layersAddLayer(definition->layers, NULL));
        texturesLayerAutoPreset(layer, TEXTURES_LAYER_PRESET_GRASS);
        layersSetName(definition->layers, 1, "Grass");
    }
    else if (preset == TEXTURES_PRESET_ALPS)
    {
        layer = layersGetLayer(definition->layers, layersAddLayer(definition->layers, NULL));
        texturesLayerAutoPreset(layer, TEXTURES_LAYER_PRESET_ROCK);
        layersSetName(definition->layers, 0, "Ground");

        layer = layersGetLayer(definition->layers, layersAddLayer(definition->layers, NULL));
        texturesLayerAutoPreset(layer, TEXTURES_LAYER_PRESET_SNOW);
        layersSetName(definition->layers, 1, "Snow");
    }
    else if (preset == TEXTURES_PRESET_CANYON)
    {
        /* TODO */
    }
}

void texturesLayerAutoPreset(TexturesLayerDefinition* definition, TexturesLayerPreset preset)
{
    noiseRandomizeOffsets(definition->_displacement_noise);
    noiseRandomizeOffsets(definition->_detail_noise);

    zoneClear(definition->terrain_zone);

    switch (preset)
    {
        case TEXTURES_LAYER_PRESET_ROCK:
            definition->displacement_height = 0.3;
            definition->displacement_scaling = 2.0;
            definition->displacement_offset = 0.0;
            definition->material.base = colorToHSL(colorFromValues(0.6, 0.55, 0.57, 1.0));
            definition->material.reflection = 0.006;
            definition->material.shininess = 6.0;
            break;
        case TEXTURES_LAYER_PRESET_GRASS:
            zoneAddHeightRangeQuick(definition->terrain_zone, 1.0, 0.45, 0.5, 0.8, 1.0);
            zoneAddSlopeRangeQuick(definition->terrain_zone, 1.0, 0.0, 0.0, 0.05, 0.4);
            definition->displacement_height = 0.0;
            definition->displacement_scaling = 1.0;
            definition->displacement_offset = 0.0;
            definition->material.base = colorToHSL(colorFromValues(0.12, 0.19, 0.035, 1.0));
            definition->material.reflection = 0.001;
            definition->material.shininess = 4.0;
            break;
        case TEXTURES_LAYER_PRESET_SAND:
            zoneAddHeightRangeQuick(definition->terrain_zone, 1.0, 0.48, 0.49, 0.505, 0.51);
            zoneAddSlopeRangeQuick(definition->terrain_zone, 1.0, 0.0, 0.0, 0.1, 0.3);
            definition->displacement_height = 0.0;
            definition->displacement_scaling = 1.0;
            definition->displacement_offset = 0.0;
            definition->material.base = colorToHSL(colorFromValues(0.93, 0.9, 0.8, 1.0));
            definition->material.reflection = 0.008;
            definition->material.shininess = 1.0;
            break;
        case TEXTURES_LAYER_PRESET_SNOW:
            zoneAddHeightRangeQuick(definition->terrain_zone, 1.0, 0.77, 0.85, 1.0, 1.0);
            zoneAddSlopeRangeQuick(definition->terrain_zone, 1.0, 0.0, 0.0, 0.2, 1.0);
            definition->displacement_height = 0.0;
            definition->displacement_scaling = 1.0;
            definition->displacement_offset = 0.0;
            definition->material.base = colorToHSL(colorFromValues(1.0, 1.0, 1.0, 1.0));
            definition->material.reflection = 0.25;
            definition->material.shininess = 0.6;
            break;
        default:
            break;
    }

    texturesGetLayerType().callback_validate(definition);
}
