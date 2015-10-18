#include "VegetationPresenceDefinition.h"

#include "Scenery.h"
#include "TerrainDefinition.h"
#include "VegetationLayerDefinition.h"
#include "VegetationModelDefinition.h"
#include "VegetationInstance.h"
#include "FloatNode.h"
#include "NoiseNode.h"
#include "NoiseGenerator.h"

VegetationPresenceDefinition::VegetationPresenceDefinition(VegetationLayerDefinition *parent):
    DefinitionNode(parent, "presence")
{
    noise = new NoiseNode(this);
    noise->setLevels(4);
    interval = new FloatNode(this, "interval", 0.1);
}

bool VegetationPresenceDefinition::collectInstances(std::vector<VegetationInstance> *result, const VegetationModelDefinition &model, double xmin, double zmin, double xmax, double zmax, bool outcomers) const
{
    bool added = 0;

    const NoiseGenerator *generator = noise->getGenerator();
    double interval_value = interval->getValue();

    double xstart = xmin - fmod(xmin, interval_value);
    double zstart = zmin - fmod(zmin, interval_value);
    for (double x = xstart; x < xmax; x += interval_value)
    {
        for (double z = zstart; z < zmax; z += interval_value)
        {
            double noise_presence = generator->get2DTotal(x * 0.1, z * 0.1);
            if (noise_presence > 0.0)
            {
                double size = 0.1 + 0.2 * fabs(generator->get2DTotal(z * 10.0, x * 10.0)) * (noise_presence * 0.5 + 0.5);
                double angle = 3.0 * generator->get2DTotal(-x * 20.0, z * 20.0);  // TODO balanced distribution
                double xoffset = fabs(generator->get2DTotal(x * 12.0, -z * 12.0));
                double zoffset = fabs(generator->get2DTotal(-x * 27.0, -z * 27.0));
                double y = getScenery()->getTerrain()->getInterpolatedHeight(x + xoffset, z + zoffset, true, true);
                result->push_back(VegetationInstance(model, Vector3(x + xoffset, y, z + zoffset), size, angle));
                added++;
            }
        }
    }

    return added > 0;
}
