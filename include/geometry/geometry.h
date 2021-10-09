#ifndef INCLUDE_GEOMETRY_GEOMETRY
#define INCLUDE_GEOMETRY_GEOMETRY

#include <cglm/types.h>

#define RED                                                                    \
    { 1.f, 0.f, 0.f }
#define GREEN                                                                  \
    { 0.f, 1.f, 0.f }
#define BLUE                                                                   \
    { 0.f, 0.f, 1.f }
#define WHITE                                                                  \
    { 1.f, 1.f, 1.f }
#define BLACK                                                                  \
    { 0.f, 0.f, 0.f }

#define VEC_2(num)                                                             \
    { num, num }
#define VEC_3(num)                                                             \
    { num, num, num }
#define VEC_4(num)                                                             \
    { num, num, num, num }

typedef struct Vertex {
    vec3 pos;
    vec3 colour;
    vec3 normal;
    vec2 texCoord;
} Vertex;

typedef struct VkImage_T *VkImage;
typedef struct VkDeviceMemory_T *VkDeviceMemory;
typedef struct VkImageView_T *VkImageView;

typedef struct Depth {
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
} Depth;

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef struct Shape {
    Vertex *vertices;
    uint32_t verticesCount;

    uint16_t *indices;
    uint16_t indicesCount;

    uint32_t index;
} Shape;

typedef struct VkBuffer_T *VkBuffer;

typedef struct ShapeBuffers {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
} ShapeBuffers;

typedef struct Vulkan Vulkan;

void combineVerticesAndIndices(Vulkan *, uint32_t, ...);

typedef uint64_t VkDeviceSize;
typedef uint32_t VkFlags;
typedef VkFlags VkBufferUsageFlags;
typedef VkFlags VkMemoryPropertyFlags;

void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags,
                  Vulkan *, VkBuffer *, VkDeviceMemory *);

void createVertexBuffer(Vulkan *);

void createIndexBuffer(Vulkan *);

void createDepthResources(Vulkan *);

void createFramebuffers(Vulkan *);

typedef struct VkVertexInputBindingDescription VkVertexInputBindingDescription;
typedef struct VkVertexInputAttributeDescription
    VkVertexInputAttributeDescription;

VkVertexInputBindingDescription getBindingDescription();

VkVertexInputAttributeDescription *getAttributeDescriptions();

void createBufferAndMemory(Vulkan *, VkBuffer *, VkDeviceMemory *, Vertex *,
                           uint16_t);

#endif /* INCLUDE_GEOMETRY_GEOMETRY */