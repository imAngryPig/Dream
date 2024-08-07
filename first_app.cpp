#include "first_app.hpp"

#include "lve_camera.hpp"
#include "lve_buffer.hpp"
#include "simple_render_system.hpp"
#include "point_light_system.hpp"
#include "shadow_render_system.hpp"
#include "keyboard_movement_controller.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <cassert>
#include <chrono>


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
        // AlignedVec3 alignVec3[2];
        //glm::vec3 test{0.1f,0.0f,0.0f};
        
    // };

    
    FirstApp::FirstApp()
    {
        globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT + 100)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT + 125)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT + 500)
            .build();

        // loadGameObjects();
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
        std::vector<LveModel::Builder> modelBuilders(10);

        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, 
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_ALL_GRAPHICS)
            // .addBinding(1,
            // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            // VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        
        // auto textureSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
        //     .addBinding(0, 
        //     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        //     VK_SHADER_STAGE_FRAGMENT_BIT)
        //     .build();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout->getDescriptorSetLayout()
            // textureSetLayout->getDescriptorSetLayout()
            };
        SimpleRenderSystem simpleRenderSystem{lveDevice,
            lveRenderer.getSwapChainRenderPass(), 
            descriptorSetLayouts};
        PointLightSystem pointLightSystem{lveDevice,
            lveRenderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout()};
        ShadowRenderSystem shadowRenderSystem{lveDevice,
            lveRenderer.getSwapChainRenderPass(), 
            descriptorSetLayouts,
            WIDTH,HEIGHT,
            *globalPool};
        
        
        loadGameObjects(*globalPool, 
        simpleRenderSystem.localSetLayouts, 
        modelBuilders);

        // VkDescriptorSet textureSet;

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for( int i = 0; i < globalDescriptorSets.size(); i++ ) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                // .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        } 
        // LveDescriptorWriter(*textureSetLayout, *globalPool)
        //         .writeImage(0, &imageInfo)
        //         .build(textureSet);

        // 这里要知道，descriptorSetLayout只是相当于一个blueprint，用来告知pipeline应该接受怎样的数据，以及
        // 如何从descriptor pool中分配set内存，layout可以被多个set复用来分配相同规模的set，而不见得一个layout绑定一个set
        // descriptorSetLayout是用来描述descriptorSet和pipeline的，而descriptorSet是用来绑定pipeline的
        // 因此，一个descriptorSetLayout可以被多个pipeline复用来描述多个pipeline
        // 因此，一个descriptorSetLayout可以被多个descriptorSet复用来描述多个descriptorSet

        
        LveCamera camera{};

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{lveWindow, viewerObject};
        
        glfwSetCursorPosCallback(lveWindow.getGLFWWindow(), cameraController.mouse_callback);
        // std::cout << "maxPushConstantsSize:" << lveDevice.properties.limits.maxPushConstantsSize << std::endl;
        auto currentTime = std::chrono::high_resolution_clock::now();

        // tell GLFW to capture our mouse
        glfwSetInputMode(lveWindow.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        while(!lveWindow.shouldClose())
        {
            glfwPollEvents();
            

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            cameraController.ProcessMouseMovement();
            cameraController.moveInPlaneXZ(frameTime);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(45.0f), aspect, 0.1f, 100.0f);

            // VkEvent renderPassEvent;
            // // 创建事件
            // VkEventCreateInfo eventCreateInfo = {};
            // eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
            // if (vkCreateEvent(lveDevice.device(), &eventCreateInfo, nullptr, &renderPassEvent) != VK_SUCCESS) {
            //     std::cerr << "Failed to create event!" << std::endl;
            // }

            LveCamera lightCamera{};
            for( auto& go : gameObjects )
            {
                if(go.second.model == nullptr) {
                    lightCamera.setViewYXZ(go.second.transform.translation, go.second.transform.rotation);
                    lightCamera.setPerspectiveProjection(glm::radians(45.0f), aspect, 0.1f, 100.0f);
                    break;
                }
            }

            // drawFrame();
            if( auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();
                
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    lightCamera,
                    globalDescriptorSets[frameIndex],
                    gameObjects,
                    *globalPool
                };
                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                ubo.lightProjection = lightCamera.getProjection();
                ubo.lightView = lightCamera.getView();
                ubo.lightInverseView = lightCamera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush(); // attention: non-coherent atomic size

                // auto t = ubo.projectionView * glm::vec4{1.0f,2.0f,3.0f,1.0f};
                // std::cout << t.x << " " << t.y << " " << t.z << " " << t.w << std::endl;
                auto cmdBuf = lveDevice.beginSingleTimeCommands();
                frameInfo.commandBuffer = cmdBuf;
                // render
                lveRenderer.beginSwapChainRenderPass(cmdBuf);

                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                
                lveRenderer.endSwapChainRenderPass(cmdBuf);
                lveDevice.endSingleTimeCommands(cmdBuf);
                // // 设置事件，表示第一个 Render Pass 完成
                // vkCmdSetEvent(commandBuffer, renderPassEvent, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
                // // 等待事件，确保第一个 Render Pass 完成后再开始第二个 Render Pass
                // vkCmdWaitEvents(commandBuffer, 1,
                // &renderPassEvent, 
                // VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
                // VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                // 0, nullptr, 
                // 0, nullptr, 
                // 0, nullptr);
                
                // 假设你需要等待所有前面的指令完成
                // 创建一个内存屏障，用于等待前面的指令完成
                // VkMemoryBarrier memoryBarrier = {};
                // memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                // memoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT; // 等待所有写操作完成
                // memoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; // 允许所有读写操作

                // // 添加管线屏障
                // vkCmdPipelineBarrier(
                //     commandBuffer,
                //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // 源阶段掩码
                //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // 目标阶段掩码
                //     0,            // 依赖性标志
                //     1,            // 内存屏障数量
                //     &memoryBarrier, // 内存屏障数组
                //     0,            // 缓冲区内存屏障数量
                //     nullptr,      // 缓冲区内存屏障数组
                //     0,            // 图像内存屏障数量
                //     nullptr       // 图像内存屏障数组
                // );


                frameInfo.commandBuffer = commandBuffer;
                // lveRenderer.endFrame();
                VkImage currentFrameImage = lveRenderer.getSwapChainImage();
                shadowRenderSystem.updateShadowImage(currentFrameImage);
                // commandBuffer = lveRenderer.beginFrame();
                // frameInfo.commandBuffer = commandBuffer;

                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                shadowRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                
                
                // std::cout << "currentFrameImage:" << currentFrameImage << std::endl;
                // std::cout << "lveRenderer.getFrameIndex():" << lveRenderer.getFrameIndex() << std::endl;
                // std::cout << "lveRenderer.getImageIndex():" << lveRenderer.getImageIndex() << std::endl;
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }



    void FirstApp::loadGameObjects(LveDescriptorPool &descriptorPool, 
        std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts,
        std::vector< LveModel::Builder >& modelBuilders
        )
    {   // E:/dataAndCode/code/Vulkan_vsProject/Dream/
        // std::shared_ptr<LveModel> lveModel =
        // LveModel::createModelFromFile(lveDevice, 
        // "../../models/flat_vase.obj");
        // auto flatVase = LveGameObject::createGameObject();
        // flatVase.model = lveModel;
        // flatVase.transform.translation = {-.5f, .5f, 0.f};
        // flatVase.transform.scale = {3.f, 1.5f, 3.f};
        // gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        // lveModel = LveModel::createModelFromFile(lveDevice, 
        // "../../models/smooth_vase.obj");
        // auto smoothVase = LveGameObject::createGameObject();
        // smoothVase.model = lveModel;
        // smoothVase.transform.translation = {.5f, .5f, 0.f};
        // smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        // gameObjects.emplace(smoothVase.getId(),std::move(smoothVase));

        // lveModel = LveModel::createModelFromFile(lveDevice, 
        // "../../models/quad.obj");
        // auto floor = LveGameObject::createGameObject();
        // floor.model = lveModel;
        // floor.transform.translation = {0.f, .5f, 0.f};
        // floor.transform.scale = {3.f, 1.f, 3.f};
        // gameObjects.emplace(floor.getId(),std::move(floor));

        std::shared_ptr<LveModel> nanosuit = LveModel::createModelFromFile(lveDevice, 
        "../../models/nanosuit/nanosuit.obj", descriptorPool, descriptorSetLayouts);
        // imageInfo = lveModel->imageInfo;
        auto nanosuitObj = LveGameObject::createGameObject();
        nanosuitObj.model = nanosuit;
        nanosuitObj.transform.translation = {0.f, 0.5f, 0.8f};
        nanosuitObj.transform.scale = {.13f, .13f, .13f};
        gameObjects.emplace(nanosuitObj.getId(),std::move(nanosuitObj));

        std::shared_ptr<LveModel> marry = LveModel::createModelFromFile(lveDevice, 
        "../../models/marry/marry.obj", descriptorPool, descriptorSetLayouts);
        // imageInfo = lveModel->imageInfo;
        auto marryObj = LveGameObject::createGameObject();
        marryObj.model = marry;
        marryObj.transform.translation = {0.f, 0.5f, -1.f};
        marryObj.transform.scale = {.4f, .4f, .4f};
        gameObjects.emplace(marryObj.getId(),std::move(marryObj));

        std::shared_ptr<LveModel> room = LveModel::createModelFromFile(lveDevice, 
        "../../models/room/house_obj.obj", descriptorPool, descriptorSetLayouts);
        auto roomObj = LveGameObject::createGameObject();
        roomObj.model = room;
        roomObj.transform.translation = {0.f, 2.0f, 0.f};
        roomObj.transform.scale = {1.5F, 1.5f, 1.5f};
        gameObjects.emplace(roomObj.getId(),std::move(roomObj));

        // { // optional, 提醒注意std::move之后不可以再使用pointLight对象
        //     auto pointLight = LveGameObject::makePointLight(0.2f);
        //     gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        // }
        std::vector<glm::vec3> lightColors{
            // {1.f, .1f, .1f},
            // {.1f, .1f, 1.f},
            // {.1f, 1.f, .1f},
            // {1.f, 1.f, .1f},
            // {.1f, 1.f, 1.f},
            // {1.f, 1.f, 1.f}  //

            {1.f, 1.f, 1.f},
            // {1.f, 1.f, 1.f},
            // {1.f, 1.f, 1.f},
            // {1.f, 1.f, 1.f},
            // {1.f, 1.f, 1.f},
            // {1.f, 1.f, 1.f}
            
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = LveGameObject::makePointLight(1.0f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * 1.1f * glm::vec4(-2.1f, -1.8f, -1.f, 1.f));
            pointLight.transform.rotation = glm::vec3(-0.5f, 1.2f, 0.f);
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }

    }

    // // temporary helper function, creates a 1x1x1 cube centered at offset with an index buffer
// std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
//   LveModel::Builder modelBuilder{};
//   modelBuilder.vertices = {
//       // left face (white)
//       {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
//       {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
//       {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
//       {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

//       // right face (yellow)
//       {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
//       {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
//       {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
//       {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

//       // top face (orange, remember y axis points down)
//       {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
//       {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
//       {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
//       {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

//       // bottom face (red)
//       {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
//       {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
//       {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
//       {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

//       // nose face (blue)
//       {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
//       {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
//       {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
//       {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

//       // tail face (green)
//       {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
//       {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
//       {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
//       {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
//   };
//   for (auto& v : modelBuilder.vertices) {
//     v.position += offset;
//   }

//   modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
//                           12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

//   return std::make_unique<LveModel>(device, modelBuilder);
// }

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
