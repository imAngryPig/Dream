#pragma once

#include "lve_camera.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_frame_info.hpp"
#include "lve_descriptors.hpp"
#include "lve_swap_chain.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class ShadowRenderSystem {
 public:
  ShadowRenderSystem(LveDevice &device, VkRenderPass renderPass, 
  std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,uint32_t texWidth, uint32_t texHeight,
    LveDescriptorPool& descriptorPool
  );
  ~ShadowRenderSystem();

  ShadowRenderSystem(const ShadowRenderSystem &) = delete;
  ShadowRenderSystem &operator=(const ShadowRenderSystem &) = delete;

  void renderGameObjects(FrameInfo & frameInfo);

  std::vector< std::unique_ptr<LveDescriptorSetLayout> > localSetLayouts;
  void updateShadowImage(VkImage currentframe);

 private:
  void createLocalSetLayouts(LveDescriptorPool& descriptorPool);
  void createPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
  void createPipeline(VkRenderPass renderPass);
  void createShadowImage();
  

  LveDevice &lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;
  VkDescriptorSet shadowDescriptorSet;

    VkDescriptorImageInfo imageInfo{};
    VkImage shadowImage;
    VkDeviceMemory shadowImageMemory;
    VkImageView shadowImageView;
    VkSampler shadowSampler;
    uint32_t texWidth;
    uint32_t texHeight;

    std::unique_ptr<LveDescriptorWriter> descriptorWriter;

};
}  // namespace lve