#include "shape/Superquadric.h"
#include "util/Shader.h"

Superquadric::Superquadric(
        Shader * pShader,
        const glm::vec3 & center,
        float a,
        float b,
        float e1,
        float e2,
        const glm::vec3 & color,
        const glm::mat4 & model
)
        : GLShape(pShader, model),
          center(center),
          a(a),
          b(b),
          e1(e1),
          e2(e2),
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

void Superquadric::render(float)
{
    pShader->use();
    pShader->setMat4("model", model);
    pShader->setVec3("center", center);
    pShader->setFloat("a", a);
    pShader->setFloat("b", b);
    pShader->setFloat("e1", e1);
    pShader->setFloat("e2", e2);
    pShader->setVec3("color", color);

    glBindVertexArray(vao);

    glPatchParameteri(GL_PATCH_VERTICES, 1);
    glDrawArrays(GL_PATCHES, 0, 1);

    glBindVertexArray(0);
}