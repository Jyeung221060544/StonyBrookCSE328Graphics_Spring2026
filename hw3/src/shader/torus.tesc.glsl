#version 410

layout (vertices = 1) out;

uniform int tessLevel;

void main()
{
    float level = float(tessLevel);

    gl_TessLevelOuter[0] = level;
    gl_TessLevelOuter[1] = level;
    gl_TessLevelOuter[2] = level;
    gl_TessLevelOuter[3] = level;

    gl_TessLevelInner[0] = level;
    gl_TessLevelInner[1] = level;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}