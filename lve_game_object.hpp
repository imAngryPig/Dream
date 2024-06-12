#pragma once

#include "lve_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace lve
{
    struct TransformComponent {
        glm::vec3 translation{};    // position offset
        glm::vec3 scale{1.f, 1.f, 1.f};  // scaling factor
        glm::vec3 rotation{};        // rotation angle in radians

        // matrix corresponds to translate * Ry * Rx * Rz * scale transformation
        // Rotation conventions uses tait-bryan angles with axis order Y(1), X(2), Z(3)
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    struct PointLightComponent {
        float lightIntensity = 1.0f;
    };
    

class LveGameObject{
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, LveGameObject>;

        static LveGameObject createGameObject(){
            static id_t currentId = 0;
            return LveGameObject{currentId++};
        }

        static LveGameObject makePointLight(
            float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f, 1.f, 1.f)
        );

        LveGameObject(const LveGameObject&) = delete;
        LveGameObject& operator=(const LveGameObject&) = delete;
        LveGameObject(LveGameObject&&) = default;
        LveGameObject& operator=(LveGameObject&&) = default;

        const id_t getId() const { return id; }

        glm::vec3 color{};
        TransformComponent transform{};

        // Optional pointer components
        std::shared_ptr<LveModel> model{}; // one model can be used by multiple game objects,so we use shared_ptr
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        LveGameObject(id_t objId) : id{objId} {} // to make sure that the only way to create a game object is through the createGameObject function

        id_t id;
};
} // namespace lve
