#ifndef CONE_H
#define CONE_H

#include <glm/glm.hpp>
#include "shape/GLShape.h"

class Shader;

class Cone : public Renderable, public GLShape
{
public:
    Cone(
            Shader * pShader,
            const glm::vec3 & center,
            float radius,
            float height,
            const glm::vec3 & color,
            const glm::mat4 & model
    );

    ~Cone() noexcept override = default;

    void render(float timeElapsedSinceLastFrame) override;

private:
    static constexpr float kNull {0.0f};

private:
    glm::vec3 center;
    float radius;
    float height;
    glm::vec3 color;
};

#endif
