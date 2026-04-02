#version 410 core

layout (quads, equal_spacing, ccw) in;

out vec3 ourNormal;
out vec3 ourFragPos;
out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 center;
uniform float majorRadius;
uniform float minorRadius;
uniform vec3 color;

const float kPi = 3.14159265358979323846f;

void main()
{
    vec4 WC = gl_in[0].gl_Position;

    float u = 2.0f * kPi * gl_TessCoord.x;
    float v = 2.0f * kPi * gl_TessCoord.y;

    float cu = cos(u);
    float su = sin(u);
    float cv = cos(v);
    float sv = sin(v);

    vec3 pos = center + vec3(
        (majorRadius + minorRadius * cv) * cu,
        minorRadius * sv,
        (majorRadius + minorRadius * cv) * su
    );

    vec3 localNormal = normalize(vec3(cv * cu, sv, cv * su));

    gl_Position = projection * view * model * vec4(pos, 1.0f);
    ourFragPos = vec3(model * vec4(pos, 1.0f));
    ourNormal = normalize(mat3(transpose(inverse(model))) * localNormal);
    ourColor = color;
}