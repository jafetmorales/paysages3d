#include "formclouds.h"

#include "../lib_paysages/clouds.h"
#include "../lib_paysages/tools/color.h"
#include "../lib_paysages/tools/euclid.h"
#include "../lib_paysages/scenery.h"

#include "tools.h"

/**************** Previews ****************/
class PreviewCloudsCoverage:public BasePreview
{
public:
    PreviewCloudsCoverage(QWidget* parent, CloudsLayerDefinition* layer):BasePreview(parent)
    {
        _renderer = rendererCreate();
        _renderer->render_quality = 5;
        //_renderer.applyLightStatus = _applyLightStatus;

        _original_layer = layer;
        _preview_layer = cloudsLayerCreateDefinition();

        configScaling(100.0, 1000.0, 20.0, 200.0);
    }
    ~PreviewCloudsCoverage()
    {
        cloudsLayerDeleteDefinition(_preview_layer);
        rendererDelete(_renderer);
    }
protected:
    Color getColor(double x, double y)
    {
        Vector3 eye, look;

        eye.x = 0.0;
        eye.y = scaling;
        eye.z = -10.0 * scaling;
        look.x = x * 0.01 / scaling;
        look.y = -y * 0.01 / scaling - 0.3;
        look.z = 1.0;
        look = v3Normalize(look);

        return cloudsApplyLayer(_preview_layer, COLOR_BLUE, _renderer, eye, v3Add(eye, v3Scale(look, 1000.0)));
    }
    void updateData()
    {
        cloudsLayerCopyDefinition(_original_layer, _preview_layer);
    }
    static Color _applyLightStatus(Renderer*, LightStatus*, Vector3, Vector3, SurfaceMaterial)
    {
        return COLOR_WHITE;
    }

private:
    Renderer* _renderer;
    CloudsLayerDefinition* _original_layer;
    CloudsLayerDefinition* _preview_layer;
};

class PreviewCloudsColor:public BasePreview
{
public:
    PreviewCloudsColor(QWidget* parent, CloudsLayerDefinition* layer):BasePreview(parent)
    {
        LightDefinition light;

        _original_layer = layer;
        _preview_layer = cloudsLayerCreateDefinition();

        /*_lighting = lightingCreateDefinition();
        light.color = COLOR_WHITE;
        light.direction.x = -1.0;
        light.direction.y = -1.0;
        light.direction.z = 1.0;
        light.direction = v3Normalize(light.direction);
        light.filtered = 0;
        light.masked = 1;
        light.reflection = 1.0;
        lightingAddLight(&_lighting, light);
        lightingValidateDefinition(&_lighting);*/

        _renderer = rendererCreate();
        _renderer->render_quality = 8;
        /*_renderer.alterLight = _alterLight;
        _renderer.getLightStatus = _getLightStatus;*/
        _renderer->customData[0] = _preview_layer;
        //_renderer.customData[1] = &_lighting;

        configScaling(0.5, 2.0, 0.1, 2.0);
    }
protected:
    Color getColor(double x, double y)
    {
        Vector3 start, end;

        start.x = x * _preview_layer->thickness * 0.5;
        start.y = -y * _preview_layer->thickness * 0.5;
        start.z = _preview_layer->thickness * 0.5;

        end.x = x * _preview_layer->thickness * 0.5;
        end.y = -y * _preview_layer->thickness * 0.5;
        end.z = -_preview_layer->thickness * 0.5;

        return cloudsApplyLayer(_preview_layer, COLOR_BLUE, _renderer, start, end);
    }
    void updateData()
    {
        cloudsLayerCopyDefinition(_original_layer, _preview_layer);
        //noiseForceValue(_preview_layer->shape_noise, 1.0);
        _preview_layer->lower_altitude = -_preview_layer->thickness * 0.5;
        //curveClear(_preview_layer->coverage_by_altitude);
        _preview_layer->base_coverage = 1.0;
        _preview_layer->_custom_coverage = _coverageFunc;
    }
private:
    Renderer* _renderer;
    CloudsLayerDefinition* _original_layer;
    CloudsLayerDefinition* _preview_layer;
    //LightingDefinition _lighting;

    static double _coverageFunc(CloudsLayerDefinition* layer, Vector3 position)
    {
        double coverage = curveGetValue(layer->_coverage_by_altitude, position.y / layer->thickness + 0.5);
        position.y = 0.0;
        double dist = v3Norm(position);

        if (dist >= layer->thickness * 0.5)
        {
            return 0.0;
        }
        else if (dist < layer->thickness * 0.4)
        {
            return coverage;
        }
        else
        {
            double density = 1.0 - (dist - (layer->thickness * 0.4)) / (layer->thickness * 0.1);
            return (density < coverage) ? density : coverage;
        }
    }

