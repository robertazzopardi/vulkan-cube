#include "geometry/shpere/sphere.h"
#include "circle/circle.h"
#include "geometry/geometry.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <math.h>
#include <stdlib.h>

void makeSphere(Vulkan *vulkan, uint32_t sectorCount, uint32_t stackCount,
                float radius) {

    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 0.5f / radius; // vertex normal
    float s, t;                                  // vertex texCoord

    float sectorStep = 2 * GLM_PIf / sectorCount;
    float stackStep = GLM_PIf / stackCount;
    float sectorAngle, stackAngle;

    vulkan->shapes[vulkan->shapeCount].vertices =
        malloc((vulkan->shapes[vulkan->shapeCount].verticesCount +
                (sectorCount + 1) * (stackCount + 1)) *
               sizeof(*vulkan->shapes[vulkan->shapeCount].vertices));

    for (uint32_t i = 0; i <= stackCount; ++i) {
        stackAngle = GLM_PI_2f - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);         // r * cos(u)
        z = radius * sinf(stackAngle);          // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but
        // different tex coords
        for (uint32_t j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
            glm_vec3_copy(
                (vec3){x, y, z},
                vulkan->shapes[vulkan->shapeCount]
                    .vertices[vulkan->shapes[vulkan->shapeCount].verticesCount]
                    .pos);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            glm_vec3_copy(
                (vec3){nx, ny, nz},
                vulkan->shapes[vulkan->shapeCount]
                    .vertices[vulkan->shapes[vulkan->shapeCount].verticesCount]
                    .normal);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            glm_vec2_copy(
                (vec2){s, t},
                vulkan->shapes[vulkan->shapeCount]
                    .vertices[vulkan->shapes[vulkan->shapeCount].verticesCount]
                    .texCoord);

            glm_vec3_copy(
                (vec3)WHITE,
                vulkan->shapes[vulkan->shapeCount]
                    .vertices[vulkan->shapes[vulkan->shapeCount].verticesCount]
                    .colour);

            vulkan->shapes[vulkan->shapeCount].verticesCount++;
        }
    }

    calculateIndices(&vulkan->shapes[vulkan->shapeCount], sectorCount,
                     stackCount);
}
