#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec4 currClipPos;
layout(location = 2) in vec4 prevClipPos;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outVelocity;

void main()
{
    outColor = vec4(fragColor, 1.0);

    vec2 currNDC = currClipPos.xy / currClipPos.w;
    vec2 prevNDC = prevClipPos.xy / prevClipPos.w;

    vec2 currScreen = currNDC * 0.5 + 0.5;
    vec2 prevScreen = prevNDC * 0.5 + 0.5;

    outVelocity = currScreen - prevScreen;
}
