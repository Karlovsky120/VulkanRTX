#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

hitAttributeEXT vec3 attribs;

layout(location = 0) rayPayloadInEXT vec3 hitValue;
layout(location = 1) rayPayloadEXT bool isShadowed;

struct Vertex {
    vec3 pos;
};

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 2, set = 0, scalar) uniform UniformBuffer 
{
	mat4 viewInverse;
	mat4 projInverse;
    vec3 lightPosition;
    vec3 lightSpan[4];
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

float PHI = 1.61803398874989484820459; // Golden Ratio   
float goldNoise(in vec2 xy, in float seed){
       return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}

float rand(vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

vec3 bilinearSample(vec3 a, vec3 b, vec3 c, vec3 seed) {
   vec3 abDir = (b-a)*goldNoise(seed.xy+seed.z, seed.z*seed.x);
   vec3 acDir = (c-a)*goldNoise(seed.yz-seed.x, seed.x-seed.y);

   return a + abDir + acDir;
}

void main()
{
    ivec3 ind = ivec3(indices[gl_InstanceCustomIndexEXT].i[3 * gl_PrimitiveID + 0],
                    indices[gl_InstanceCustomIndexEXT].i[3 * gl_PrimitiveID + 1],
                    indices[gl_InstanceCustomIndexEXT].i[3 * gl_PrimitiveID + 2]);

    vec3 v0 = vertices.v[ind.x].pos;
    vec3 v1 = vertices.v[ind.y].pos;
    vec3 v2 = vertices.v[ind.z].pos;

    vec3 first = v1 - v0;
    vec3 second = v2 - v0;
    vec3 normal = normalize(cross(first, second));

    vec3 worldPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

    vec3 toLight[4];
    toLight[0] = normalize(UB.lightSpan[0] - worldPos);
    toLight[1] = normalize(UB.lightSpan[1] - worldPos);
    toLight[2] = normalize(UB.lightSpan[2] - worldPos);
    toLight[3] = normalize(UB.lightSpan[3] - worldPos);

    if (dot(normal, toLight[0]) > 0
        || dot(normal, toLight[1]) > 0
        || dot(normal, toLight[2]) > 0
        || dot(normal, toLight[3]) > 0) {

        float tMin = 0.001;
        vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
        uint flags = 
            gl_RayFlagsTerminateOnFirstHitEXT
            | gl_RayFlagsOpaqueEXT
            | gl_RayFlagsSkipClosestHitShaderEXT;

        uint obstruction = 0;
        uint shadowSampleCount = 10;
        for (uint i = 0; i < shadowSampleCount; ++i) {
            vec3 samplePoint = bilinearSample(
                UB.lightSpan[0],
                UB.lightSpan[1],
                UB.lightSpan[2],
                vec3(cos(worldPos.y)/(i+1), cos(i)*worldPos.x, sin(i)*worldPos.z));

            //vec3 samplePoint = UB.lightSpan[0] + UB.lightSpan[1] * i + UB.lightSpan[2] * (i % 10);

            float tMax = length(samplePoint - worldPos);
            vec3 rayDir = normalize(samplePoint - worldPos);
            isShadowed = true;
            traceRayEXT(
                topLevelAS,
                flags,
                0xFF,
                0,
                0,
                1,
                origin,
                tMin,
                rayDir,
                tMax,
                1);

            if (isShadowed) {
                ++obstruction;
            }
        }

        hitValue = vec3(0, 1.0 / (1 + obstruction), 0);

    } else {
        hitValue = vec3(0.2, 0, 0);
    }

    return;

    /*vec3 ambientComponent = vec3(0.0f, 0.2f, 0.0f);
    vec3 diffuseComponent = vec3(0.0f, 1.0f, 0.0f);
    vec3 specularComponent = vec3(0.0f, 1.0f, 0.0f);

    vec3 toEye = normalize(push.cameraPosition - worldPos);

    float diffuseFactor = dot(normal, toLight);

    vec3 reflected = -toLight - 2 * dot(normal, -toLight) * normal;
    float specularFactor = pow(dot(toEye, reflected), 25);

    vec3 diffuse = diffuseFactor * vec3(1.0) * diffuseComponent;
    diffuse = clamp(diffuse, vec3(0.0), vec3(1.0));
    vec3 specular = specularFactor * vec3(1.0) * specularComponent;
    specular = clamp(specular, vec3(0.0), vec3(1.0));

    vec3 fullColor = ambientComponent + diffuse + specular;
    vec3 fullColorClamped = clamp(fullColor, vec3(0.0), vec3(1.0));

    hitValue = shadowFactor * fullColor;*/
}
