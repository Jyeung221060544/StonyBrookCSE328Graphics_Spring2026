#ifndef CYLINDER_H
#define CYLINDER_H

#include <glm/glm.hpp>

#include "shape/GLShape.h"

class Shader;

class Cylinder : public Renderable, public GLShape
{
public:
    Cylinder(
            Shader * pShader,
            const glm::vec3 & center,
            float radius,
            float height,
            const glm::vec3 & color,
            const glm::mat4 & model
    );

    ~Cylinder() noexcept override = default;

    void render(float timeElapsedSinceLastFrame) override;

private:
    static constexpr float kNull {0.0f};

private:
    glm::vec3 center {0.0f, 0.0f, 0.0f};
    float radius {1.0f};
    float height {2.0f};
    glm::vec3 color {0.7f, 0.7f, 0.7f};
};

#endif