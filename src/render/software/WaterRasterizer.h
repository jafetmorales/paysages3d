#ifndef WATERRASTERIZER_H
#define WATERRASTERIZER_H

#include "software_global.h"

namespace paysages {
namespace software {

class WaterRasterizer
{
public:
    WaterRasterizer(SoftwareRenderer* renderer);

    void renderSurface();

private:
    SoftwareRenderer* renderer;
};

}
}

#endif // WATERRASTERIZER_H
