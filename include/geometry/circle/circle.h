#ifndef INCLUDE_GEOMETRY_CIRCLE_CIRCLE
#define INCLUDE_GEOMETRY_CIRCLE_CIRCLE

typedef struct Vulkan Vulkan;
typedef unsigned int uint32_t;
typedef struct Shape Shape;

void makeCircle(Shape *, uint32_t, float);

void calculateIndices(Shape *, uint32_t, uint32_t);

#endif /* INCLUDE_GEOMETRY_CIRCLE_CIRCLE */
