#pragma once

#include "lve_device.hpp"
#include "lve_buffer.hpp"
#include "lve_texture.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace lve{
    class LveModel
    {
        public:

        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex &other) const {
                return position == other.position && color == other.color && normal == other.normal &&
                uv == other.uv;
            }
        };

        struct Builder {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            std::unordered_map<std::string, std::vector<uint32_t> > subMeshesIndex;

            std::unordered_map<std::string, LveTexture::materialBuilder> mapMaterials;
            std::unordered_map<std::string, std::shared_ptr<LveTexture> > mapTextures;

            void loadModel(const std::string &filepath, LveDevice& device);
        };

        LveModel(LveDevice& device, LveModel::Builder builder, LveDescriptorPool &descriptorPool, 
        std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts);
        ~LveModel();

        LveModel(const LveModel&) = delete;
        LveModel& operator=(const LveModel&) = delete;

        static std::unique_ptr<LveModel> createModelFromFile(
        LveDevice& device, 
        const std::string& filepath,
        LveDescriptorPool &descriptorPool, 
        std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts
        // LveModel::Builder &builder
        );

        void bind(VkCommandBuffer commandBuffer, std::string name);
        void draw(VkCommandBuffer commandBuffer, std::string name);

        void drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
        
        VkDescriptorImageInfo imageInfo{};

        private:
        void createVertexBuffers(std::string name, const std::vector<Vertex> &vertices);
        void createIndexBuffers(std::string name, const std::vector<uint32_t> &indices);
        void createRenderingResources(LveDescriptorPool &descriptorPool, 
        std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts);
        void createTextureBuffers();
        void createUniformBuffers(std::string name, LveTexture::MaterialCoefficients &materialCoef);
        void createDescriptorSet(LveModel::Builder &builder, LveDescriptorPool &descriptorPool, 
        std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts);

        LveDevice& lveDevice;
        Builder localbuilder{};

        std::unordered_map<std::string, std::unique_ptr<LveBuffer>> vertexBuffers;
        std::unordered_map<std::string, uint32_t> vertexCounts;

        bool hasIndexBuffer = false;
        std::unordered_map<std::string, std::unique_ptr<LveBuffer>> indexBuffers;
        std::unordered_map<std::string, uint32_t> indexCounts;

        bool hasTexture = false;
        std::shared_ptr<LveTexture> texture;

        std::unordered_map<std::string, std::unique_ptr<LveBuffer> > uniformBuffers;

        std::unordered_map<std::string, std::vector<VkDescriptorSet> > descriptorSets;
        
        
    };

} // namespace lve