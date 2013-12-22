#include "OpenGLSharedState.h"

OpenGLSharedState::OpenGLSharedState()
{
}

void OpenGLSharedState::apply(OpenGLShaderProgram *program)
{
    for (const auto &pair : variables)
    {
        pair.second->apply(program);
    }
}

OpenGLVariable *OpenGLSharedState::get(const std::string &name)
{
    OpenGLVariable*& var = variables[name];
    if (var == 0)
    {
        var = new OpenGLVariable(name);
    }
    return var;
}
