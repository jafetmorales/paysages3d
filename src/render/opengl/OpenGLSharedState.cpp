#include "OpenGLSharedState.h"

OpenGLSharedState::OpenGLSharedState() {
}

paysages::opengl::OpenGLSharedState::~OpenGLSharedState()
{
    for (const auto &pair : variables) {
        delete pair.second;
    }
}

void OpenGLSharedState::apply(OpenGLShaderProgram *program, int &texture_unit) {
    for (const auto &pair : variables) {
        pair.second->apply(program, texture_unit);
    }
}

OpenGLVariable *OpenGLSharedState::get(const std::string &name) {
    OpenGLVariable *&var = variables[name];
    if (var == NULL) {
        var = new OpenGLVariable(name);
    }
    return var;
}
