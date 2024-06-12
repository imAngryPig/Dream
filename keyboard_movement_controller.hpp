#pragma once

#include "lve_game_object.hpp"
#include "lve_window.hpp"

namespace lve {
class KeyboardMovementController {
    public:
    struct KeyMappings {
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
};

    // TODO: Add onUpdate, onEvent, onImGuiRender
    // void onUpdate(LveWindow* lveWindow, float dt);
    // void onEvent(LveEvent& event);
    // void onImGuiRender();
    // void setMoveSpeed(float moveSpeed);
    // void setLookSpeed(float lookSpeed);
    // void setKeyMappings(KeyMappings keyMappings);

    KeyboardMovementController(LveWindow& window, LveGameObject& gameObject_): lveWindow(&window), gameObject(gameObject_){

    };
    ~KeyboardMovementController() = default;

    void moveInPlaneXZ(float dt);
    
    void processInput(float dt, LveGameObject& gameObject);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    void ProcessMouseMovement(bool constaintPitch = true);

    static float lastX;
    static float lastY;
    static bool firstMouse;
    static float xoffset;
    static float yoffset;
    
private:
    KeyMappings keys{};
    float moveSpeed{3.f};
    float lookSpeed{1.5f};

    
    float MouseSensitivity =  0.05f;
    
    LveGameObject& gameObject;
    LveWindow* lveWindow;
    
    // void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    
};
}  // namespace lve