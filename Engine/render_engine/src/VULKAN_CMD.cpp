#include <VULKAN_CMD.hpp>


void VULKAN_CMD::INIT(VULKAN_PHYSICAL_DEVICE& PH_DEVICE, WINDOW& WIN, VULKAN_LOGICAL_DEVICE& LG_DEVICE, uint8_t FRAMES_IN_FLIGHT)
{
    VkPhysicalDevice vkPhysicalDevice = PH_DEVICE.GET_VK_HANDLE_TO_DEVICE();
    QueueFamilyIndices queueFamilyIndices = QUEUE::findQueueFamilies(vkPhysicalDevice, WIN.getSurface());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(LG_DEVICE.GET_HANDLE_TO_VK_LOGICAL_DEVICE(), &poolInfo, nullptr, &CMD_POOL_GRAPHICS) != VK_SUCCESS) {
        DEBUG_LOG("FAILED TO CREATE COMMAND POOL !!!");
        throw std::runtime_error("failed to create command pool!");
    } else {
        DEBUG_LOG("COMMAND POOL CREATED SUCCESSFULLY !!!");
    }

    this->CMD_BUFFERS_GRAPHICS.RE_SIZE(FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = CMD_POOL_GRAPHICS;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)this->CMD_BUFFERS_GRAPHICS.SIZE();



    if (vkAllocateCommandBuffers(LG_DEVICE.GET_HANDLE_TO_VK_LOGICAL_DEVICE(), &allocInfo, CMD_BUFFERS_GRAPHICS.DATA()) != VK_SUCCESS) {
        DEBUG_LOG("FAILED TO ALLOCATE CMD_BUFFERS !!!");
        throw std::runtime_error("failed to allocate command buffers!");
    } else {
        DEBUG_LOG("SUCCESFULLY ALLOCATED CMD_BUFFERS !");
    }
}

void VULKAN_CMD::RECORD_CMD_BUFFER_GRAPHICS(VkCommandBuffer& CMD_BUFFER, uint32_t imageIndex, VULKAN_SWAPCHAIN& SWAPCHAIN,  VULKAN_PIPELINE& PIPELINE, RE2_VK_BUFFER& VK_BUFFER)
{
    vkResetCommandBuffer(CMD_BUFFER, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    vkBeginCommandBuffer(CMD_BUFFER, &beginInfo);




    // NOTE: BARRIERS FOR RENDERING
    VkImageMemoryBarrier2 toRender{};
    toRender.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    toRender.srcStageMask     = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    toRender.srcAccessMask    = VK_ACCESS_2_NONE;
    toRender.dstStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toRender.dstAccessMask    = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toRender.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
    toRender.newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toRender.image            = SWAPCHAIN.SW_CHAIN_IMAGES[imageIndex];
    toRender.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo depInfo{};
    depInfo.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers    = &toRender;

    vkCmdPipelineBarrier2(CMD_BUFFER, &depInfo);


    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView   = SWAPCHAIN.SW_CHAIN_IMAGE_VIEWS[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue  = { .color = {{ 0.0f, 0.0f, 0.0f, 1.0f }} };

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea           = { {0, 0}, SWAPCHAIN.GET_SWAPCHAIN_RESOLUTION() };
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments    = &colorAttachment;
    renderingInfo.pDepthAttachment     = nullptr;
    renderingInfo.pStencilAttachment   = nullptr;

    vkCmdBeginRendering(CMD_BUFFER, &renderingInfo);

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float)SWAPCHAIN.GET_SWAPCHAIN_RESOLUTION().width;
    viewport.height   = (float)SWAPCHAIN.GET_SWAPCHAIN_RESOLUTION().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(CMD_BUFFER, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = SWAPCHAIN.GET_SWAPCHAIN_RESOLUTION();
    vkCmdSetScissor(CMD_BUFFER, 0, 1, &scissor);

    vkCmdBindPipeline(CMD_BUFFER, VK_PIPELINE_BIND_POINT_GRAPHICS, PIPELINE.GRAPHICS_PIPELINE);
    VkBuffer VK_BUFFERS[] = {VK_BUFFER.BUFFER[0]};
    VkDeviceSize OFFSETS[] = {0};
    vkCmdBindVertexBuffers(CMD_BUFFER, 0, 1, VK_BUFFERS, OFFSETS);
    vkCmdDraw(CMD_BUFFER, 3, 1, 0, 0);

    vkCmdEndRendering(CMD_BUFFER);


    // NOTE: BARRRIERS FOR PRESENT
    VkImageMemoryBarrier2 toPresent{};
    toPresent.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    toPresent.srcStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    toPresent.srcAccessMask    = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    toPresent.dstStageMask     = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    toPresent.dstAccessMask    = VK_ACCESS_2_NONE;
    toPresent.oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toPresent.newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    toPresent.image            = SWAPCHAIN.SW_CHAIN_IMAGES[imageIndex];
    toPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo depInfo2{};
    depInfo2.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo2.imageMemoryBarrierCount = 1;
    depInfo2.pImageMemoryBarriers    = &toPresent;

    vkCmdPipelineBarrier2(CMD_BUFFER, &depInfo2);


    vkEndCommandBuffer(CMD_BUFFER);
}

void VULKAN_CMD::FREE(VULKAN_LOGICAL_DEVICE& LG_DEVICE)
{
    vkDestroyCommandPool(LG_DEVICE.GET_HANDLE_TO_VK_LOGICAL_DEVICE(), this->CMD_POOL_GRAPHICS, nullptr);
}
