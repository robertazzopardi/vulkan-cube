#include "geometry/shpere/trisphere.h"
#include "geometry/geometry.h"
#include "vulkan_handle/memory.h"
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <string.h>
#include <vulkan_handle/vulkan_handle.h>

Icosahedron icosahedron = {
    {
        {{-X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, -Z, X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{X, 0.0, Z}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, Z, X}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, Z}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{X, 0.0, -Z}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, -Z}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, Z, -X}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{X, 0.0, Z}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, X, 0.0}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, Z, X}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{X, 0.0, -Z}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, Z, -X}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, X, 0.0}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0, Z, X}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-Z, X, 0.0}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, Z}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0, Z, X}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, X, 0.0}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, Z, -X}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, Z, -X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0, -Z, X}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, Z}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-Z, -X, 0.0}, RED, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0, -Z, -X}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{X, 0.0, -Z}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, -X, 0.0}, BLUE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0, -Z, -X}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, -X, 0.0}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, -Z, X}, GREEN, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, -Z, -X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, -Z, X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{-Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, -Z, -X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{-Z, X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, Z, X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, Z, -X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0, -Z, -X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{-X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{X, 0.0, -Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
    {
        {{X, 0.0, Z}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{0.0, -Z, X}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
        {{Z, -X, 0.0}, WHITE, GLM_VEC3_ZERO_INIT, GLM_VEC2_ZERO_INIT},
    },
};

Octahedron octahedron = {
    {
        {{0.0f, GLM_SQRT1_2, 0.0f},
         RED,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, 0.0f, GLM_SQRT1_2},
         RED,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{GLM_SQRT1_2, 0.0f, 0.0f},
         RED,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0f, -GLM_SQRT1_2, 0.0f},
         BLUE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{GLM_SQRT1_2, 0.0f, 0.0f},
         BLUE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, 0.0f, GLM_SQRT1_2},
         BLUE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0f, -GLM_SQRT1_2, 0.0f},
         GREEN,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, 0.0f, GLM_SQRT1_2},
         GREEN,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{-GLM_SQRT1_2, 0.0f, 0.0f},
         GREEN,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0f, GLM_SQRT1_2, 0.0f},
         WHITE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{-GLM_SQRT1_2, 0.0f, 0.0f},
         WHITE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, 0.0f, GLM_SQRT1_2},
         WHITE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0f, -GLM_SQRT1_2, 0.0f},
         WHITE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, 0.0f, -GLM_SQRT1_2},
         WHITE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{GLM_SQRT1_2, 0.0f, 0.0f},
         WHITE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
    },
    {
        {{GLM_SQRT1_2, 0.0f, 0.0f},
         GREEN,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, 0.0f, -GLM_SQRT1_2},
         GREEN,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, GLM_SQRT1_2, 0.0f},
         GREEN,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0f, -GLM_SQRT1_2, 0.0f},
         RED,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{-GLM_SQRT1_2, 0.0f, 0.0f},
         RED,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, 0.0f, -GLM_SQRT1_2},
         RED,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
    },
    {
        {{0.0f, GLM_SQRT1_2, 0.0f},
         BLUE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{0.0f, 0.0f, -GLM_SQRT1_2},
         BLUE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
        {{-GLM_SQRT1_2, 0.0f, 0.0f},
         BLUE,
         GLM_VEC3_ZERO_INIT,
         GLM_VEC2_ZERO_INIT},
    },
};

static inline void splitTriangle(Triangle tri, Triangle *split) {
    vec3 a, b, c;
    getMiddlePoint(tri[0].pos, tri[1].pos, a);
    getMiddlePoint(tri[1].pos, tri[2].pos, b);
    getMiddlePoint(tri[2].pos, tri[0].pos, c);

    glm_vec3_copy(tri[0].pos, split[0][0].pos);
    glm_vec3_copy(a, split[0][1].pos);
    glm_vec3_copy(c, split[0][2].pos);

    glm_vec3_copy(tri[1].pos, split[1][0].pos);
    glm_vec3_copy(b, split[1][1].pos);
    glm_vec3_copy(a, split[1][2].pos);

    glm_vec3_copy(tri[2].pos, split[2][0].pos);
    glm_vec3_copy(c, split[2][1].pos);
    glm_vec3_copy(b, split[2][2].pos);

    glm_vec3_copy(a, split[3][0].pos);
    glm_vec3_copy(b, split[3][1].pos);
    glm_vec3_copy(c, split[3][2].pos);
}

static inline void findTriangles(Triangle triangle, int currentDepth, int depth,
                                 Triangle *storage, size_t *index) {
    // Depth is reached.
    if (currentDepth == depth) {
        calculateNormals(triangle, 3);

        for (size_t i = 0; i < 3; i++) {
            glm_vec3_copy(triangle[i].normal, storage[(*index)]->normal);
            glm_vec3_copy(triangle[i].pos, storage[(*index)++]->pos);
        }
        return;
    }

    Triangle split[4];
    splitTriangle(triangle, split);

    findTriangles(split[0], currentDepth + 1, depth, storage, index);
    findTriangles(split[1], currentDepth + 1, depth, storage, index);
    findTriangles(split[2], currentDepth + 1, depth, storage, index);
    findTriangles(split[3], currentDepth + 1, depth, storage, index);
}

static inline void calculateIndicesForSphere(Shape *vulkanShape,
                                             size_t indices) {
    for (size_t i = vulkanShape->index; i < vulkanShape->index + indices; i++) {
        vulkanShape->indices[i] = i;
    }
    vulkanShape->index += indices;
}

void combineVerticesAndIndicesForSphere(Vulkan *vulkan, ShapeType shapeType,
                                        size_t depth) {

    Triangle *vertexData = NULL;
    uint32_t count = 0;

    switch (shapeType) {
    case OCTASPHERE:
        vertexData = octahedron;
        count = SIZEOF(octahedron);
        break;
    case ICOSPHERE:
        vertexData = icosahedron;
        count = SIZEOF(icosahedron);
        break;
    default:
        break;
    }

    size_t perFace = pow(4, depth);
    size_t verticesPerFace = perFace * 3;
    size_t numVertices = count * verticesPerFace;

    allocateVerticesAndIndices(vulkan, numVertices, numVertices);

    for (size_t i = 0; i < count; i++) {
        Vertex *face = vertexData[i];

        Triangle faceTriangles[verticesPerFace];

        size_t index = 0;
        findTriangles(face, 0, depth, faceTriangles, &index);

        Vertex arc[verticesPerFace];
        for (size_t j = 0; j < verticesPerFace; j++) {

            glm_vec3_copy(faceTriangles[j]->pos, arc[j].pos);

            vec3 a = {0.0f, 0.0f, 0.0f};
            normalize(a, &arc[j], 0.8);

            glm_vec3_copy(arc[j].pos, arc[j].normal);

            glm_vec3_copy(face->colour, arc[j].colour);
        }

        memcpy(vulkan->shapes.vertices + vulkan->shapes.verticesCount, arc,
               verticesPerFace * sizeof(*arc));

        vulkan->shapes.verticesCount += verticesPerFace;

        calculateIndicesForSphere(&vulkan->shapes, verticesPerFace);

        vulkan->shapes.indicesCount += verticesPerFace;
    }
}
