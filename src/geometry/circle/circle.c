#include "geometry/circle/circle.h"
#include "geometry/geometry.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/mat3.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

inline void calculateIndices(Shape *shape, uint32_t sectorCount,
                             uint32_t stackCount) {
    uint32_t k1, k2;
    for (uint32_t i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                shape->indices =
                    realloc(shape->indices, (shape->indicesCount + 3) *
                                                sizeof(*shape->indices));
                shape->indices[shape->indicesCount++] = k1;
                shape->indices[shape->indicesCount++] = k2;
                shape->indices[shape->indicesCount++] = k1 + 1;
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1)) {
                shape->indices =
                    realloc(shape->indices, (shape->indicesCount + 3) *
                                                sizeof(*shape->indices));
                shape->indices[shape->indicesCount++] = k1 + 1;
                shape->indices[shape->indicesCount++] = k2;
                shape->indices[shape->indicesCount++] = k2 + 1;
            }
        }
    }
}

void makeCircle(Shape *shape, uint32_t sectorCount, float radius) {
    uint32_t stackCount = 2;

    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 0.5f / radius; // vertex normal
    float s, t;                                  // vertex texCoord

    float sectorStep = 2 * GLM_PIf / sectorCount;
    float stackStep = GLM_PIf / stackCount;
    float sectorAngle, stackAngle;

    shape->vertices =
        malloc((shape->verticesCount + (sectorCount + 1) * (stackCount + 1)) *
               sizeof(*shape->vertices));

    for (uint32_t i = 0; i <= stackCount; ++i) {
        stackAngle = GLM_PI_2f - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cos(stackAngle);          // r * cos(u)
        // z = radius * sin(stackAngle);          // r * sin(u)
        z = 0.0f;

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but
        // different tex coords
        for (uint32_t j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cos(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sin(sectorAngle); // r * cos(u) * sin(v)
            glm_vec3_copy((vec3){x, y, z},
                          shape->vertices[shape->verticesCount].pos);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            glm_vec3_copy((vec3){nx, ny, nz},
                          shape->vertices[shape->verticesCount].normal);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            glm_vec2_copy((vec2){t, s},
                          shape->vertices[shape->verticesCount].texCoord);

            glm_vec3_copy((vec3)WHITE,
                          shape->vertices[shape->verticesCount].colour);

            shape->verticesCount++;
        }
    }

    calculateIndices(shape, sectorCount, stackCount);
}
