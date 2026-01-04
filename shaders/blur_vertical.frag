#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D inputTexture;
layout(set = 0, binding = 1) uniform sampler2D velocityTexture;
layout(set = 0, binding = 2) uniform sampler2D depthTexture;

layout(set = 0, binding = 3) uniform PostProcessParams
{
    float blurStrength;
    float motionScale;
    vec2 texelSize;
}
params;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec3 result = texture(inputTexture, fragTexCoord).rgb * weights[0];

    // Vertical blur (along Y axis)
    for (int i = 1; i < 5; i++)
    {
        vec2 offset = vec2(0.0, params.texelSize.y * float(i) * params.blurStrength);
        result += texture(inputTexture, fragTexCoord + offset).rgb * weights[i];
        result += texture(inputTexture, fragTexCoord - offset).rgb * weights[i];
    }

    outColor = vec4(result, 1.0);
}
