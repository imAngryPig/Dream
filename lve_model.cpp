#include "lve_model.hpp"
#include "staticValue.hpp"

#include "lve_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp> // 这个头文件提供了glm一些用于生成哈希值的函数，这些函数可以用于例如std::unordered_map这样的哈希容器

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// std
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <filesystem>

namespace std {
template <>
struct hash<lve::LveModel::Vertex> {
    size_t operator()(lve::LveModel::Vertex const &vertex) const {
        size_t seed = 0;
        lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};
}  // namespace std

namespace lve
{
    LveModel::LveModel(LveDevice &device, LveModel::Builder builder, 
    LveDescriptorPool &descriptorPool, 
    std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts
    ) : lveDevice{device}
    {
        // createVertexBuffers("container.jpg", builder.vertices);
        // createIndexBuffers("container.jpg", builder.indices);
        // createTextureBuffers();
        localbuilder = std::move(builder);
        std::cout << "descriptorSetLayouts.size():" << descriptorSetLayouts.size() << std::endl;
        createRenderingResources(descriptorPool, descriptorSetLayouts);
        std::cout << "createRenderingResources success" << std::endl;
    }

    LveModel::~LveModel()
    {
        // vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
        // vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);

        // if ( hasIndexBuffer ){
        //     vkDestroyBuffer(lveDevice.device(), indexBuffer, nullptr);
        //     vkFreeMemory(lveDevice.device(), indexBufferMemory, nullptr);
        // }  // nolonger needed since lve_buffer

        
        
        // vkDestroySampler(lveDevice.device(), textureSampler, nullptr);
        // vkDestroyImageView(lveDevice.device(), textureImageView, nullptr);
        // vkFreeMemory(lveDevice.device(), textureImageMemory, nullptr);
        // vkDestroyImage(lveDevice.device(), textureImage, nullptr);
    }

    std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice &device, const std::string &filepath,
    LveDescriptorPool &descriptorPool, 
    std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts
    // LveModel::Builder &builder
    )
    {
        Builder builder{};
        builder.loadModel(filepath, device);
        std::cout << "Vertex count:" << builder.vertices.size() << std::endl;

        return std::make_unique<LveModel>(device, std::move(builder), descriptorPool, descriptorSetLayouts);
    }

