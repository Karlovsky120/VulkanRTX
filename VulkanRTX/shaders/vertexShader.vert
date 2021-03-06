#version 460
#extension GL_ARB_separate_shader_objects : enable

layout( push_constant ) uniform PushConstants {
    mat4 camera;
    int chunkId;
} push;

layout(binding = 0) uniform UniformBufferObject {
    mat4 models[256];
    vec3 playerPosition;
    float spacer1;
    vec3 lightPosition;
    float spacer2;
    vec3 lightColor;
    float spacer3;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragPosition;
layout(location = 2) flat out vec3 fragPlayerPosition;
layout(location = 3) flat out vec3 fragLightPosition;
layout(location = 4) flat out vec3 fragLightColor;

void main() {
    vec4 modelWorldPosition = ubo.models[push.chunkId] * vec4(inPosition, 1.0);

    gl_Position = push.camera * modelWorldPosition;
    fragPosition = modelWorldPosition.xyz;

    fragPlayerPosition = ubo.playerPosition;
    fragLightPosition = ubo.lightPosition;
    fragLightColor = ubo.lightColor;
}