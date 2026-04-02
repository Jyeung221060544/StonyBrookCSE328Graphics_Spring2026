#include "shape/Cone.h"
#include "util/Shader.h"

Cone::Cone(
        Shader * pShader,
        const glm::vec3 & center,
        float radius,
        float height,
        const glm::vec3 & color,
        const glm::mat4 & model
)
        : GLShape(pShader, model),
          center(center),
          radius(radius),
          height(height),
          color(color)
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          1,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(float),
                          reinterpret_cast<void *>(0));

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float),
                 &kNull,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Cone::render(float)
{
    pShader->use();
    pShader->setMat4("model", model);
    pShader->setVec3("center", center);
    pShader->setFloat("radius", radius);
    pShader->setFloat("height", height);
    pShader->setVec3("color", color);

    glBindVertexArray(vao);

    glPatchParameteri(GL_PATCH_VERTICES, 1);
    glDrawArrays(GL_PATCHES, 0, 1);

    glBindVertexArray(0);
}