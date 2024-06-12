cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
#set(CMAKE_SYSTEM_INCLUDE_PATH "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/ucrt")

project(Dream LANGUAGES CXX)

# if (NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_BUILD_TYPE Release)
# endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:"MSVCRT.lib")
endif()

if (WIN32)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_executable(pigine ${SOURCES})

target_include_directories(pigine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/engine/3rdparty/glm)
target_include_directories(pigine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/engine/3rdparty/glfw-3.3.8.bin.WIN64/include)
target_include_directories(pigine PRIVATE E:/dataAndCode/CG/dependencies/VulkanSDK/Include)
target_include_directories(pigine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/engine/3rdparty/tinyObjLoader)


target_link_directories(pigine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/engine/3rdparty/glfw-3.3.8.bin.WIN64/lib-vc2022
    E:/dataAndCode/CG/dependencies/VulkanSDK/Lib
)

target_link_libraries(pigine glfw3 vulkan-1)

# add_custom_command(
#     OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/engine/shader/res.txt
#     COMMAND ${CMAKE_COMMAND} -E echo "Running pre-build command"
#     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile.bat
#     COMMENT "Running a .bat script before building"
# )
# add_custom_target(
#     PreBuild ALL
#     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/engine/shader/res.txt
# )

#add_dependencies(pigine PreBuild)

add_custom_command(
    TARGET pigine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Running post-build command"
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile.bat
    COMMENT "Running a .bat script after building"
)


