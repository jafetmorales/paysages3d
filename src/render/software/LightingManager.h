#pragma once

#include "software_global.h"

#include "LightComponent.h"

#include <vector>

namespace paysages {
namespace software {

/**
 * Global lighting manager.
 *
 * This manager handles the lights, light filters and final light rendering.
 *
 * There are both static and dynamic lights.
 * A dynamic light depends on the location at which the lighting occurs.
 */
class SOFTWARESHARED_EXPORT LightingManager {
  public:
    LightingManager();

    int getStaticLightsCount() const;
    int getSourcesCount() const;
    int getFiltersCount() const;

    /**
     * Clear the static lights.
     */
    void clearStaticLights();

    /**
     * Add a static light.
     */
    void addStaticLight(const LightComponent &light);

    /**
     * Remove all registered sources.
     */
    void clearSources();

    /**
     * Register a source of dynamic lighting.
     */
    void registerSource(LightSource *source);

    /**
     * Remove a registered light source.
     */
    void unregisterSource(LightSource *source);

    /**
     * Remove all registered filters.
     */
    void clearFilters();

    /**
     * Register a filter that will receive all alterable lights.
     */
    void registerFilter(LightFilter *filter);

    /**
     * Remove a registered light filter.
     */
    void unregisterFilter(LightFilter *filter);

    /**
     * Alter the light component at a given location.
     *
     * Returns true if the light is useful
     */
    bool alterLight(LightComponent &component, const Vector3 &location);

    /**
     * Enable or disable the specularity lighting.
     */
    void setSpecularity(bool enabled);

    /**
     * Enable or disable the filtering (shadows).
     */
    void setFiltering(bool enabled);

    /**
     * Apply a final component on a surface material.
     */
    Color applyFinalComponent(const LightComponent &component, const Vector3 &eye, const Vector3 &location,
                              const Vector3 &normal, const SurfaceMaterial &material);

    /**
     * Compute the light status at a given location.
     */
    void fillStatus(LightStatus &status, const Vector3 &location) const;

    /**
     * Apply lighting to a surface at a given location.
     */
    Color apply(const Vector3 &eye, const Vector3 &location, const Vector3 &normal, const SurfaceMaterial &material);

  private:
    bool specularity;
    bool filtering;
    vector<LightComponent> static_lights;
    vector<LightFilter *> filters;
    vector<LightSource *> sources;
};
}
}
