/* Copyright (c) 2019-2020, Sascha Willems
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

hitAttributeEXT vec3 attribs;

layout(location = 0) rayPayloadInEXT vec3 hitValue;

struct Vertex {
    vec3 pos;
};

layout(binding = 3, set = 0, scalar) buffer Vertices {
    Vertex v[];
} vertices;

layout(binding = 4, set = 0) buffer Indices {
    uint i[];
} indices[];

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

  hitValue = (normal + vec3(1)) / 2;
}
