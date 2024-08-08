#pragma once

#include "lve_camera.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_frame_info.hpp"
#include "lve_descriptors.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class SimpleRenderSystem {
 public:
  SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(FrameInfo & frameInfo);

  std::vector< std::unique_ptr<LveDescriptorSetLayout> > localSetLayouts;
  
 private:
  void createLocalSetLayouts();
  void createPipelineLayout(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
  void createPipeline(VkRenderPass renderPass);

  LveDevice &lveDevice;

  std::unique_ptr<LvePipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;
  
  

};
}  // namespace lve