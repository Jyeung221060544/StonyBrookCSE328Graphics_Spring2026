#version 410 core

layout (quads, equal_spacing, ccw) in;

out vec3 ourNormal;
out vec3 ourFragPos;
out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 center;
uniform float radius;
uniform float height;
uniform vec3 color;

const float kPi = 3.14159265358979323846f;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    float phi = 2.0f * kPi * u;

    float y = -0.5f * height + v * height;
    float r = radius * (1.0f - v);

    vec3 pos = center + vec3(r * cos(phi), y, r * sin(phi));

    // Approx normal (not perfect but acceptable)
    vec3 normal = normalize(vec3(cos(phi), radius/height, sin(phi)));

    gl_Position = projection * view * model * vec4(pos, 1.0f);
    ourFragPos = vec3(model * vec4(pos, 1.0f));
    ourNormal = normalize(mat3(transpose(inverse(model))) * normal);
    ourColor = color;
}
