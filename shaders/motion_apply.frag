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

void main()
{
    vec2 velocity = texture(velocityTexture, fragTexCoord).rg * params.motionScale;

    // Simple 4-tap motion blur reconstruction
    vec3 color = vec3(0.0);

    color += texture(inputTexture, fragTexCoord).rgb;
    color += texture(inputTexture, fragTexCoord + velocity * 0.25).rgb;
    color += texture(inputTexture, fragTexCoord + velocity * 0.50).rgb;
    color += texture(inputTexture, fragTexCoord + velocity * 0.75).rgb;

    outColor = vec4(color / 4.0, 1.0);
}
