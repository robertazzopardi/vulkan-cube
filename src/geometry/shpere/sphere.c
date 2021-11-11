#include "geometry/shpere/sphere.h"
#include "circle/circle.h"
#include "geometry/geometry.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <math.h>
#include <stdlib.h>

void makeSphere(Shape *shape, uint32_t sectorCount, uint32_t stackCount,
                float radius) {

    float xy;                        // vertex position
    float lengthInv = 1.0f / radius; // vertex normal

    float sectorStep = 2 * GLM_PIf / sectorCount;
    float stackStep = GLM_PIf / stackCount;
    float sectorAngle, stackAngle;

    vec3 p = GLM_VEC3_ZERO_INIT;
    vec3 n = GLM_VEC3_ZERO_INIT;
    vec2 t = GLM_VEC2_ZERO_INIT;

    shape->vertices =
        malloc((shape->verticesCount + (sectorCount + 1) * (stackCount + 1)) *
               sizeof(*shape->vertices));

    for (uint32_t i = 0; i <= stackCount; ++i) {
        stackAngle = GLM_PI_2f - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cos(stackAngle);          // r * cos(u)
        p[2] = radius * sin(stackAngle);        // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but
        // different tex coords
        for (uint32_t j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            p[0] = xy * cos(sectorAngle); // r * cos(u) * cos(v)
            p[1] = xy * sin(sectorAngle); // r * cos(u) * sin(v)
            glm_vec3_copy(p, shape->vertices[shape->verticesCount].pos);

            // normalized vertex normal (nx, ny, nz)
            glm_vec3_scale(p, lengthInv, n);
            glm_vec3_copy(n, shape->vertices[shape->verticesCount].normal);

            // vertex tex coord (s, t) range between [0, 1]
            t[0] = (float)j / sectorCount;
            t[1] = (float)i / stackCount;
            glm_vec2_copy(t, shape->vertices[shape->verticesCount].texCoord);

            glm_vec3_copy((vec3)WHITE,
                          shape->vertices[shape->verticesCount].colour);

            shape->verticesCount++;
        }
    }

    calculateIndices(shape, sectorCount, stackCount);
}
