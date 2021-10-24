#ifndef INCLUDE_GEOMETRY_SHPERE_TRISPHERE
#define INCLUDE_GEOMETRY_SHPERE_TRISPHERE

#include "geometry/geometry.h"

typedef unsigned long size_t;

typedef struct Vulkan Vulkan;
typedef struct Vertex Vertex;

typedef enum ShapeType ShapeTye;

void makeTriSphere(Vulkan *, ShapeType, size_t);

#endif /* INCLUDE_GEOMETRY_SHPERE_TRISPHERE */
