#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include "opengl_global.h"

#include "SoftwareRenderer.h"

namespace paysages {
namespace opengl {

/*!
 * \brief Scenery renderer in an OpenGL context.
 */
class OPENGLSHARED_EXPORT OpenGLRenderer: public SoftwareRenderer
{
public:
    OpenGLRenderer(Scenery* scenery);
    virtual ~OpenGLRenderer();

    inline OpenGLSkybox *getSkybox() const {return skybox;}
    inline OpenGLWater *getWater() const {return water;}
    inline OpenGLTerrain *getTerrain() const {return terrain;}

    void initialize();
    void prepareOpenGLState();
    void resize(int width, int height);
    void paint();

    /**
     * Pause the rendering process.
     *
     * This will prevent paintings and stop background tasks, until resume() is called.
     */
    void pause();
    /**
     * Resume the rendering process, put on hold by pause().
     */
    void resume();

    /**
     * Change the camera location.
     *
     * This may change the camera passed as argument (to stay above ground for example).
     */
    void setCamera(CameraDefinition *camera);
    void cameraChangeEvent(CameraDefinition* camera);

    inline OpenGLFunctions* getOpenGlFunctions() const {return functions;}
    inline OpenGLSharedState* getSharedState() const {return shared_state;}

    virtual double getPrecision(const Vector3 &location) override;
    virtual Color applyMediumTraversal(Vector3 location, Color color) override;

private:
    bool ready;
    bool paused;
    int vp_width;
    int vp_height;

    OpenGLFunctions* functions;
    OpenGLSharedState* shared_state;

    OpenGLSkybox* skybox;
    OpenGLWater* water;
    OpenGLTerrain* terrain;
};

}
}

#endif // OPENGLRENDERER_H
