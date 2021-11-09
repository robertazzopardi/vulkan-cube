#include "geometry/ring/ring.h"
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

void makeRing(Shape *shape, uint32_t sectorCount, float radius) {
    uint32_t stackCount = 2;

    uint32_t verticesCount = (sectorCount + 1) * 2;
    shape->vertices = malloc(verticesCount * sizeof(*shape->vertices));

    float angle = 0;
    float angleStep = 180.0f / sectorCount;
    float lengthInv = 0.5f / radius;

    float x = 0;
    float y = 0;
    float z = 0;
    float nx, ny, nz;
    float outsideRadius = 1;
    float insideRadius = 0.5;

    for (uint32_t i = 0; i <= sectorCount; i++) {
        float px = x + cos(glm_rad(angle)) * outsideRadius;
        float py = y + sin(glm_rad(angle)) * outsideRadius;

        angle += angleStep;
        glm_vec3_copy((vec3){px, py, z},
                      shape->vertices[shape->verticesCount].pos);

        // normalized vertex normal (nx, ny, nz)
        nx = x * lengthInv;
        ny = y * lengthInv;
        nz = z * lengthInv;
        glm_vec3_copy((vec3){nx, ny, nz},
                      shape->vertices[shape->verticesCount].normal);

        // vertex tex coord (s, t) range between [0, 1]
        glm_vec2_copy((vec2){1, 1},
                      shape->vertices[shape->verticesCount].texCoord);

        glm_vec3_copy((vec3)WHITE,
                      shape->vertices[shape->verticesCount].colour);

        shape->verticesCount++;

        //

        px = x + cos(glm_rad(angle)) * insideRadius;
        py = y + sin(glm_rad(angle)) * insideRadius;
        angle += angleStep;

        glm_vec3_copy((vec3){px, py, z},
                      shape->vertices[shape->verticesCount].pos);

        // normalized vertex normal (nx, ny, nz)
        nx = x * lengthInv;
        ny = y * lengthInv;
        nz = z * lengthInv;
        glm_vec3_copy((vec3){nx, ny, nz},
                      shape->vertices[shape->verticesCount].normal);

        // vertex tex coord (s, t) range between [0, 1]
        glm_vec2_copy((vec2){0, 0},
                      shape->vertices[shape->verticesCount].texCoord);

        glm_vec3_copy((vec3)WHITE,
                      shape->vertices[shape->verticesCount].colour);

        shape->verticesCount++;
    }

    calculateIndices(shape, sectorCount, stackCount);
}
