#version 450

layout(binding = 1) uniform LightModel {
    vec3 color;
    vec3 pos;
} light;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inPosition;

layout(location = 0) out vec4 outColor;

void main() {
    // ambient
    // float ambientStrength = 0.1;
    // vec3 ambient = ambientStrength * light.color;

    // // diffuse
    // vec3 norm = normalize(inNormal);
    // vec3 lightDir = normalize(light.pos - inPosition);
    // float diff = max(dot(norm, lightDir), 0.0);
    // vec3 diffuse = diff * light.color;

    // vec3 result = (ambient + diffuse) * inColor;
    // outColor = vec4(result, 1.0);

    vec3 lightPos = vec3(1.0, 1.0, 1.0);

    vec3 N = normalize(inNormal);
	vec3 L = normalize((lightPos.xyz - inPosition.xyz) - inPosition);
	vec3 V = normalize(-inPosition);
	vec3 R = reflect(-L, N);
	vec3 ambient = vec3(0.1);
	vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
	outColor = vec4((ambient + diffuse) * inColor.rgb , 1.0);
}
