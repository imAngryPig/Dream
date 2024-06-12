#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

void printMatrix(const glm::mat4& matrix) {
    const float* pSource = (const float*)glm::value_ptr(matrix);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << pSource[j * 4 + i] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void printVec4(const glm::vec4& vec) {
    std::cout << vec.x << " " << vec.y << " " << vec.z << " " << vec.w << std::endl;
}

void test() {
    // 内在旋转
    glm::mat4 intrinsicTransform = glm::mat4(1.0f);
    auto intrinsicTransform1 = glm::rotate(intrinsicTransform, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // 绕X轴旋转30度
    auto intrinsicTransform2 = glm::rotate(intrinsicTransform1, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 绕更新后的Y轴旋转45度
    auto intrinsicTransform3 = glm::rotate(intrinsicTransform2, glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 绕更新后的Z轴旋转60度

    // 外在旋转
    glm::mat4 extrinsicTransform = glm::mat4(1.0f);
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // 绕X轴旋转30度
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 绕Y轴旋转45度
    glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 绕Z轴旋转60度
    extrinsicTransform = rotationX * rotationY * rotationZ * extrinsicTransform; // 按正确顺序应用外在旋转

    // 输出内在旋转和外在旋转的结果
    std::cout << "Intrinsic Rotation Matrix: " << std::endl;
    // printMatrix(intrinsicTransform3);

    std::cout << "Extrinsic Rotation Matrix: " << std::endl;
    // printMatrix(extrinsicTransform);

    
    // printMatrix(rotationX * rotationY * rotationZ);
    // printMatrix(rotationZ * rotationY * rotationX);

    printVec4(rotationZ * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    // printMatrix(intrinsicTransform2);

    // printMatrix(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.0f, 3.0f)), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));


}