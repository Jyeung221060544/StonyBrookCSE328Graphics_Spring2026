#ifndef TORUS_H
#define TORUS_H

#include <glm/glm.hpp>
#include "shape/GLShape.h"

class Shader;

class Torus : public Renderable, public GLShape
{
public:
    Torus(
            Shader * pShader,
            const glm::vec3 & center,
            float majorRadius,
            float minorRadius,
            int tessLevel,
            const glm::vec3 & color,
            const glm::mat4 & model
    );

    ~Torus() noexcept override = default;

    void render(float timeElapsedSinceLastFrame) override;

private:
    static constexpr float kNull {0.0f};

private:
    glm::vec3 center;
    float majorRadius;
    float minorRadius;
    int tessLevel;
    glm::vec3 color;
};

#endif