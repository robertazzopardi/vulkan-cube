#include "geometry/ring/ring.h"
#include "geometry/circle/circle.h"
#include "geometry/geometry.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline void makePoint(Shape *shape, vec3 p, vec3 n, vec2 texCoord,
                             float *angle, float step, float radius,
                             float length) {
    p[0] = cos(glm_rad(*angle)) * radius;
    p[1] = sin(glm_rad(*angle)) * radius;
    *angle += step;

    glm_vec3_copy(p, shape->vertices[shape->verticesCount].pos);

    // normalized vertex normal (nx, ny, nz)
    glm_vec3_scale(p, length, n);
    glm_vec3_copy(n, shape->vertices[shape->verticesCount].normal);

    // vertex tex coord (s, t) range between [0, 1]
    glm_vec2_copy(texCoord, shape->vertices[shape->verticesCount].texCoord);

    glm_vec3_copy((vec3)WHITE, shape->vertices[shape->verticesCount].colour);

    shape->verticesCount++;
}

void makeRing(Shape *shape, uint32_t sectorCount, float radius) {
    uint32_t stackCount = 2;

    uint32_t verticesCount = (sectorCount + 1) * 2;
    shape->vertices = malloc(verticesCount * sizeof(*shape->vertices));

    float angle = 0.0f;
    float angleStep = 180.0f / sectorCount;
    float length = 1.0f / radius;

    float outsideRadius = 1.0f;
    float insideRadius = 0.5f;

    vec3 p = GLM_VEC3_ZERO_INIT;
    vec3 n = GLM_VEC3_ZERO_INIT;

    for (uint32_t i = 0; i <= sectorCount; i++) {
        makePoint(shape, p, n, (vec2){0.0f, 0.0f}, &angle, angleStep,
                  outsideRadius, length);
        makePoint(shape, p, n, (vec2){1.0f, 1.0f}, &angle, angleStep,
                  insideRadius, length);
    }

    calculateIndices(shape, sectorCount, stackCount);
}
