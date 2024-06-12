#include "keyboard_movement_controller.hpp"

// std
#include <limits>
#include <iostream>

namespace lve {

void KeyboardMovementController::moveInPlaneXZ(
    float dt) {
    auto window = lveWindow->getGLFWWindow();


    glm::vec3 rotate{0};
    if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
    if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
    if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
    if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
    } // ensure rotate is not zero length

    // limit pitch values between about +/- 85ish degrees
    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

    float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, -1.f, 0.f};

    glm::vec3 moveDir{0.f};
    if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
    } // ensure rotate is not zero length
}

void KeyboardMovementController::processInput(float dt, LveGameObject &gameObject)
{
    auto extent = lveWindow->getExtent();
    lastX = extent.width / 2.f;
    lastY = extent.height / 2.f;
    firstMouse = true;

    // glfwSetCursorPosCallback(lveWindow->getGLFWWindow(), mouse_callback);
    
}
void KeyboardMovementController::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    xoffset = xpos - lastX;
    yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    // yoffset = -yoffset;  屏幕空间中的y和opengl的y是反向的
    // std::cout << ypos << std::endl;

    lastX = xpos;
    lastY = ypos;
    // std::cout << xoffset << " " << yoffset << std::endl;
    // ProcessMouseMovement(xoffset, yoffset);
}

void KeyboardMovementController::ProcessMouseMovement(bool constaintPitch)
{
    if(lveWindow->wasWindowResized()){
        auto extent = lveWindow->getExtent();
        lastX = extent.width / 2.f;
        lastY = extent.height / 2.f;
        firstMouse = true;
        xoffset = 0.f;
        yoffset = 0.f;
    }

    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    gameObject.transform.rotation.y += glm::radians(xoffset);
    gameObject.transform.rotation.x += glm::radians(yoffset);

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    
    if(constaintPitch){
        if (gameObject.transform.rotation.x > glm::radians(89.0f))
            gameObject.transform.rotation.x = glm::radians(89.0f);
        if (gameObject.transform.rotation.x < glm::radians(-89.0f))
            gameObject.transform.rotation.x = glm::radians(-89.0f);
    }
}


float KeyboardMovementController::lastX = 800.f / 2.f;
float KeyboardMovementController::lastY = 600.f / 2.f;
bool KeyboardMovementController::firstMouse = true;
float KeyboardMovementController::xoffset = 0.f;
float KeyboardMovementController::yoffset = 0.f;

} // namespace lve