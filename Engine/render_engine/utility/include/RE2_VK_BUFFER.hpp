#ifndef RE2_VK_BUFFER_H
#define RE2_VK_BUFFER_H

/*
*  Copyright (c) 2026 CortexR7
*
*  This Header file defines a vulkan buffer object.
*  Which can take input data and store it in VRAM.
*/


#include <vulkan/vulkan.h>
#include <VULKAN_LOGICAL_DEVICE.hpp>
#include <VULKAN_PHYSICAL_DEVICE.hpp>

#define DEBUG_ON                    // NOTE: Comment this macro out if u want to disable DEBUG_LOGs for RE2_VK_BUFFER
#include <ETL.hpp>


class RE2_VK_BUFFER{

    private:
    VULKAN_LOGICAL_DEVICE LGD;
    VULKAN_PHYSICAL_DEVICE PHD;


    void SETUP_ALLOCATE_MEMORY(
        VULKAN_LOGICAL_DEVICE LGD,
        VULKAN_PHYSICAL_DEVICE PHD,
        VkBufferUsageFlags USAGE,
        VkDeviceMemory& VERTEX_BUFFER_MEMORY,
        VkBuffer& VERTEX_BUFFER
    );

    void CREATE_BUFFER(                 // NOTE: creates a generic Buffer
        VkDeviceSize SIZE,
        VkBufferUsageFlags USAGE,
        VkMemoryPropertyFlags PROPS,
        VkBuffer& BUFFER,
        VkDeviceMemory& BUFFER_MEMORY
    );

    void CREATE_VERTEX_BUFFER(          // NOTE: creates a buffer with optimal memory (most likely VRAM) to store vertexes inside it.
        void *DATA,
        VkDeviceSize SIZE,
        VkBuffer& BUFFER,
        VkDeviceMemory& BUFFER_MEMORY
    );

    void WRITE_BUFFER(
        void* DATA,
        size_t SIZE,
        VkDeviceMemory& BUFFER_MEMORY
    );

    public:
    std::vector<VkDeviceMemory> BUFFER_MEMORY;
    std::vector<VkBuffer> BUFFER;

    void INIT(void* DATA, size_t SIZE, VULKAN_LOGICAL_DEVICE LGD, VULKAN_PHYSICAL_DEVICE PHD);
    void FREE();


    RE2_VK_BUFFER(VULKAN_LOGICAL_DEVICE LGD,VULKAN_PHYSICAL_DEVICE PHD, size_t SIZE, void* DATA);
    ~RE2_VK_BUFFER();

    RE2_VK_BUFFER();
};

#endif
