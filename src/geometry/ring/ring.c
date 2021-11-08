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

    shape->vertices = malloc((10 + 1) * 2 * sizeof(*shape->vertices));

    float angle = 0;
    float angleStep = 180.0f / 10.0f;
    float lengthInv = 0.5f / radius;

    float x = 0;
    float y = 0;
    float z = 0;
    float nx, ny, nz;
    float s, t;
    float outsideRadius = 2;
    float insideRadius = 1;

    for (uint32_t i = 0; i <= 10; i++) {
        float px = x + cos(glm_rad(angle)) * outsideRadius;
        float py = y + sin(glm_rad(angle)) * outsideRadius;
        angle += angleStep;
        // vertex(px, py);
        glm_vec3_copy((vec3){x, y, z},
                      shape->vertices[shape->verticesCount].pos);

        // normalized vertex normal (nx, ny, nz)
        nx = x * lengthInv;
        ny = y * lengthInv;
        nz = z * lengthInv;
        glm_vec3_copy((vec3){nx, ny, nz},
                      shape->vertices[shape->verticesCount].normal);

        // vertex tex coord (s, t) range between [0, 1]
        s = (float)1 / sectorCount;
        t = (float)i / stackCount;
        glm_vec2_copy((vec2){t, s},
                      shape->vertices[shape->verticesCount].texCoord);

        glm_vec3_copy((vec3)WHITE,
                      shape->vertices[shape->verticesCount].colour);

        shape->verticesCount++;

        //

        px = x + cos(glm_rad(angle)) * insideRadius;
        py = y + sin(glm_rad(angle)) * insideRadius;
        // vertex(px, py);
        angle += angleStep;

        glm_vec3_copy((vec3){x, y, z},
                      shape->vertices[shape->verticesCount].pos);

        // normalized vertex normal (nx, ny, nz)
        nx = x * lengthInv;
        ny = y * lengthInv;
        nz = z * lengthInv;
        glm_vec3_copy((vec3){nx, ny, nz},
                      shape->vertices[shape->verticesCount].normal);

        // vertex tex coord (s, t) range between [0, 1]
        s = (float)1 / sectorCount;
        t = (float)i / stackCount;
        glm_vec2_copy((vec2){t, s},
                      shape->vertices[shape->verticesCount].texCoord);

        glm_vec3_copy((vec3)WHITE,
                      shape->vertices[shape->verticesCount].colour);

        shape->verticesCount++;
    }

    calculateIndices(shape, sectorCount, stackCount);
}