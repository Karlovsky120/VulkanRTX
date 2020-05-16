#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( push_constant ) uniform PushConstants {
    mat4 camera;
} cameraMatrix;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    vec3 playerPosition;
    float spacer1;
    vec3 lightPosition;
    float spacer2;
    vec3 lightColor;
    float spacer3;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) flat out vec3 fragPlayerPosition;
layout(location = 3) flat out vec3 fragLightPosition;
layout(location = 4) flat out vec3 fragLightColor;

void main() {
    vec4 modelWorldPosition = ubo.model * vec4(inPosition, 1.0);

    gl_Position = cameraMatrix.camera * modelWorldPosition;
    fragPosition = modelWorldPosition.xyz;
    fragNormal = mat3(ubo.model) * inNormal;

    fragPlayerPosition = ubo.playerPosition;
    fragLightPosition = ubo.lightPosition;
    fragLightColor = ubo.lightColor;
}