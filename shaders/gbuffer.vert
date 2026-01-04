#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 currMVP;
    mat4 prevMVP;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec4 currClipPos;
layout(location = 2) out vec4 prevClipPos;

void main()
{
    currClipPos = ubo.currMVP * vec4(inPosition, 1.0);
    prevClipPos = ubo.prevMVP * vec4(inPosition, 1.0);

    gl_Position = currClipPos;
    fragColor = inColor;
}
