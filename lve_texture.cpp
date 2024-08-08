#include "lve_texture.hpp"

// libs
// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// std
#include <iostream>


namespace lve
{
LveTexture::LveTexture(LveDevice & device, const std::string & _filename, TextureType _type):lveDevice{device}, filename{_filename}, type{_type}
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    std::cout<<"load texture ok" << std::endl;

    uint32_t mipLevels = 1;// static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    // 创建一个临时的staging buffer
        LveBuffer stagingBuffer{
            lveDevice,
            imageSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        // VkBuffer stagingBuffer;
        // VkDeviceMemory stagingBufferMemory;
        // lveDevice.createBuffer(
        //     imageSize,
        //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        //     stagingBuffer,
        //     stagingBufferMemory);
        
        // 将图像数据复制到staging buffer
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)pixels);
        // void *data;
        // vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
        // memcpy(data, pixels, static_cast<size_t>(imageSize));
        // vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

        stbi_image_free(pixels);


        // 创建纹理图像
        
        lveDevice.createImage(texWidth, 
        texHeight, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        textureImage, 
        textureImageMemory);

        // 将staging buffer的数据复制到纹理图像
        lveDevice.transitionImageLayout(textureImage, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_UNDEFINED, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        lveDevice.copyBufferToImage(stagingBuffer.getBuffer(), 
        textureImage, 
        static_cast<uint32_t>(texWidth), 
        static_cast<uint32_t>(texHeight),
        1);
        lveDevice.transitionImageLayout(textureImage, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // 清理staging buffer
        // vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
        // vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);

        textureImageView = lveDevice.createImageView(textureImage, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_ASPECT_COLOR_BIT);


        textureSampler = lveDevice.createTextureSampler();

        // 更新描述符集
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

}

LveTexture::~LveTexture()
{
    vkDestroySampler(lveDevice.device(), textureSampler, nullptr);
    vkDestroyImageView(lveDevice.device(), textureImageView, nullptr);
    vkDestroyImage(lveDevice.device(), textureImage, nullptr);
    vkFreeMemory(lveDevice.device(), textureImageMemory, nullptr);
}

std::shared_ptr<LveTexture> LveTexture::createTextureFromFile(LveDevice &device, const std::string &filename,TextureType type)
{

    return std::make_shared<LveTexture>(device, filename, type);
}

void LveTexture::materialBuilder::loadMaterial(const tinyobj::material_t &material)
{
    materialCoef.ambient = glm::vec4(material.ambient[0], material.ambient[1], material.ambient[2], 1.0f);
    materialCoef.diffuse = glm::vec4(material.diffuse[0], material.diffuse[1], material.diffuse[2], 1.0f);
    materialCoef.specular = glm::vec4(material.specular[0], material.specular[1], material.specular[2], 1.0f);
    materialCoef.emission = glm::vec4(material.emission[0], material.emission[1], material.emission[2], 1.0f);
    materialCoef.shininess = material.shininess;
    materialCoef.ior = material.ior;
    materialCoef.dissolve = material.dissolve;
    materialCoef.illum = material.illum;
    materialCoef.Ka_exist = !material.ambient_texname.empty();
    materialCoef.Kd_exist = !material.diffuse_texname.empty();
    materialCoef.Ks_exist = !material.specular_texname.empty();
    materialCoef.Ke_exist = !material.specular_highlight_texname.empty();

    texturePath.ambient_texname = material.ambient_texname;
    texturePath.diffuse_texname = material.diffuse_texname;
    // std::cout << "texturePath.diffuse_texname:" << texturePath.diffuse_texname << std::endl;
    texturePath.specular_texname = material.specular_texname;
    texturePath.specular_highlight_texname = material.specular_highlight_texname;

}

}
