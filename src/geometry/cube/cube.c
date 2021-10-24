#include "geometry/cube/cube.h"
#include "geometry/geometry.h"
#include "vulkan_handle/memory.h"
#include "vulkan_handle/vulkan_handle.h"
#include <cglm/vec3.h>
#include <string.h>

Cube cube = {
    {
        {{-0.5f, -0.5f, 0.5f}, WHITE, GLM_VEC3_ZERO_INIT, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, WHITE, GLM_VEC3_ZERO_INIT, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, WHITE, GLM_VEC3_ZERO_INIT, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, WHITE, GLM_VEC3_ZERO_INIT, {1.0f, 1.0f}},
    },
    {
        {{-0.5f, 0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, GREEN, GLM_VEC3_ZERO_INIT, {1.0f, 1.0f}},
    },
    {
        {{-0.5f, 0.5f, 0.5f}, RED, GLM_VEC3_ZERO_INIT, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, RED, GLM_VEC3_ZERO_INIT, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, RED, GLM_VEC3_ZERO_INIT, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, RED, GLM_VEC3_ZERO_INIT, {1.0f, 1.0f}},
    },
    {
        {{0.5f, -0.5f, 0.5f}, RED, GLM_VEC3_ZERO_INIT, {1.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, RED, GLM_VEC3_ZERO_INIT, {0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, RED, GLM_VEC3_ZERO_INIT, {0.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, RED, GLM_VEC3_ZERO_INIT, {1.0f, 1.0f}},
    },
    {
        {{0.5f, 0.5f, 0.5f}, BLUE, GLM_VEC3_ZERO_INIT, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, BLUE, GLM_VEC3_ZERO_INIT, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, BLUE, GLM_VEC3_ZERO_INIT, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, BLUE, GLM_VEC3_ZERO_INIT, {1.0f, 1.0f}},
    },
    {
        {{0.5f, -0.5f, -0.5f}, BLUE, GLM_VEC3_ZERO_INIT, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, BLUE, GLM_VEC3_ZERO_INIT, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, BLUE, GLM_VEC3_ZERO_INIT, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, BLUE, GLM_VEC3_ZERO_INIT, {1.0f, 1.0f}},
    },
};

const uint16_t indices[] = {
    0,  1,  2,  2,  3,  0,  // top
    4,  5,  6,  6,  7,  4,  // bottom
    8,  9,  10, 8,  10, 11, // right
    12, 13, 14, 12, 14, 15, // left
    16, 17, 18, 16, 18, 19, // front
    20, 21, 22, 20, 22, 23, // back
};

void makeCube(Vulkan *vulkan) {
    size_t count = SIZEOF(cube);

    allocateVerticesAndIndices(vulkan, count * 4, count * 6);

    // TODO: clean up

    for (uint32_t i = 0; i < count; i++) {
        Vertex *shape = cube[i];

        calculateNormals(shape, 4);

        memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount, shape,
               4 * sizeof(*shape));

        vulkan->shapes.verticesCount += 4;
    }

    memcpy(vulkan->shapes.indices, indices, SIZEOF(indices) * sizeof(*indices));
    vulkan->shapes.indicesCount = SIZEOF(indices);
}
