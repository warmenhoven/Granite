#version 310 es
layout(location = 0) in vec2 Position;
layout(location = 0) out highp vec4 vClip;

layout(std430, push_constant) uniform Registers
{
    mat4 inverse_view_projection;
    vec3 falloff;
} registers;

void main()
{
    gl_Position = vec4(Position, 1.0, 1.0);
    vClip = registers.inverse_view_projection * vec4(Position, 0.0, 1.0);
}