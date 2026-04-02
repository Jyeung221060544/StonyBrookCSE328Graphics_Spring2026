#ifndef SUPERQUADRIC_H
#define SUPERQUADRIC_H

#include <glm/glm.hpp>
#include "shape/GLShape.h"

class Shader;

class Superquadric : public Renderable, public GLShape
{
public:
    Superquadric(
            Shader * pShader,
            const glm::vec3 & center,
            float a,
            float b,
            float e1,
            float e2,
            const glm::vec3 & color,
            const glm::mat4 & model
    );

    ~Superquadric() noexcept override = default;

    void render(float timeElapsedSinceLastFrame) override;

private:
    static constexpr float kNull {0.0f};

private:
    glm::vec3 center;
    float a;
    float b;
    float e1;
    float e2;
    glm::vec3 color;
};

#endif