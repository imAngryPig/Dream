#pragma once

#include "lve_device.hpp"
#include "lve_buffer.hpp"
#include "lve_descriptors.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>

// std
#include <memory>

namespace lve
{
    
class LveTexture
{
    public:
        enum class TextureType
        {
            AMBIENT,
            DIFFUSE,
            SPECULAR,
            SPECULAR_HIGHLIGHT
        };
        
        struct MaterialCoefficients
        {
            alignas(16) glm::vec4 ambient{0.2f, 0.2f, 0.2f, 1.0f};
            alignas(16) glm::vec4 diffuse{0.8f, 0.8f, 0.8f, 1.0f};
            alignas(16) glm::vec4 specular{0.0f, 0.0f, 0.0f, 1.0f};
            alignas(16) glm::vec4 emission{0.0f, 0.0f, 0.0f, 1.0f};
            alignas(4) float shininess = 900.0f;
            alignas(4) float ior = 1.45f;
            alignas(4) float dissolve = 1.0f;
            alignas(4) int illum = 1;
            alignas(4) int Ka_exist = 0; // Ka, Kd, Ks, Ns   // Must alignas 4
            alignas(4) int Kd_exist = 0; // Ka, Kd, Ks, Ns   // Must alignas 4
            alignas(4) int Ks_exist = 0; // Ka, Kd, Ks, Ns   // Must alignas 4
            alignas(4) int Ke_exist = 0; // Ka, Kd, Ks, Ns   // Must alignas 4
        };

        struct MaterialPath
        {
            std::string ambient_texname;   // map_Ka. For ambient or ambient occlusion.
            std::string diffuse_texname;   // map_Kd
            std::string specular_texname;  // map_Ks
            std::string specular_highlight_texname;  // map_Ns
        };

        struct materialBuilder
        {
            MaterialCoefficients materialCoef;
            MaterialPath texturePath;
            void loadMaterial(const tinyobj::material_t &material);
            
        };
        
        LveTexture(LveDevice& device, const std::string& _filename, TextureType type);
        ~LveTexture();

        LveTexture(const LveTexture&) = delete;
        LveTexture& operator=(const LveTexture&) = delete;

        static std::shared_ptr<LveTexture> createTextureFromFile(LveDevice& device, const std::string& filename, TextureType type);

        VkDescriptorImageInfo imageInfo{};

        VkImageView imageView() const { return textureImageView; }
        VkSampler sampler() const { return textureSampler; }

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

    private:
        LveDevice& lveDevice;
        
        std::string filename;
        TextureType type;

};

} // namespace lve
