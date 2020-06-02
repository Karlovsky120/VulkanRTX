#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

hitAttributeEXT vec3 attribs;

struct Payload {
    vec3 hitValue;
    uint depth;
};

layout(location = 0) rayPayloadInEXT Payload backPayload;

layout(location = 1) rayPayloadEXT bool isShadowed;
layout(location = 2) rayPayloadEXT Payload forwardPayload; 

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
   vec3 abDir = (b-a)*goldNoise(seed.xy, seed.z);
   vec3 acDir = (c-a)*goldNoise(seed.yz, seed.x);

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

    if (dot(normal, worldPos - push.cameraPosition) > 0) {
        backPayload.hitValue = vec3(0.0f);
        return;
    }

    vec3 toLight[4];
    toLight[0] = normalize(UB.lightSpan[0] - worldPos);
    toLight[1] = normalize(UB.lightSpan[1] - worldPos);
    toLight[2] = normalize(UB.lightSpan[2] - worldPos);
    toLight[3] = normalize(UB.lightSpan[3] - worldPos);

    float lightAngle[4];
    lightAngle[0] = dot(normal, toLight[0]);
    lightAngle[1] = dot(normal, toLight[1]);
    lightAngle[2] = dot(normal, toLight[2]);
    lightAngle[3] = dot(normal, toLight[3]);

    float shadowFactor = 0;
    if (lightAngle[0] > 0
        || lightAngle[1] > 0
        || lightAngle[2] > 0
        || lightAngle[3] > 0) {

        float tMin = 0.001;
        vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
        uint flags = 
            gl_RayFlagsTerminateOnFirstHitEXT
            | gl_RayFlagsOpaqueEXT
            | gl_RayFlagsSkipClosestHitShaderEXT;

        uint obstruction = 0;
        uint shadowSampleCount = 20;
        for (uint i = 0; i < shadowSampleCount; ++i) {
            vec3 samplePoint = bilinearSample(
                UB.lightSpan[0],
                UB.lightSpan[1],
                UB.lightSpan[2],
                vec3((i+1)*worldPos.y, (i+1)*worldPos.x, (i+1)*worldPos.z));

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

        shadowFactor = 1.0 / (1 + obstruction);
        //backPayload.hitValue = vec3(0, 1.0 / (1 + obstruction), 0);

    } /*else {
        backPayload.hitValue = vec3(0.2, 0, 0);
    }*/

    float diffuseFactor = (lightAngle[0] + lightAngle[1] + lightAngle[2] + lightAngle[3]) / 4.0;

    vec3 lightBase = shadowFactor * diffuseFactor * vec3(0.0f, 1.0f, 0.0f);

    vec3 toEye = normalize(push.cameraPosition - worldPos);
    vec3 reflectedToEye = -toEye - 2 * dot(normal, -toEye) * normal;

    if (backPayload.depth == 0) {
        return;
    }

    forwardPayload.hitValue = vec3(0.0f);
    forwardPayload.depth = backPayload.depth - 1;

    traceRayEXT(
        topLevelAS,
        gl_RayFlagsOpaqueEXT,
        0xFF,
        0,
        0,
        0,
        worldPos,
        0.001,
        reflectedToEye,
        1000.0,
        2);

    backPayload.hitValue = lightBase + 0.5 * forwardPayload.hitValue;

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

    hitValue = fullColor;*/
}
