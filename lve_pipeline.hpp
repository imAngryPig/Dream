#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <vector>

namespace lve
{

struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    // VkViewport viewport;
    // VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;

    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class LvePipeline
{
    public:
    LvePipeline(
        LveDevice& device,
        const std::string& vertFilepath, 
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo
    );
    ~LvePipeline();

    LvePipeline(const LvePipeline&) = delete;
    LvePipeline& operator=(const LvePipeline&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
    static void enableAlphaBlending(PipelineConfigInfo& configInfo);

    private:
    static std::string rootFilePath; 
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(
        const std::string& vertFilepath, 
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo
    ); // helper function to create graphics pipeline

    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    LveDevice& lveDevice;
    VkPipeline graphicsPipeline;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

};

} // namespace lve
