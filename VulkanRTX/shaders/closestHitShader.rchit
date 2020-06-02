#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

hitAttributeEXT vec3 attribs;

layout(location = 0) rayPayloadInEXT vec3 hitValue;

struct Vertex {
    vec3 pos;
};

layout(binding = 2, set = 0) uniform UniformBuffer 
{
	mat4 viewInverse;
	mat4 projInverse;
    vec3 playerPosition;
    int padding1;
    vec3 lightPosition;
    int padding2;
} UB;

layout(binding = 3, set = 0, scalar) buffer Vertices {
    Vertex v[];
} vertices;

layout(binding = 4, set = 0) buffer Indices {
    uint i[];
} indices[];

layout( push_constant ) uniform PushConstants {
    mat4 viewInverse;
    mat4 projInverse;
    vec3 cameraPosition;
    vec3 lightPosition;
} push;

void main()
{
    ivec3 ind = ivec3(indices[gl_InstanceCustomIndexEXT].i[3 * gl_PrimitiveID + 0],
                    indices[gl_InstanceCustomIndexEXT].i[3 * gl_PrimitiveID + 1],
                    indices[gl_InstanceCustomIndexEXT].i[3 * gl_PrimitiveID + 2]);

    int length = indices[gl_InstanceCustomIndexEXT].i.length();

    vec3 v0 = vertices.v[ind.x].pos;
    vec3 v1 = vertices.v[ind.y].pos;
    vec3 v2 = vertices.v[ind.z].pos;

    vec3 first = v1 - v0;
    vec3 second = v2 - v0;
    vec3 normal = normalize(cross(first, second));

    const vec3 barycentrics = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

    vec3 hitPoint = v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
    vec3 worldPos = hitPoint + vec3(32, 32, 0);

    vec3 ambientComponent = vec3(0.0f, 0.2f, 0.0f);
    vec3 diffuseComponent = vec3(0.0f, 1.0f, 0.0f);
    vec3 specularComponent = vec3(0.0f, 1.0f, 0.0f);

    vec3 toEye = normalize(push.cameraPosition - worldPos);
    vec3 toLight = normalize(push.lightPosition - worldPos);

    float diffuseFactor = dot(normal, toLight);

    vec3 reflected = -toLight - 2 * dot(normal, -toLight) * normal;
    float specularFactor = pow(dot(toEye, reflected), 25);

    vec3 diffuse = diffuseFactor * vec3(1.0) * diffuseComponent;
    diffuse = clamp(diffuse, vec3(0.0), vec3(1.0));
    vec3 specular = specularFactor * vec3(1.0) * specularComponent;
    specular = clamp(specular, vec3(0.0), vec3(1.0));

    vec3 fullColor = ambientComponent + diffuse + specular;
    vec3 fullColorClamped = clamp(fullColor, vec3(0.0), vec3(1.0));

    hitValue = fullColorClamped;

    //hitValue = vec3(1.0);
}
