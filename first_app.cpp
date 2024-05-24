#include "first_app.hpp"

#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace lve
{
    struct alignas(16) AlignedVec3
    {
        glm::vec3 data;
    };

    // struct SimplePushConstantData
    // {
    //     glm::mat2 transform{1.0f};
    //     glm::vec2 offset;
    //     alignas(16) glm::vec3 color;
    //     // AlignedVec3 alignVec3[2];
    //     //glm::vec3 test{0.1f,0.0f,0.0f};
        
    // };

    FirstApp::FirstApp()
    {
        loadGameObjects();
        // createPipelineLayout();
        // createPipeline();
        // recreateSwapChain();
        // createCommandBuffers();
    }

    FirstApp::~FirstApp()
    {
        // vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void FirstApp::run()
    {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};

        std::cout << "maxPushConstantsSize:" << lveDevice.properties.limits.maxPushConstantsSize << std::endl;
        while(!lveWindow.shouldClose())
        {
            glfwPollEvents();
            // drawFrame();
            if( auto commandBuffer = lveRenderer.beginFrame())
            {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                // renderGameObjects(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadGameObjects()
    {
        std::vector<LveModel::Vertex> vertices {
            {{0.0f, -0.5f},{1.0f,0.0f,0.0f}},
            {{0.5f, 0.5f},{0.0f,1.0f,0.0f}},
            {{-0.5f, 0.5f},{0.0f,0.0f,1.0f}}
        };
        
        std::shared_ptr<LveModel> lveModel = std::make_shared<LveModel>(lveDevice, vertices);

        LveGameObject triangle = LveGameObject::createGameObject();
        triangle.model = lveModel;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, 0.5f};
        triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }

    // void FirstApp::createPipelineLayout()
    // {
    //     VkPushConstantRange pushConstantRange{};
    //     pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    //     pushConstantRange.offset = 0;
    //     pushConstantRange.size = sizeof(SimplePushConstantData);
        
    //     std::cout << "sizeof(SimplePushConstantData):" << sizeof(SimplePushConstantData) << std::endl;

    //     VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    //     pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //     pipelineLayoutInfo.setLayoutCount = 0;
    //     pipelineLayoutInfo.pSetLayouts = nullptr;
    //     pipelineLayoutInfo.pushConstantRangeCount = 1;
    //     pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    //     if(vkCreatePipelineLayout(lveDevice.device(), 
    //     &pipelineLayoutInfo, 
    //     nullptr, 
    //     &pipelineLayout) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("failed to create pipeline layout");
    //     }
    // }

    // void FirstApp::createPipeline()
    // {
    //     // assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
    //     assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    //     PipelineConfigInfo pipelineConfig{};
    //     LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
    //     pipelineConfig.renderPass = lveRenderer.getSwapChainRenderPass();
    //     pipelineConfig.pipelineLayout = pipelineLayout;
    //     lvePipeline = std::make_unique<LvePipeline>(lveDevice, 
    //     "engine/shader/simple_shader.vert.spv", 
    //     "engine/shader/simple_shader.frag.spv", 
    //     pipelineConfig
    //     );

    // }

    // void FirstApp::recreateSwapChain()
    // {   
    //     std::cout << "recreateSwapChain\n" << std::endl;
    //     auto extent = lveWindow.getExtent();
    //     std::cout << "extent.x :" << extent.width << " extent.y :" << extent.height << std::endl;
    //     while(extent.width == 0 || extent.height == 0)
    //     {
    //         extent = lveWindow.getExtent();
    //         glfwWaitEvents();
    //     }  

    //     vkDeviceWaitIdle(lveDevice.device());
    //     if(lveSwapChain == nullptr)
    //     {
    //         lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
    //     }
    //     else
    //     {
    //         lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
    //         if(lveSwapChain->imageCount() != commandBuffers.size())
    //         {
    //             freeCommandBuffers();
    //             createCommandBuffers();
    //         }
    //     }

    //     std::cout << "finish create lveSwapChain\n" << std::endl;
    //     createPipeline();
    // }

    // void FirstApp::createCommandBuffers()
    // {
    //     commandBuffers.resize(lveSwapChain->imageCount());
    //     VkCommandBufferAllocateInfo allocateInfo{};
    //     allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    //     allocateInfo.commandPool = lveDevice.getCommandPool();
    //     allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    //     allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    //     if(vkAllocateCommandBuffers(lveDevice.device(), &allocateInfo, commandBuffers.data())
    //     != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("failed to allocate command buffers");
    //     }

        
    // }

    // void FirstApp::freeCommandBuffers()
    // {
    //     vkFreeCommandBuffers(lveDevice.device(), 
    //     lveDevice.getCommandPool(), 
    //     static_cast<uint32_t>(commandBuffers.size()), 
    //     commandBuffers.data());

    //     commandBuffers.clear();
    // }

    // void FirstApp::recordCommandBuffer(int imageIndex)
    // {
    //     VkCommandBufferBeginInfo beginInfo{}; 
    //     beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //     if(vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("failed to begin recording command buffer");
    //     }

    //     VkRenderPassBeginInfo renderPassInfo{};
    //     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //     renderPassInfo.renderPass = lveSwapChain->getRenderPass();
    //     renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

    //     renderPassInfo.renderArea.offset = {0, 0};
    //     renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();
        

    //     std::array<VkClearValue, 2> clearValues{};
    //     clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    //     clearValues[1].depthStencil = {1.0f, 0};
    //     renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    //     renderPassInfo.pClearValues = clearValues.data();

    //     vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //     VkViewport viewport{};
    //     viewport.x = 0.0f;
    //     viewport.y = 0.0f;
    //     viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
    //     viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
    //     viewport.minDepth = 0.0f;
    //     viewport.maxDepth = 1.0f;
    //     VkRect2D scissor{{0,0}, lveSwapChain->getSwapChainExtent()};
    //     vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
    //     vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);


    //     // lvePipeline->bind(commandBuffers[imageIndex]);
    //     // // vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
    //     // lveModel->bind(commandBuffers[imageIndex]);

    //     // for(int j = 0; j < 4; j ++){
    //     //     SimplePushConstantData push{};
    //     //     push.offset = {-0.5f + frame * 0.002f, -0.4f + j * 0.25f};
    //     //     push.color = {0.0f, 0.0f, 0.2f + 0.2f * j};
    //     //     //push.alignVec3[1].data[2] = 0.4f;
    //     //     vkCmdPushConstants(
    //     //         commandBuffers[imageIndex],
    //     //         pipelineLayout,
    //     //         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    //     //         0,
    //     //         sizeof(SimplePushConstantData),
    //     //         &push
    //     //     );
    //     //     lveModel->draw(commandBuffers[imageIndex]);
    //     // }
    //     renderGameObjects(commandBuffers[imageIndex]);

    //     vkCmdEndRenderPass(commandBuffers[imageIndex]);

    //     if(vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("failed to record command buffer");
    //     }
    // }

    // void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer)
    // {
    //     lvePipeline->bind(commandBuffer);

    //     for(auto& obj : gameObjects)
    //     {
    //         obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.001f, glm::two_pi<float>());

    //         SimplePushConstantData push{};
    //         push.offset = obj.transform2d.translation;
    //         push.color = obj.color;
    //         push.transform = obj.transform2d.mat2();

    //         vkCmdPushConstants(
    //             commandBuffer,
    //             pipelineLayout,
    //             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    //             0,
    //             sizeof(SimplePushConstantData),
    //             &push
    //         );

    //         obj.model->bind(commandBuffer);
    //         obj.model->draw(commandBuffer);
    //     }
    // }

    // void FirstApp::drawFrame()
    // {
    //     uint32_t imageIndex;
    //     auto result = lveSwapChain->acquireNextImage(&imageIndex);

    //     if(result == VK_ERROR_OUT_OF_DATE_KHR) {
    //         std::cout << "acquire next image out of date\n" << std::endl;
    //         recreateSwapChain();
    //         return;
    //     }

    //     if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    //     {
    //         throw std::runtime_error("failed to acquire next image");
    //     }

    //     recordCommandBuffer(imageIndex);
    //     result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    //     if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized())
    //     {
    //         std::cout << "submitCommandBuffers\n" << std::endl;
    //         std::cout << "imageIndex:" << imageIndex << std::endl;
    //         lveWindow.resetWindowResizedFlag();
    //         recreateSwapChain();
    //         return ;
    //     }

    //     if(result != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("failed to submit command buffer");
    //     }
    // }

    

} // namespace lve
