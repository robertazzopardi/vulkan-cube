#include "geometry/circle/circle.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline void calculateIndices(Vulkan *vulkan, uint32_t sectorCount,
                                    uint32_t stackCount) {
    uint32_t k1, k2;
    for (uint32_t i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                vulkan->shapes[vulkan->shapeCount].indices = realloc(
                    vulkan->shapes[vulkan->shapeCount].indices,
                    (vulkan->shapes[vulkan->shapeCount].indicesCount + 3) *
                        sizeof(*vulkan->shapes[vulkan->shapeCount].indices));
                vulkan->shapes[vulkan->shapeCount].indices
                    [vulkan->shapes[vulkan->shapeCount].indicesCount++] = k1;
                vulkan->shapes[vulkan->shapeCount].indices
                    [vulkan->shapes[vulkan->shapeCount].indicesCount++] = k2;
                vulkan->shapes[vulkan->shapeCount]
                    .indices[vulkan->shapes[vulkan->shapeCount]
                                 .indicesCount++] = k1 + 1;
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1)) {
                vulkan->shapes[vulkan->shapeCount].indices = realloc(
                    vulkan->shapes[vulkan->shapeCount].indices,
                    (vulkan->shapes[vulkan->shapeCount].indicesCount + 3) *
                        sizeof(*vulkan->shapes[vulkan->shapeCount].indices));
                vulkan->shapes[vulkan->shapeCount]
                    .indices[vulkan->shapes[vulkan->shapeCount]
                                 .indicesCount++] = k1 + 1;
                vulkan->shapes[vulkan->shapeCount].indices
                    [vulkan->shapes[vulkan->shapeCount].indicesCount++] = k2;
                vulkan->shapes[vulkan->shapeCount]
                    .indices[vulkan->shapes[vulkan->shapeCount]
                                 .indicesCount++] = k2 + 1;
            }
        }
    }
}

void makeCircle(Vulkan *vulkan, uint32_t sectorCount, float radius) {

    uint32_t stackCount = 2;

    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 0.5f / radius; // vertex normal
    float s, t;                                  // vertex texCoord

    float sectorStep = 2 * GLM_PI / sectorCount;
    float stackStep = GLM_PI / stackCount;
    float sectorAngle, stackAngle;

    for (uint32_t i = 0; i <= stackCount; ++i) {
        stackAngle = GLM_PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);          // r * cos(u)
        z = radius * sinf(stackAngle);           // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but
        // different tex coords
        for (uint32_t j = 0; j <= sectorCount; ++j) {
            vulkan->shapes[vulkan->shapeCount].vertices = realloc(
                vulkan->shapes[vulkan->shapeCount].vertices,
                (vulkan->shapes[vulkan->shapeCount].verticesCount + 4) *
                    sizeof(*vulkan->shapes[vulkan->shapeCount].vertices));

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

    printf("%u\n", vulkan->shapes[vulkan->shapeCount].verticesCount);

    calculateIndices(vulkan, sectorCount, stackCount);
}
