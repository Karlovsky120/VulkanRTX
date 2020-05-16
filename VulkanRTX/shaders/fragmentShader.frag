#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) flat in vec3 fragPlayerPosition;
layout(location = 3) flat in vec3 fragLightPosition;
layout(location = 4) flat in vec3 fragLightColor;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 ambientComponent = vec3(0.2f, 0.0f, 0.0f);
    vec3 diffuseComponent = vec3(1.0f, 0.0f, 0.0f);
    vec3 specularComponent = vec3(1.0f, 0.0f, 0.0f);

    vec3 toEye = normalize(fragPlayerPosition - fragPosition);
    vec3 toLight = normalize(fragLightPosition - fragPosition);

    vec3 dFdxPos = dFdx(fragPosition);
    vec3 dFdyPos = dFdy(fragPosition);

    vec3 normalizedNormal = normalize(cross(dFdxPos, dFdyPos));
    
    float diffuseFactor = dot(normalizedNormal, toLight);

    vec3 reflected = -toLight - 2 * dot(normalizedNormal, -toLight) * normalizedNormal;
    float specularFactor = pow(dot(toEye, reflected), 25);

    vec3 diffuse = diffuseFactor * fragLightColor * diffuseComponent;
    //vec3 diffuse = vec3(toEye, vec3(0, 0, -1)));
    diffuse = clamp(diffuse, vec3(0.0), vec3(1.0));
    vec3 specular = specularFactor * fragLightColor * specularComponent;
    specular = clamp(specular, vec3(0.0), vec3(1.0));

    vec3 fullColor = ambientComponent + diffuse + specular;

    outColor = vec4(fullColor, 1.0);
}