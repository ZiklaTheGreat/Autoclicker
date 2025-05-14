#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdexcept>
#include <iostream>
#include <thread>
#include <chrono>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <atomic>

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2
};

enum class MousePosition {
    X = 0,
    Y = 0
};

class Gui {
private:
    GLFWwindow* window;           // GLFW window handle
    ImGuiContext* imgui_context;  // ImGui context
    int window_width;             // Window width
    int window_height;            // Window height
    const char* window_title;     // Window title

    bool show_settings = true; // Flag to show settings window

    int minutes = 0; // Minutes
    int seconds = 0; // Seconds
    int milliseconds = 0; // Milliseconds
    int repetitions = 0; // Repetitions
    MouseButton mouse_button = MouseButton::Left; // Mouse button

    std::atomic<bool> global_key_listener_running{false};
    std::thread global_key_listener_thread;

    // Initialize GLFW and create window
    int init_glfw() {
        if (!glfwInit()) {
            return -1;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        return 0;
    }

    // Initialize ImGui with GLFW and OpenGL3 backends
    void init_imgui() {
        IMGUI_CHECKVERSION();
        imgui_context = ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    // Start a global key listener
    void start_global_key_listener() {
        global_key_listener_running = true;
        global_key_listener_thread = std::thread([this]() {
            Display* display = XOpenDisplay(nullptr);
            if (!display) {
                std::cerr << "Failed to open X display" << std::endl;
                return;
            }

            Window root = DefaultRootWindow(display);
            XEvent event;

            while (global_key_listener_running) {
                XGrabKey(display, XKeysymToKeycode(display, XK_F9), AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
                XSelectInput(display, root, KeyPressMask);

                while (XPending(display)) {
                    XNextEvent(display, &event);
                    if (event.type == KeyPress) {
                        XKeyEvent* key_event = (XKeyEvent*)&event;
                        if (key_event->keycode == XKeysymToKeycode(display, XK_F9)) {
                            double xpos, ypos;
                            glfwGetCursorPos(window, &xpos, &ypos);

                            // Get the window position on the screen
                            int window_x, window_y;
                            glfwGetWindowPos(window, &window_x, &window_y);

                            // Calculate the cursor position relative to the screen
                            double screen_x = xpos + window_x;
                            double screen_y = ypos + window_y;

                            std::cout << "Global Screen Cursor Position (F9): (" << screen_x << ", " << screen_y << ")" << std::endl;
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            XCloseDisplay(display);
        });
    }

    // Stop the global key listener
    void stop_global_key_listener() {
        global_key_listener_running = false;
        if (global_key_listener_thread.joinable()) {
            global_key_listener_thread.join();
        }
    }

    // Render the ImGui frame
    void render() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        // Example ImGui window
        settingsWindow();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void settingsWindow() {
        if (!show_settings) return;

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Settings");

        // Input fields for time settings
        ImGui::Text("Set Timer:");
        ImGui::InputInt("Minutes", &minutes);
        ImGui::InputInt("Seconds", &seconds);
        ImGui::InputInt("Milliseconds", &milliseconds);
        ImGui::InputInt("Repetitions", &repetitions);

        // Ensure values are non-negative
        if (minutes < 0) minutes = 0;
        if (seconds < 0) seconds = 0;
        if (milliseconds < 0) milliseconds = 0;
        if (repetitions < 0) repetitions = 0;

        // Dropdown for mouse button selection
        ImGui::Text("Choose Mouse Button:");
        const char* mouse_button_items[] = { "Left", "Right", "Middle" };
        int mouse_button_index = static_cast<int>(mouse_button);
        if (ImGui::Combo("Mouse Button", &mouse_button_index, mouse_button_items, IM_ARRAYSIZE(mouse_button_items))) {
            mouse_button = static_cast<MouseButton>(mouse_button_index);
        }

        // Button to get mouse cursor position on the entire screen
        if (ImGui::Button("Get Screen Cursor Position")) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            // Get the window position on the screen
            int window_x, window_y;
            glfwGetWindowPos(window, &window_x, &window_y);

            // Calculate the cursor position relative to the screen
            double screen_x = xpos + window_x;
            double screen_y = ypos + window_y;

            std::cout << "Screen Cursor Position: (" << screen_x << ", " << screen_y << ")" << std::endl;
        }

        ImGui::End();
    }

public:
    Gui(int width, int height, const char* title)
        : window_width(width), window_height(height), window_title(title), window(nullptr), imgui_context(nullptr) {
        if (init_glfw() != 0) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        init_imgui();
        start_global_key_listener();
    }

    ~Gui() {
        stop_global_key_listener();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imgui_context);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void showSettings() {
        settingsWindow();
    }

    // Run the main application loop
    void run() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            render();
            glfwSwapBuffers(window);
        }
    }

    // Check if the window is open
    int is_window_open() {
        return !glfwWindowShouldClose(window);
    }
};