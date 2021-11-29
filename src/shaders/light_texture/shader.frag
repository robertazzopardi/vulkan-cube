#version 450

layout(binding = 1) uniform LightModel {
    vec3 color;
    vec3 pos;
} light;

layout(binding = 1) uniform sampler2D texSampler;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inPosition;
layout (location = 3) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 tex = texture(texSampler, inTexCoord);

    vec3 lightPos = vec3(10.0, 10.0, 10.0);

    vec3 N = normalize(inNormal);
	vec3 L = normalize((lightPos.xyz - inPosition.xyz) - inPosition);
	vec3 V = normalize(-inPosition);
	vec3 R = reflect(-L, N);
	vec3 ambient = vec3(0.1);
	vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);

	outColor = vec4((ambient + diffuse) * inColor.rgb , 1.0) * tex;
}
