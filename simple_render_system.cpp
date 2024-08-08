#include "simple_render_system.hpp"
#include "lve_descriptors.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace lve {

struct SimplePushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};// 实际上这里其实应该是mat3，但是由于Vulkan的对齐要求，mat3实际上是3个vec3，很难对齐，所以用mat4方便对齐
};

SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
    : lveDevice{device} {
  createLocalSetLayouts();
  createPipelineLayout(descriptorSetLayouts);
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::createLocalSetLayouts()
{
  localSetLayouts.emplace_back( LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, 
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(2,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(3,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_SHADER_STAGE_FRAGMENT_BIT)
            .build()
  );
  localSetLayouts.emplace_back( LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, 
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_ALL_GRAPHICS)
            .build()
  );


}

void SimpleRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> &globalSetLayouts)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayouts};
    for(auto& x:localSetLayouts){
        descriptorSetLayouts.push_back(x->getDescriptorSetLayout());
    }
    std::cout << "createPipelineLayoutDescriptorLayout.size():" << descriptorSetLayouts.size() << std::endl;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "/engine/shader/texture_shader.vert.spv",
      "/engine/shader/texture_shader.frag.spv",
      pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(
    FrameInfo & frameInfo
    ) {
  lvePipeline->bind(frameInfo.commandBuffer);
  std::vector<VkDescriptorSet> descriptorSets = {frameInfo.globalDescriptorSet};
  // auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();
  vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      0,
      static_cast<uint32_t>(descriptorSets.size()),
      descriptorSets.data(),
      0,
      nullptr);

  for (auto& kv : frameInfo.gameObjects) {
    // obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
    // obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.01f, glm::two_pi<float>());
    auto &obj = kv.second;
    if (obj.model == nullptr) continue; // filter criterial

    // TODO: 传递材质描述符集,实现下面的方便的传递效果
    // std::vector<VkDescriptorSet> descriptorSets = {obj.material->descriptorSet};

    // vkCmdBindDescriptorSets(
    //   frameInfo.commandBuffer,
    //   VK_PIPELINE_BIND_POINT_GRAPHICS,
    //   pipelineLayout,
    //   1,
    //   1,
    //   &frameInfo.textureSet,
    //   0,
    //   nullptr);
    glm::mat4 openglToVulkan = glm::mat4{
                            1.0f, 0.f, 0.f, 0.f,
                            0.f, -1.0f, 0.f, 0.f,
                            0.f, 0.f, -1.0f, 0.f,
                            0.f, 0.f, 0.f, 1.0f};

    SimplePushConstantData push{};
    push.modelMatrix = obj.transform.mat4() * openglToVulkan;
    push.normalMatrix = obj.transform.normalMatrix() * glm::mat3( openglToVulkan ); // 自动把mat3->mat4 ,最后一列0 0 0 1



    vkCmdPushConstants(
        frameInfo.commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);

    // obj.model->bind(frameInfo.commandBuffer, "container.jpg");
    // obj.model->draw(frameInfo.commandBuffer);
    obj.model->drawAll(frameInfo.commandBuffer, pipelineLayout);
  }
}

}  // namespace lve