    /*static void _alterLight(Renderer* renderer, LightDefinition* light, Vector3 location)
    {
        light->color = cloudsLayerFilterLight((CloudsLayerDefinition*)renderer->customData[0], renderer, light->color, location, v3Scale(light->direction, -1000.0), v3Scale(light->direction, -1.0));
    }

    static void _getLightStatus(Renderer* renderer, LightStatus* status, Vector3 location)
    {
        lightingGetStatus((LightingDefinition*)renderer->customData[1], renderer, location, status);
    }*/
};

/**************** Form ****************/
FormClouds::FormClouds(QWidget *parent):
    BaseFormLayer(parent)
{
    addAutoPreset(tr("Cirrus"));
    addAutoPreset(tr("Cumulus"));
    addAutoPreset(tr("Stratocumulus"));
    addAutoPreset(tr("Stratus"));

    _definition = cloudsCreateDefinition();
    _layer = cloudsLayerCreateDefinition();

    _previewCoverage = new PreviewCloudsCoverage(parent, _layer);
    _previewColor = new PreviewCloudsColor(parent, _layer);
    addPreview(_previewCoverage, tr("Layer coverage (no lighting)"));
    addPreview(_previewColor, tr("Appearance"));

    addInputEnum(tr("Clouds model"), (int*)&_layer->type, QStringList() << tr("Cirrus") << tr("Cumulus") << tr("Stratocumulus") << tr("Stratus"));
    addInputDouble(tr("Lower altitude"), &_layer->lower_altitude, -10.0, 50.0, 0.5, 5.0);
    addInputDouble(tr("Layer thickness"), &_layer->thickness, 0.0, 20.0, 0.1, 1.0);
    addInputDouble(tr("Max coverage"), &_layer->base_coverage, 0.0, 1.0, 0.01, 0.1);
//    addInputCurve(tr("Coverage by altitude"), _layer->_coverage_by_altitude, 0.0, 1.0, 0.0, 1.0, tr("Altitude in cloud layer"), tr("Coverage value"));
//    addInputNoise(tr("Shape noise"), _layer->_shape_noise);
    addInputDouble(tr("Shape scaling"), &_layer->shape_scaling, 3.0, 30.0, 0.3, 3.0);
//    addInputNoise(tr("Edge noise"), _layer->_edge_noise);
    addInputDouble(tr("Edge scaling"), &_layer->edge_scaling, 0.5, 5.0, 0.05, 0.5);
    addInputDouble(tr("Edge length"), &_layer->edge_length, 0.0, 1.0, 0.01, 0.1);
    addInputMaterial(tr("Material"), &_layer->material);
    addInputDouble(tr("Hardness to light"), &_layer->hardness, 0.0, 1.0, 0.01, 0.1);
    addInputDouble(tr("Transparency depth"), &_layer->transparencydepth, 0.0, 10.0, 0.1, 1.0);
    addInputDouble(tr("Light traversal depth"), &_layer->lighttraversal, 0.0, 10.0, 0.1, 1.0);
    addInputDouble(tr("Minimum lighting"), &_layer->minimumlight, 0.0, 1.0, 0.01, 0.1);

    setLayers(_definition.layers);
}

void FormClouds::revertConfig()
{
    sceneryGetClouds(&_definition);
    BaseFormLayer::revertConfig();
}

void FormClouds::applyConfig()
{
    BaseFormLayer::applyConfig();
    scenerySetClouds(&_definition);
}

void FormClouds::configChangeEvent()
{
    cloudsLayerValidateDefinition(_layer);
    BaseFormLayer::configChangeEvent();
}

void FormClouds::layerReadCurrentFrom(void* layer_definition)
{
    cloudsLayerCopyDefinition((CloudsLayerDefinition*)layer_definition, _layer);
}

void FormClouds::layerWriteCurrentTo(void* layer_definition)
{
    cloudsLayerCopyDefinition(_layer, (CloudsLayerDefinition*)layer_definition);
}

void FormClouds::autoPresetSelected(int preset)
{
    cloudsLayerAutoPreset(_layer, (CloudsPreset)preset);
    BaseForm::autoPresetSelected(preset);
}

