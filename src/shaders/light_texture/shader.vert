#version 450

layout(binding = 0) uniform ModelViewProjection {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outPosition;
layout(location = 3) out vec2 outTexCoord;

void main() {
    outPosition = vec3(mvp.model * vec4(inPosition, 1.0));
    outColor = inColor;
    outNormal = mat3(mvp.model) * inNormal;
    outTexCoord = inTexCoord;

    gl_Position = mvp.proj * mvp.view * vec4(outPosition, 1.0);
}
