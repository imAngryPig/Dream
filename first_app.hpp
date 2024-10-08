#pragma once

#include "lve_window.hpp"
// #include "lve_pipeline.hpp"
#include "lve_device.hpp"
// #include "lve_swap_chain.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "staticValue.hpp"
#include "lve_descriptors.hpp"

// std
#include <memory>
#include <vector>

namespace lve
{
class FirstApp {

    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 1060;
        

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp& operator=(const FirstApp&) = delete;

    void run();
    private:
        void loadGameObjects(LveDescriptorPool &descriptorPool, 
        std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts,
        std::vector< LveModel::Builder >& modelBuilders
        );
        // void createPipelineLayout();
        // void createPipeline();
        // void createCommandBuffers();
        // void freeCommandBuffers();
        // void drawFrame();
        // void recreateSwapChain();
        // void recordCommandBuffer(int imageIndex);
        // void renderGameObjects(VkCommandBuffer commandBuffer);

        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};
        // std::unique_ptr<LveSwapChain> lveSwapChain;
        // std::unique_ptr<LvePipeline> lvePipeline;
        // VkPipelineLayout pipelineLayout;
        // std::vector<VkCommandBuffer> commandBuffers; 

        // note: order of declarations matters
        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;

        // for test
        VkDescriptorImageInfo imageInfo;
};


} // namespace lve