    void LveModel::bind(VkCommandBuffer commandBuffer, std::string name)
    {
        VkBuffer buffers[] = {vertexBuffers[name]->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if ( hasIndexBuffer ) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffers[name]->getBuffer(), 0, VK_INDEX_TYPE_UINT32);  
        }
    }

    void LveModel::draw(VkCommandBuffer commandBuffer, std::string name)
    {
        if(hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCounts[name], 1, 0, 0, 0);
        } 
        else {
            vkCmdDraw(commandBuffer, vertexCounts[name], 1, 0, 0);
        }
    }

    void LveModel::drawAll(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
    {
        for(auto& x : descriptorSets){
            // std::cout << "x.first:" << x.first << std::endl;
            if(x.second.size() != 2){
                std::cout << "descriptorSets.size() != 2" << std::endl;
                continue;
            }
            if(x.second[0] == VK_NULL_HANDLE || x.second[1] == VK_NULL_HANDLE) {
                std::cout << "descriptorSets == VK_NULL_HANDLE" << std::endl;
                continue;
            }
            // std::cout << "drawALL" << std::endl;
            vkCmdBindDescriptorSets(commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            pipelineLayout, 
            1, 
            1, 
            &x.second[0], 
            0, 
            nullptr);
            vkCmdBindDescriptorSets(commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            pipelineLayout, 
            2, 
            1, 
            &x.second[1], 
            0, 
            nullptr);
            
            bind(commandBuffer, x.first);
            draw(commandBuffer, x.first);
        }
    }

    void LveModel::createVertexBuffers(std::string name, const std::vector<Vertex> &vertices)
    {
        #ifdef USE_VULKAN_MEMORY_COHERENT
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        lveDevice.createBuffer(bufferSize,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                vertexBuffer,
                                vertexBufferMemory);

        void *data;
        vkMapMemory(lveDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), vertexBufferMemory);
        #else
        vertexCounts[name] = static_cast<uint32_t>(vertices.size());
        assert(vertexCounts[name] >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCounts[name];
        uint32_t vertexSize = sizeof(vertices[0]);

        LveBuffer stagingBuffer{
            lveDevice,
            vertexSize,
            vertexCounts[name],
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        // void *data;
        // vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        // memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        // vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

        vertexBuffers[name] = std::make_unique<LveBuffer>(
            lveDevice,
            vertexSize,
            vertexCounts[name],
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        // lveDevice.createBuffer(
        //     bufferSize,
        //     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        //     vertexBuffer,
        //     vertexBufferMemory);

        lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffers[name]->getBuffer(), bufferSize);

        // staging buffer is a stack variable can be automatically cleaned up when exist the creatBuffer Function.
        // vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
        // vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
        
        #endif
        
    }

    void LveModel::createIndexBuffers(std::string name, const std::vector<uint32_t> &indices)
    {
        #ifdef USE_VULKAN_MEMORY_COHERENT
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if( !hasIndexBuffer ){
            return ;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        lveDevice.createBuffer(bufferSize,
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                indexBuffer,
                                indexBufferMemory);

        void *data;
        vkMapMemory(lveDevice.device(), indexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), indexBufferMemory);
        #else
        indexCounts[name] = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCounts[name] > 0;   // if indexCount is 0, then we don't have index buffer

        if (!hasIndexBuffer) {
        return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCounts[name];

        uint32_t indexSize = sizeof(indices[0]);

        LveBuffer stagingBuffer{
            lveDevice,
            indexSize,
            indexCounts[name],
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        

        // VkBuffer stagingBuffer;
        // VkDeviceMemory stagingBufferMemory;
        // lveDevice.createBuffer(
        //     bufferSize,
        //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        //     stagingBuffer,
        //     stagingBufferMemory);

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)indices.data());
        // void *data;
        // vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        // memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        // vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

        indexBuffers[name] = std::make_unique<LveBuffer>(
            lveDevice,
            indexSize,
            indexCounts[name],
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        // lveDevice.createBuffer(
        //     bufferSize,
        //     VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        //     indexBuffer,
        //     indexBufferMemory);


        lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffers[name]->getBuffer(), bufferSize);

        // vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
        // vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);

        #endif

    }

    void LveModel::createRenderingResources(LveDescriptorPool &descriptorPool, 
        std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts)
    {
        for(const auto& sub_mesh : localbuilder.subMeshesIndex){
            std::string material_name = sub_mesh.first;
            std::vector<uint32_t> indices = sub_mesh.second;
            std::vector<Vertex> sub_vertices;
            std::vector<uint32_t> localIndices;
            for(int i = 0; i < indices.size(); i++){
                sub_vertices.emplace_back(localbuilder.vertices[indices[i]]);
                localIndices.push_back(i);
            }
            // assert(indices.size()==builder.indices.size() && "indices.size()==builder.indices.size()");
            createVertexBuffers(material_name, sub_vertices);
            createIndexBuffers(material_name, localIndices);

            LveTexture::materialBuilder& material = localbuilder.mapMaterials[material_name];
            
            createUniformBuffers(material_name, material.materialCoef);
        }
            createDescriptorSet(localbuilder, descriptorPool, descriptorSetLayouts);
        
    }

    void LveModel::createTextureBuffers()
    {   
        // std::string mtl_BaseDir = "../../models/";
        // texture = LveTexture::createTextureFromFile(lveDevice, 
        // "../../models/container.jpg", 
        // LveTexture::TextureType::DIFFUSE);
        // imageInfo = texture->imageInfo;

    }

    void LveModel::createUniformBuffers(std::string name, LveTexture::MaterialCoefficients &materialCoef)
    {
        VkDeviceSize bufferSize = sizeof(materialCoef);

        LveBuffer stagingBuffer{
            lveDevice,
            bufferSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)&materialCoef);

        uniformBuffers[name] = std::make_unique<LveBuffer>(
            lveDevice,
            bufferSize,
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        lveDevice.copyBuffer(stagingBuffer.getBuffer(), uniformBuffers[name]->getBuffer(), bufferSize);
    }

    void LveModel::createDescriptorSet(LveModel::Builder &builder, LveDescriptorPool &descriptorPool, 
        std::vector< std::unique_ptr<LveDescriptorSetLayout> >& descriptorSetLayouts)
    {
        // TODO: 根据builder中的subMeshesIndex，mapMaterials创建descriptorSets
        for(auto& x : builder.subMeshesIndex){
            std::string material_name = x.first;
            std::shared_ptr<LveTexture> map_Ka = builder.mapTextures[builder.mapMaterials[material_name].texturePath.ambient_texname];
            std::shared_ptr<LveTexture> map_Kd = builder.mapTextures[builder.mapMaterials[material_name].texturePath.diffuse_texname];
            std::shared_ptr<LveTexture> map_Ks = builder.mapTextures[builder.mapMaterials[material_name].texturePath.specular_texname];
            std::shared_ptr<LveTexture> map_Ns = builder.mapTextures[builder.mapMaterials[material_name].texturePath.specular_highlight_texname];
            // std::vector<VkDescriptorSet> temp(2);
            // std::cout << "builder.mapMaterials[material_name].texturePath.diffuse_texname:" <<
            // builder.mapMaterials[material_name].texturePath.diffuse_texname << std::endl;

            descriptorSets[material_name].resize(2);
            
            auto t = LveDescriptorWriter(*descriptorSetLayouts[0], descriptorPool);
            int Ka_exist = builder.mapMaterials[material_name].materialCoef.Ka_exist;
            int Kd_exist = builder.mapMaterials[material_name].materialCoef.Kd_exist;
            int Ks_exist = builder.mapMaterials[material_name].materialCoef.Ks_exist;
            int Ke_exist = builder.mapMaterials[material_name].materialCoef.Ke_exist;
            // std::cout << "Kd_exist:" << Kd_exist << std::endl;
            // if(!map_Kd) std::cout << "!!!!!!" << std::endl;
            if(Ka_exist) t.writeImage(0, &map_Ka->imageInfo); else t.writeImage(0, &builder.mapTextures["container.jpg"]->imageInfo);
            if(Kd_exist) 
            {t.writeImage(1, &map_Kd->imageInfo); std::cout << "write binding1" << std::endl;}
            else{
                t.writeImage(1, &builder.mapTextures["container.jpg"]->imageInfo);
            }

            if(Ks_exist) t.writeImage(2, &map_Ks->imageInfo); else t.writeImage(2, &builder.mapTextures["container.jpg"]->imageInfo);
            if(Ke_exist) t.writeImage(3, &map_Ns->imageInfo); else t.writeImage(3, &builder.mapTextures["container.jpg"]->imageInfo);
            t.build(descriptorSets[material_name][0]);
            // std::cout << "material_name" << material_name <<  std::endl;
            // LveDescriptorWriter(*descriptorSetLayouts[0], descriptorPool).writeImage(
            //     0, 
            //     &map_Ka->imageInfo
            //     ).writeImage(
            //     1, 
            //     &map_Kd->imageInfo
            //     ).writeImage(
            //     2, 
            //     &map_Ks->imageInfo
            //     ).writeImage(
            //     3, 
            //     &map_Ns->imageInfo
            //     ).build(temp[0]);
            LveDescriptorWriter(*descriptorSetLayouts[1], descriptorPool).writeBuffer(
                0, 
                &uniformBuffers[material_name]->descriptorInfo()
                ).build(descriptorSets[material_name][1]);
            
        }
        std::cout << "createDescriptorSet finish " << std::endl;
    }

    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        // position
        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        // color
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        // normal
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        // texture coordinates
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
        // // position
        // attributeDescriptions[0].binding = 0;
        // attributeDescriptions[0].location = 0;
        // attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        // attributeDescriptions[0].offset = offsetof(Vertex, position);

        // attributeDescriptions[1].binding = 0;
        // attributeDescriptions[1].location = 1;
        // attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        // attributeDescriptions[1].offset = offsetof(Vertex, color);
        // return attributeDescriptions;
    }

    void LveModel::Builder::loadModel(const std::string &filepath, LveDevice& device)
    {
        tinyobj::attrib_t attrib; // position color normal texture coordinate
        std::vector<tinyobj::shape_t> shapes; // index value for each face
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string mtlBaseDir = std::filesystem::path(filepath).parent_path().string();

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), mtlBaseDir.c_str())) {
            throw std::runtime_error(warn + err);
        }
        if(!warn.empty())
        std::cout << "warn:" << warn << std::endl;
        if(!err.empty())
        std::cout << "err:" << err << std::endl;

        std::cout << "materials:" << materials.size() << std::endl;

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        
        for( const auto& shape : shapes ) {
            // std::cout << "shape.name:" << shape.name << std::endl;
            // std::cout << "shape.mesh.indices.size():" << shape.mesh.indices.size() << std::endl;
            // std::cout << "shape.mesh.num_face_vertices.size():"<< shape.mesh.num_face_vertices.size() << std::endl;
            // std::cout << "shape.mesh.material_ids.size():" << shape.mesh.material_ids.size() << std::endl;
            
            // std::cout << std::endl;
            // for(int i = 0; i < shape.mesh.num_face_vertices.size(); i++){
                // std::cout << "shape.mesh.num_face_vertices[" << i << "]:" << shape.mesh.num_face_vertices[i] << std::endl;
                // std::cout << "shape.mesh.material_ids[" << i << "]:" << shape.mesh.material_ids[i] << std::endl;
            // }
            int index_offset = 0;

            assert(shape.mesh.num_face_vertices.size() == shape.mesh.material_ids.size() && "num_face_vertices.size() != material_ids.size()");

            for(int i = 0; i < shape.mesh.num_face_vertices.size(); i++){
                auto material_name = materials[shape.mesh.material_ids[i]].name;
                // process per face
                int num_size = shape.mesh.num_face_vertices[i];
                for(int j = 0; j < num_size; j++){
                    const tinyobj::index_t& index = shape.mesh.indices[index_offset + j];
                    Vertex vertex{};

                    if(index.vertex_index >= 0){
                        vertex.position = {
                            attrib.vertices[num_size * index.vertex_index + 0],
                            attrib.vertices[num_size * index.vertex_index + 1],
                            attrib.vertices[num_size * index.vertex_index + 2]
                        };

                        vertex.color = {        // tinyobj会自动将attrib.color分配和attrib.vertices相同数量的内存，
                            attrib.colors[num_size * index.vertex_index + 0],  // 并且如果obj未显式指定，则默认1.0f
                            attrib.colors[num_size * index.vertex_index + 1],
                            attrib.colors[num_size * index.vertex_index + 2]
                        };
                    
                    }

                    if(index.normal_index >= 0){
                        vertex.normal = {
                            attrib.normals[num_size * index.normal_index + 0],
                            attrib.normals[num_size * index.normal_index + 1],
                            attrib.normals[num_size * index.normal_index + 2]
                        };
                    }

                    if(index.texcoord_index >= 0){
                        vertex.uv = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                        };
                    }

                    if (uniqueVertices.count(vertex) == 0) {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }
                    indices.push_back(uniqueVertices[vertex]);
                    subMeshesIndex[material_name].push_back(uniqueVertices[vertex]);
                } // end process face
                index_offset += num_size;
            }

        }

        if(mapTextures.find("container.jpg") == mapTextures.end()){
            mapTextures["container.jpg"] = LveTexture::createTextureFromFile(device, mtlBaseDir + "/../" + "container.jpg", LveTexture::TextureType::DIFFUSE);
        }   // 初始化一个默认的texture

        // createImageInfoFromMaterial
        for (const auto &material : materials) {
            LveTexture::materialBuilder builder;
            builder.loadMaterial(material);
            mapMaterials[material.name] = builder;

            auto map_Ka = builder.texturePath.ambient_texname;
            if( !map_Ka.empty() && mapTextures.find(map_Ka) == mapTextures.end()){
                mapTextures[map_Ka] = LveTexture::createTextureFromFile(device, mtlBaseDir + "/" + map_Ka, LveTexture::TextureType::AMBIENT);
            }
            auto map_Kd = builder.texturePath.diffuse_texname;
            if( !map_Kd.empty() && mapTextures.find(map_Kd) == mapTextures.end()){
                mapTextures[map_Kd] = LveTexture::createTextureFromFile(device, mtlBaseDir + "/" + map_Kd, LveTexture::TextureType::DIFFUSE);
                std::cout << "mapTextures[map_Kd] = LveTexture::createTextureFromFile:" <<map_Kd << " " << mtlBaseDir + "/" + map_Kd << std::endl;
            } 
            auto map_Ks = builder.texturePath.specular_texname;
            if( !map_Ks.empty() && mapTextures.find(map_Ks) == mapTextures.end()){
                mapTextures[map_Ks] = LveTexture::createTextureFromFile(device, mtlBaseDir + "/" + map_Ks, LveTexture::TextureType::SPECULAR);
            }
            auto map_Ns = builder.texturePath.specular_highlight_texname;
            if( !map_Ns.empty() && mapTextures.find(map_Ns) == mapTextures.end()){
                mapTextures[map_Ns] = LveTexture::createTextureFromFile(device, mtlBaseDir + "/" + map_Ns, LveTexture::TextureType::SPECULAR_HIGHLIGHT);
            }

        }
    }

   

} // namespace lve
