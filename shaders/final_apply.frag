#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D motionTexture;
layout(set = 0, binding = 1) uniform sampler2D blurTexture;

void main()
{
    vec3 motionResult = texture(motionTexture, fragTexCoord).rgb;
    vec3 blurResult = texture(blurTexture, fragTexCoord).rgb;

    float dofAmount = 0.3;
    vec3 finalColor = mix(motionResult, blurResult, dofAmount);

    // Simple tone mapping and Gamma correction
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    outColor = vec4(finalColor, 1.0);
}
