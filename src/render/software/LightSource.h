#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "software_global.h"

#include <vector>

namespace paysages {
namespace software {

/**
 * Source of dynamic lights.
 */
class SOFTWARESHARED_EXPORT LightSource {
  public:
    LightSource() = default;

    /**
     * Get the list of raw lights that may be applied at a given location.
     *
     * Returns true if lights were added to 'result'.
     */
    virtual bool getLightsAt(vector<LightComponent> &result, const Vector3 &location) const = 0;
};
}
}

#endif // LIGHTSOURCE_H
