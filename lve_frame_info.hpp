#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"
#include "lve_descriptors.hpp"

// lib
#include <vulkan/vulkan.h>

namespace lve {

#define MAX_LIGHTS 10

struct PointLight {
  glm::vec4 position{}; // ignore w
  glm::vec4 color{}; // w is intensity
  // float radius; // 由于simple_render_system 不需要radius信息来做渲染，所以这里的radius不需要给出，给point_light_system即可
};

struct GlobalUbo {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::mat4 lightProjection{1.f};
        glm::mat4 lightView{1.f};
        glm::mat4 lightInverseView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.04f}; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };


struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  LveCamera &camera;
  LveCamera &lightcamera;
  VkDescriptorSet globalDescriptorSet;
  LveGameObject::Map &gameObjects;
  LveDescriptorPool &descriptorPool;
};
}  // namespace lve
