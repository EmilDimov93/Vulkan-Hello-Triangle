#include <string>
#include <fstream>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <vector>

static constexpr uint32_t FRAMES_IN_FLIGHT = 2;
static constexpr VkFormat swapChainImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
static constexpr VkExtent2D swapChainExtent = {500, 500};

class Renderer
{
public:
    Renderer()
    {
        // Window
        if (!glfwInit())
            exit(1);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(swapChainExtent.width, swapChainExtent.height, "Vulkan", nullptr, nullptr);

        if (!window)
            exit(1);

        // Instance
        VkApplicationInfo s_app{};
        s_app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        s_app.pApplicationName = "VulkanRenderer";
        s_app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        s_app.apiVersion = VK_API_VERSION_1_3;

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        VkInstanceCreateInfo s_instance{};
        s_instance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        s_instance.pApplicationInfo = &s_app;
        s_instance.enabledExtensionCount = glfwExtensionCount;
        s_instance.ppEnabledExtensionNames = glfwExtensions;

        vkCreateInstance(&s_instance, nullptr, &instance);

        // Surface
        glfwCreateWindowSurface(instance, window, nullptr, &surface);

        // Device
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        physicalDevice = devices[0];

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        uint32_t i = 0;
        for (const VkQueueFamilyProperties &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueueFamilyIndex = i;
                break;
            }
            i++;
        }

        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo s_queue{};
        s_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        s_queue.queueFamilyIndex = graphicsQueueFamilyIndex;
        s_queue.queueCount = 1;
        s_queue.pQueuePriorities = &queuePriority;

        const char *swapChainExtention = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

        VkPhysicalDeviceDynamicRenderingFeatures s_dynamicRenderingFeatures{};
        s_dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        s_dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkDeviceCreateInfo s_device{};
        s_device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        s_device.pNext = &s_dynamicRenderingFeatures;
        s_device.queueCreateInfoCount = 1;
        s_device.pQueueCreateInfos = &s_queue;
        s_device.enabledExtensionCount = 1;
        s_device.ppEnabledExtensionNames = &swapChainExtention;
        s_device.pEnabledFeatures = nullptr;

        vkCreateDevice(physicalDevice, &s_device, nullptr, &device);

        vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);

        // Swapchain
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

        VkSwapchainCreateInfoKHR s_swapChain{};
        s_swapChain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        s_swapChain.surface = surface;
        s_swapChain.minImageCount = surfaceCapabilities.minImageCount;
        s_swapChain.imageFormat = swapChainImageFormat;
        s_swapChain.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        s_swapChain.imageExtent = swapChainExtent;
        s_swapChain.imageArrayLayers = 1;
        s_swapChain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        s_swapChain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        s_swapChain.queueFamilyIndexCount = 0;
        s_swapChain.pQueueFamilyIndices = nullptr;
        s_swapChain.preTransform = surfaceCapabilities.currentTransform;
        s_swapChain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        s_swapChain.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        s_swapChain.clipped = VK_FALSE;
        s_swapChain.oldSwapchain = VK_NULL_HANDLE;

        vkCreateSwapchainKHR(device, &s_swapChain, nullptr, &swapChain);

        uint32_t swapChainImageCount;
        vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
        swapChainImages.resize(swapChainImageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages.data());

        // Swapchain image views
        swapChainImageViews.resize(swapChainImageCount);
        for (size_t i = 0; i < swapChainImageCount; i++)
        {
            VkImageViewCreateInfo s_imageView{};
            s_imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            s_imageView.image = swapChainImages[i];
            s_imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            s_imageView.format = swapChainImageFormat;
            s_imageView.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            s_imageView.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            s_imageView.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            s_imageView.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            s_imageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            s_imageView.subresourceRange.baseMipLevel = 0;
            s_imageView.subresourceRange.levelCount = 1;
            s_imageView.subresourceRange.baseArrayLayer = 0;
            s_imageView.subresourceRange.layerCount = 1;
            vkCreateImageView(device, &s_imageView, nullptr, &swapChainImageViews[i]);
        }

        // Pipeline
        auto createShaderModule = [device = this->device](std::string fileName)
        {
            std::ifstream file(fileName, std::ios::binary | std::ios::ate);
            size_t fileSize = (size_t)file.tellg();
            std::vector<char> fileBuffer(fileSize);

            file.seekg(0);
            file.read(fileBuffer.data(), fileSize);
            file.close();

            VkShaderModuleCreateInfo s_shaderModule{};
            s_shaderModule.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            s_shaderModule.codeSize = fileBuffer.size();
            s_shaderModule.pCode = reinterpret_cast<const uint32_t *>(fileBuffer.data());

            VkShaderModule shaderModule;

            vkCreateShaderModule(device, &s_shaderModule, nullptr, &shaderModule);

            return shaderModule;
        };

        VkShaderModule vertexShaderModule = createShaderModule("shaders/vert.spv");
        VkShaderModule fragmentShaderModule = createShaderModule("shaders/frag.spv");

        VkPipelineShaderStageCreateInfo s_vertexShaderStage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShaderModule,
            .pName = "main"};

        VkPipelineShaderStageCreateInfo s_fragmentShaderStage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShaderModule,
            .pName = "main"};

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {s_vertexShaderStage, s_fragmentShaderStage};

        VkVertexInputBindingDescription s_bindingDescription = {
            .binding = 0,
            .stride = sizeof(float) * 3,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

        std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions;

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = 0;

        VkPipelineVertexInputStateCreateInfo s_vertexInputState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &s_bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()};

        VkPipelineInputAssemblyStateCreateInfo s_inputAssemblyState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE};

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapChainExtent.width),
            .height = static_cast<float>(swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f};

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChainExtent};

        VkPipelineViewportStateCreateInfo s_viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor};

        VkPipelineRasterizationStateCreateInfo s_rasterizationState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_FRONT_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f};

        VkPipelineMultisampleStateCreateInfo s_multisampleState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE};

        VkPipelineColorBlendAttachmentState s_colorBlendAttachmentState = {
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

        VkPipelineColorBlendStateCreateInfo s_colorBlendState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments = &s_colorBlendAttachmentState};

        VkPipelineLayoutCreateInfo s_pipelineLayout = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr};

        vkCreatePipelineLayout(device, &s_pipelineLayout, nullptr, &pipelineLayout);

        VkPipelineRenderingCreateInfo s_pipelineRendering{};
        s_pipelineRendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        s_pipelineRendering.colorAttachmentCount = 1;
        s_pipelineRendering.pColorAttachmentFormats = &swapChainImageFormat;

        VkGraphicsPipelineCreateInfo s_graphicsPipeline = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &s_pipelineRendering,
            .stageCount = 2,
            .pStages = shaderStages.data(),
            .pVertexInputState = &s_vertexInputState,
            .pInputAssemblyState = &s_inputAssemblyState,
            .pViewportState = &s_viewportState,
            .pRasterizationState = &s_rasterizationState,
            .pMultisampleState = &s_multisampleState,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &s_colorBlendState,
            .pDynamicState = nullptr,
            .layout = pipelineLayout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1};

        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &s_graphicsPipeline, nullptr, &pipeline);

        vkDestroyShaderModule(device, vertexShaderModule, nullptr);
        vkDestroyShaderModule(device, fragmentShaderModule, nullptr);

        // Command Pool
        VkCommandPoolCreateInfo s_commandPool{};
        s_commandPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        s_commandPool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        s_commandPool.queueFamilyIndex = static_cast<uint32_t>(graphicsQueueFamilyIndex);

        vkCreateCommandPool(device, &s_commandPool, nullptr, &commandPool);

        // Command Buffers
        VkCommandBufferAllocateInfo s_commandBuffer{};
        s_commandBuffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        s_commandBuffer.commandPool = commandPool;
        s_commandBuffer.commandBufferCount = 1;
        s_commandBuffer.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            vkAllocateCommandBuffers(device, &s_commandBuffer, &commandBuffers[i]);

        // Semaphores & Fences
        renderFinishedSemaphores.resize(swapChainImages.size());

        VkSemaphoreCreateInfo s_semaphore{};
        s_semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo s_fence = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT};

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            vkCreateSemaphore(device, &s_semaphore, nullptr, &imageAvailableSemaphores[i]);
            vkCreateFence(device, &s_fence, nullptr, &inFlightFences[i]);
        }

        for (size_t i = 0; i < swapChainImages.size(); i++)
            vkCreateSemaphore(device, &s_semaphore, nullptr, &renderFinishedSemaphores[i]);

        // Triangle
        std::vector<std::array<float, 3>> vertices = {
            std::array<float, 3>{{0.f, -0.5f, 0.f}},
            std::array<float, 3>{{0.5f, 0.5f, 0.f}},
            std::array<float, 3>{{-0.5f, 0.5f, 0.f}},
        };
        std::vector<uint32_t> indices = {0, 1, 2};

        auto uploadHostVisibleBuffer = [&](VkDeviceSize bufferSize, const void *srcData, VkBufferUsageFlags usage, VkBuffer *outBuffer, VkDeviceMemory *outMemory)
        {
            VkBufferCreateInfo s_buffer{};
            s_buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            s_buffer.size = bufferSize;
            s_buffer.usage = usage;
            s_buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            vkCreateBuffer(device, &s_buffer, nullptr, outBuffer);

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, *outBuffer, &memRequirements);

            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

            VkMemoryPropertyFlags requiredProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            uint32_t memoryTypeIndex = 0;
            for (uint32_t typeIndex = 0; typeIndex < memoryProperties.memoryTypeCount; typeIndex++)
            {
                bool isTypeAllowed = (memRequirements.memoryTypeBits & (1 << typeIndex)) != 0;
                bool hasRequiredProperties = (memoryProperties.memoryTypes[typeIndex].propertyFlags & requiredProperties) == requiredProperties;
                if (isTypeAllowed && hasRequiredProperties)
                {
                    memoryTypeIndex = typeIndex;
                    break;
                }
            }

            VkMemoryAllocateInfo s_alloc{};
            s_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            s_alloc.allocationSize = memRequirements.size;
            s_alloc.memoryTypeIndex = memoryTypeIndex;
            vkAllocateMemory(device, &s_alloc, nullptr, outMemory);
            vkBindBufferMemory(device, *outBuffer, *outMemory, 0);

            void *mappedData;
            vkMapMemory(device, *outMemory, 0, bufferSize, 0, &mappedData);
            memcpy(mappedData, srcData, (size_t)bufferSize);
            vkUnmapMemory(device, *outMemory);
        };

        uploadHostVisibleBuffer((sizeof(float) * 3) * vertices.size(), vertices.data(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &vertexBuffer, &vertexBufferMemory);
        uploadHostVisibleBuffer(sizeof(uint32_t) * indices.size(), indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &indexBuffer, &indexBufferMemory);
    }

    ~Renderer()
    {
        vkDeviceWaitIdle(device);

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);
        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyPipeline(device, pipeline, nullptr);

        for (VkImageView view : swapChainImageViews)
            vkDestroyImageView(device, view, nullptr);

        vkDestroySwapchainKHR(device, swapChain, nullptr);

        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void run()
    {
        uint32_t currentFrame = 0;

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

            uint32_t imageIndex;
            vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

            // Record commands
            vkResetCommandBuffer(commandBuffers[currentFrame], 0);

            VkCommandBufferBeginInfo s_begin{};
            s_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            vkBeginCommandBuffer(commandBuffers[currentFrame], &s_begin);

            VkImageMemoryBarrier s_imageMemoryBarrier{};
            s_imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            s_imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            s_imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            s_imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            s_imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            s_imageMemoryBarrier.image = swapChainImages[imageIndex];
            s_imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            s_imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            s_imageMemoryBarrier.subresourceRange.levelCount = 1;
            s_imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            s_imageMemoryBarrier.subresourceRange.layerCount = 1;
            s_imageMemoryBarrier.srcAccessMask = 0;
            s_imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &s_imageMemoryBarrier);

            VkRenderingAttachmentInfo s_colorAttachment{};
            s_colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            s_colorAttachment.imageView = swapChainImageViews[imageIndex];
            s_colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            s_colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            s_colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            s_colorAttachment.clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

            VkRenderingInfo s_rendering{};
            s_rendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
            s_rendering.renderArea.offset = {0, 0};
            s_rendering.renderArea.extent = swapChainExtent;
            s_rendering.layerCount = 1;
            s_rendering.colorAttachmentCount = 1;
            s_rendering.pColorAttachments = &s_colorAttachment;

            vkCmdBeginRendering(commandBuffers[currentFrame], &s_rendering);

            vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &vertexBuffer, &offset);
            vkCmdBindIndexBuffer(commandBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffers[currentFrame], indexCount, 1, 0, 0, 0);

            vkCmdEndRendering(commandBuffers[currentFrame]);

            s_imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            s_imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            s_imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            s_imageMemoryBarrier.dstAccessMask = 0;

            vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &s_imageMemoryBarrier);

            vkEndCommandBuffer(commandBuffers[currentFrame]);

            // Submit
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSubmitInfo s_submit{};
            s_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            s_submit.waitSemaphoreCount = 1;
            s_submit.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
            s_submit.pWaitDstStageMask = &waitStage;
            s_submit.commandBufferCount = 1;
            s_submit.pCommandBuffers = &commandBuffers[currentFrame];
            s_submit.signalSemaphoreCount = 1;
            s_submit.pSignalSemaphores = &renderFinishedSemaphores[imageIndex];

            vkResetFences(device, 1, &inFlightFences[currentFrame]);

            vkQueueSubmit(graphicsQueue, 1, &s_submit, inFlightFences[currentFrame]);

            VkPresentInfoKHR s_present{};
            s_present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            s_present.waitSemaphoreCount = 1;
            s_present.pWaitSemaphores = &renderFinishedSemaphores[imageIndex];
            s_present.swapchainCount = 1;
            s_present.pSwapchains = &swapChain;
            s_present.pImageIndices = &imageIndex;

            vkQueuePresentKHR(graphicsQueue, &s_present);

            currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
        }
    }

private:
    VkInstance instance = VK_NULL_HANDLE;

    GLFWwindow *window = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> commandBuffers;

    std::vector<VkSemaphore> renderFinishedSemaphores;

    std::array<VkSemaphore, FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    std::array<VkFence, FRAMES_IN_FLIGHT> inFlightFences;

    uint32_t graphicsQueueFamilyIndex = 0;

    // Triangle
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t vertexCount;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    uint32_t indexCount;
};

int main()
{
    Renderer r;
    r.run();
}