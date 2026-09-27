cmake_minimum_required(VERSION 3.20)
project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(OpenGLProject
    src/main.cpp
    src/glad.c
    src/shaderClass.cpp
    src/VAO.cpp
    src/VBO.cpp
    src/EBO.cpp
)

target_include_directories(OpenGLProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(OpenGLProject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib
    opengl32
    user32
    gdi32
    shell32
)

add_custom_command(TARGET OpenGLProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
    $<TARGET_FILE_DIR:OpenGLProject>/Shaders
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
