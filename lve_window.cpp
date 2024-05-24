#include "lve_window.hpp"

//  std
#include <stdexcept>


namespace lve{
LveWindow::LveWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name}
{
    initWindow();
}

LveWindow::~LveWindow(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface");
    }
}

void LveWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
    app->width = width;
    app->height = height;
}

void LveWindow::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    // 这行代码的作用是将一个用户定义的指针与窗口关联起来。在这个例子中，this指针（指向当前的lve_window对象）被关联到了window窗口。
    // 这个函数将pointer参数关联到window参数。然后，你可以通过调用glfwGetWindowUserPointer函数来获取这个指针：
    // void* glfwGetWindowUserPointer(GLFWwindow* window);
    // 这个功能在你需要在窗口的回调函数中访问一些额外数据时非常有用。例如，你可能在窗口的大小改变回调函数中需要访问lve_window对象。
    // 由于GLFW的回调函数是C风格的函数，
    // 它们不能直接访问C++对象的成员。但是，你可以通过glfwGetWindowUserPointer函数来获取lve_window对象的指针，然后在回调函数中访问它的成员。
    glfwSetWindowUserPointer(window, this);
    
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

} // namespace lve