//Temporary shader

#version 460 core
#extension GL_NV_ray_tracing : enable

layout(binding = 1, set = 0, rgba32f) uniform image2D image;

void main() 
{
    imageStore(image, ivec2(gl_LaunchIDNV.xy), vec4(1.0, 0.0, 0.0, 1.0));
}
