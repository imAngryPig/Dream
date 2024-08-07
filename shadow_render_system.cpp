#include "shadow_render_system.hpp"
#include "lve_descriptors.hpp"
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

ShadowRenderSystem::ShadowRenderSystem(LveDevice& device, VkRenderPass renderPass, 
std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,uint32_t texWidth, uint32_t texHeight,
LveDescriptorPool& descriptorPool
)
    : lveDevice{device}, texWidth{texWidth}, texHeight{texHeight} 
{
    createShadowImage();
    createLocalSetLayouts(descriptorPool);
    createPipelineLayout(descriptorSetLayouts);
    createPipeline(renderPass);
}

ShadowRenderSystem::~ShadowRenderSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);

  vkDestroySampler(lveDevice.device(), shadowSampler, nullptr);
    vkDestroyImageView(lveDevice.device(), shadowImageView, nullptr);
    vkDestroyImage(lveDevice.device(), shadowImage, nullptr);
    vkFreeMemory(lveDevice.device(), shadowImageMemory, nullptr);
}

void ShadowRenderSystem::createLocalSetLayouts(LveDescriptorPool& descriptorPool)
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
    localSetLayouts.emplace_back( LveDescriptorSetLayout::Builder(lveDevice)
                .addBinding(0, 
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_FRAGMENT_BIT)
                .build()
    );

    LveDescriptorWriter t(*localSetLayouts[2], descriptorPool);
    descriptorWriter = std::make_unique<LveDescriptorWriter>(t);
    descriptorWriter->writeImage(0, &imageInfo).build(shadowDescriptorSet);
}

void ShadowRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> &globalSetLayouts
)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayouts};
    for(auto& x:localSetLayouts){
        descriptorSetLayouts.push_back(x->getDescriptorSetLayout());
    }
    std::cout << "shadow_PipelineLayoutDescriptorLayout.size():" << descriptorSetLayouts.size() << std::endl;

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

void ShadowRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
//   // point light render don't need model's information
//   pipelineConfig.attributeDescriptions.clear();
//   pipelineConfig.bindingDescriptions.clear();

  lvePipeline = std::make_unique<LvePipeline>(
      lveDevice,
      "/engine/shader/shadow_shader.vert.spv",
      "/engine/shader/shadow_shader.frag.spv",
      pipelineConfig);
}

void ShadowRenderSystem::createShadowImage()
{
    lveDevice.createImage(texWidth, 
        texHeight, 
        VK_FORMAT_B8G8R8A8_SRGB, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        shadowImage, 
        shadowImageMemory);

        // // 将staging buffer的数据复制到纹理图像
        // lveDevice.transitionImageLayout(shadowImage, 
        // VK_FORMAT_R8G8B8A8_SRGB, 
        // VK_IMAGE_LAYOUT_UNDEFINED, 
        // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // lveDevice.copyBufferToImage(stagingBuffer.getBuffer(), 
        // shadowImage, 
        // static_cast<uint32_t>(texWidth), 
        // static_cast<uint32_t>(texHeight),
        // 1);
        // lveDevice.transitionImageLayout(textureImage, 
        // VK_FORMAT_R8G8B8A8_SRGB, 
        // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        shadowImageView = lveDevice.createImageView(shadowImage, 
        VK_FORMAT_B8G8R8A8_SRGB, 
        VK_IMAGE_ASPECT_COLOR_BIT);


        shadowSampler = lveDevice.createTextureSampler(
        VK_FILTER_NEAREST, 
        VK_FILTER_NEAREST, 
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
        );

        // 更新描述符集
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = shadowImageView;
        imageInfo.sampler = shadowSampler;
}

void ShadowRenderSystem::updateShadowImage(VkImage currentframe)
{
    lveDevice.transitionImageLayout(currentframe, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,  // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    lveDevice.transitionImageLayout(shadowImage, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    lveDevice.copyImage2D(currentframe, 
        shadowImage, 
        texWidth, 
        texHeight);
    lveDevice.transitionImageLayout(shadowImage, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    lveDevice.transitionImageLayout(currentframe, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,  // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void ShadowRenderSystem::renderGameObjects(
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

    // descriptorWriter->overwrite(shadowDescriptorSet);
    
    vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelineLayout,
      3,
      1,
      &shadowDescriptorSet,
      0,
      nullptr);

    for (auto& kv : frameInfo.gameObjects) {
    auto &obj = kv.second;
    if (obj.model == nullptr) continue; // filter criterial

    glm::mat4 openglToVulkan = glm::mat4{1.0f, 0.f, 0.f, 0.f,
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

    obj.model->drawAll(frameInfo.commandBuffer, pipelineLayout);
  }
  
}

}  // namespace lve