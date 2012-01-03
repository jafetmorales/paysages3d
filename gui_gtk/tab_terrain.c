/* Terrain tab */

#include "common.h"
#include "lib_paysages/shared/functions.h"
#include "lib_paysages/terrain.h"
#include "lib_paysages/textures.h"

static SmallPreview* _preview;
static TerrainDefinition _definition;

static Color _cbPreviewRenderPixel(SmallPreview* preview, double x, double y, double xoffset, double yoffset, double scaling)
{
    Color result;

    result.r = result.g = result.b = terrainGetHeightNormalized(x, y);
    result.a = 1.0;
    
    /* TEMP */
    //result = terrainGetColor(x, y, 0.01);
    
    return result;
}

static void _cbEditNoiseDone(NoiseGenerator* generator)
{
    noiseCopy(generator, _definition.height_noise);
    terrainSetDefinition(_definition);
    
    /* TODO Redraw only affected by terrain */
    guiPreviewRedrawAll();
}

static void _cbEditNoise(GtkWidget* widget, gpointer data)
{
    guiNoiseEdit(texturesGetDefinition(0).bump_noise, _cbEditNoiseDone);
}

void guiTerrainInit()
{
    g_signal_connect(GET_WIDGET("terrain_noise_edit"), "clicked", G_CALLBACK(_cbEditNoise), NULL);

    _preview = guiPreviewNew(GTK_IMAGE(GET_WIDGET("terrain_preview")));
    guiPreviewConfigScaling(_preview, 0.01, 1.0, 0.05);
    guiPreviewConfigScrolling(_preview, -1000.0, 1000.0, -1000.0, 1000.0);
    guiPreviewSetViewport(_preview, 0.0, 0.0, 0.2);
    guiPreviewSetRenderer(_preview, _cbPreviewRenderPixel);
}

