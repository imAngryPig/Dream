#include "lve_pipeline.hpp"

#include "lve_model.hpp"

//std
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <filesystem>

namespace lve
{


    LvePipeline::LvePipeline(LveDevice &device, const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo)
    : lveDevice{device}
    {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    LvePipeline::~LvePipeline()
    {
        vkDestroyShaderModule(lveDevice.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(lveDevice.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(lveDevice.device(), graphicsPipeline, nullptr);
        
    }

    void LvePipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    void LvePipeline::defaultPipelineConfigInfo(PipelineConfigInfo &configInfo)
    {
        //static PipelineConfigInfo configInfo{};

        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // configInfo.viewport.x = 0.0f;
        // configInfo.viewport.y = 0.0f;
        // configInfo.viewport.width = static_cast<float>(width);
        // configInfo.viewport.height = static_cast<float>(height);    // 视口大小
        // configInfo.viewport.minDepth = 0.0f;
        // configInfo.viewport.maxDepth = 1.0f;        // 0到1的深度范围

        // configInfo.scissor.offset = {0, 0};
        // configInfo.scissor.extent = {width, height};    // 裁剪，把超出width height的部分裁剪掉

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = nullptr;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = nullptr;

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE; // 把小于0的depth值变为0，大于1的depth值变为1
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE; // 是否丢弃片段
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL; // 可选线框模式，或者填充模式
        configInfo.rasterizationInfo.lineWidth = 1.0f; // 线宽
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE; // 背面剔除
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // 顺时针为正面
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE; // 深度偏移
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;         // optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;   // optional

        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;          // optional
        configInfo.multisampleInfo.pSampleMask = nullptr;            // optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // optional

        configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
        | VK_COLOR_COMPONENT_G_BIT 
        | VK_COLOR_COMPONENT_B_BIT 
        | VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // optional

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // optional

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f; // optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {}; // optional
        configInfo.depthStencilInfo.back = {};  // optional

        configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.flags = 0;

        configInfo.bindingDescriptions = LveModel::Vertex::getBindingDescriptions();
        configInfo.attributeDescriptions = LveModel::Vertex::getAttributeDescriptions();
    }

    void LvePipeline::enableAlphaBlending(PipelineConfigInfo &configInfo)
    {
        configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    }

    std::vector<char> LvePipeline::readFile(const std::string &filepath)
    {
        auto currentPath = std::filesystem::current_path().parent_path().parent_path();
        std::cout << "currentPath: " << currentPath << std::endl;
        std::ifstream file{currentPath.string() + filepath, std::ios::ate | std::ios::binary};
        

        if(!file.is_open()){
            std::cout << "filepath: " << filepath << std::endl;
            throw std::runtime_error("failed to open file: " + currentPath.string() + filepath);
        }
        
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void LvePipeline::createGraphicsPipeline(
        const std::string &vertFilepath, 
        const std::string &fragFilepath,
        const PipelineConfigInfo &configInfo)
    {
        assert(configInfo.pipelineLayout != VK_NULL_HANDLE
        && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
        assert(configInfo.renderPass != VK_NULL_HANDLE 
        && "Cannot create graphics pipeline:: no renderPass provided in configInfo");

        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;


        auto &bindingDescriptions = configInfo.bindingDescriptions;
        auto &attributeDescriptions = configInfo.attributeDescriptions;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // VkPipelineViewportStateCreateInfo viewportInfo{};
        // viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        // viewportInfo.viewportCount = 1;
        // viewportInfo.pViewports = &configInfo.viewport;
        // viewportInfo.scissorCount = 1;
        // viewportInfo.pScissors = &configInfo.scissor;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        std::cout << "start Create Pipeline" << std::endl;
        if(vkCreateGraphicsPipelines(lveDevice.device(),
        VK_NULL_HANDLE, 
        1, 
        &pipelineInfo, 
        nullptr, 
        &graphicsPipeline) != VK_SUCCESS)
        {
            std::cout << "pipeline create wrong" << std::endl;
            throw std::runtime_error("failed to create graphics pipeline");
        }
        std::cout << "finish Create Pipeline" << std::endl;
    }

    void LvePipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if(vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module");
        }

    }

} // namespace lve
