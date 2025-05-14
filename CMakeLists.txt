cmake_minimum_required(VERSION 3.10)

project(Autoclicker)

# Set CMP0072 policy to prefer GLVND
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

include(FetchContent)

# Fetch Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.4  # Replace with the desired version
)
FetchContent_MakeAvailable(imgui)

# Add Dear ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)

# Add executable
add_executable(autoclicker_app src/main.cpp ${IMGUI_SOURCES})

# Include Dear ImGui headers
target_include_directories(autoclicker_app PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Link required libraries (e.g., OpenGL, GLFW)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(X11 REQUIRED)

target_link_libraries(autoclicker_app PRIVATE OpenGL::GL glfw ${X11_LIBRARIES})
