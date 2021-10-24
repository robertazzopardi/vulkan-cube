#include "geometry/shpere/sphere.h"
#include "geometry/geometry.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <math.h>
#include <stdlib.h>

void calculateIndices(Vulkan *vulkan, uint32_t sectorCount,
                      uint32_t stackCount) {
    uint32_t k1, k2;
    for (uint32_t i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                vulkan->shapes.indices =
                    realloc(vulkan->shapes.indices,
                            (vulkan->shapes.indicesCount + 3) *
                                sizeof(*vulkan->shapes.indices));
                vulkan->shapes.indices[vulkan->shapes.indicesCount++] = k1;
                vulkan->shapes.indices[vulkan->shapes.indicesCount++] = k2;
                vulkan->shapes.indices[vulkan->shapes.indicesCount++] = k1 + 1;
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1)) {
                vulkan->shapes.indices =
                    realloc(vulkan->shapes.indices,
                            (vulkan->shapes.indicesCount + 3) *
                                sizeof(*vulkan->shapes.indices));
                vulkan->shapes.indices[vulkan->shapes.indicesCount++] = k1 + 1;
                vulkan->shapes.indices[vulkan->shapes.indicesCount++] = k2;
                vulkan->shapes.indices[vulkan->shapes.indicesCount++] = k2 + 1;
            }
        }
    }
}

void makeSphere(Vulkan *vulkan, uint32_t sectorCount, uint32_t stackCount,
                uint32_t radius) {

    uint32_t verticesCount = (2 * sectorCount) + (sectorCount * stackCount) + 1;

    allocateVerticesAndIndices(vulkan, verticesCount, 0);

    float x, y, z, xy;                           // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // vertex normal
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
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
            glm_vec3_copy(
                (vec3){x, y, z},
                vulkan->shapes.vertices[vulkan->shapes.verticesCount].pos);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            glm_vec3_copy(
                (vec3){nx, ny, nz},
                vulkan->shapes.vertices[vulkan->shapes.verticesCount].normal);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            glm_vec2_copy(
                (vec2){s, t},
                vulkan->shapes.vertices[vulkan->shapes.verticesCount].texCoord);

            glm_vec3_copy(
                (vec3)WHITE,
                vulkan->shapes.vertices[vulkan->shapes.verticesCount].colour);

            vulkan->shapes.verticesCount++;
        }
    }

    calculateIndices(vulkan, sectorCount, stackCount);
}
