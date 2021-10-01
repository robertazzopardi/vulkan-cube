#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outPosition;

void main() {
    outPosition = vec3(ubo.model * vec4(inPosition, 1.0));
    outColor = inColor;
    outNormal = inNormal;

    gl_Position = ubo.proj * ubo.view * vec4(outPosition, 1.0);
}
