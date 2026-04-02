#version 410 core

layout (quads, equal_spacing, ccw) in;

out vec3 ourNormal;
out vec3 ourFragPos;
out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 center;
uniform float a;
uniform float b;
uniform float e1;
uniform float e2;
uniform vec3 color;

const float kPi = 3.14159265358979323846f;

// signed power helper
float spow(float base, float exp)
{
    return sign(base) * pow(abs(base), exp);
}

void main()
{
    float u = mix(-kPi, kPi, gl_TessCoord.x);
    float v = mix(-kPi/2.0, kPi/2.0, gl_TessCoord.y);

    float cu = cos(u);
    float su = sin(u);
    float cv = cos(v);
    float sv = sin(v);

    float x = a * spow(cv, e1) * spow(cu, e2);
    float y = b * spow(cv, e1) * spow(su, e2);
    float z = spow(sv, e1);

    vec3 pos = center + vec3(x, y, z);

    vec3 normal = normalize(vec3(x, y, z));

    gl_Position = projection * view * model * vec4(pos, 1.0f);
    ourFragPos = vec3(model * vec4(pos, 1.0f));
    ourNormal = normalize(mat3(transpose(inverse(model))) * normal);
    ourColor = color;
}