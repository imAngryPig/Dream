#pragma once

#include "lve_model.hpp"

// std
#include <memory>

namespace lve
{
    struct Transform2dComponent {
        glm::vec2 translation{};    // position offset
        glm::vec2 scale{1.f, 1.f};  // scaling factor
        float rotation{0.f};        // rotation angle in radians

        glm::mat2 mat2() {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotMatrix{{c, s}, {-s, c}};

            glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}}; // attention: glm matrix constructor is column-major, 
                                // so the first column is the x-axis, the second column is the y-axis
            return rotMatrix * scaleMat;
        }
    };
    

class LveGameObject{
    public:
        using id_t = unsigned int;

        static LveGameObject createGameObject(){
            static id_t currentId = 0;
            return LveGameObject{currentId++};
        }

        LveGameObject(const LveGameObject&) = delete;
        LveGameObject& operator=(const LveGameObject&) = delete;
        LveGameObject(LveGameObject&&) = default;
        LveGameObject& operator=(LveGameObject&&) = default;

        const id_t getId() const { return id; }

        std::shared_ptr<LveModel> model{}; // one model can be used by multiple game objects,so we use shared_ptr
        glm::vec3 color{};
        Transform2dComponent transform2d;

    private:
        LveGameObject(id_t objId) : id{objId} {} // to make sure that the only way to create a game object is through the createGameObject function

        id_t id;
};
} // namespace lve
