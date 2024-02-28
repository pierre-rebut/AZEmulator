//
// Created by pierr on 16/03/2023.
//

#include "GlfwWindow.h"
#include "Commons/Log.h"
#include "Commons/AstraException.h"
#include "ViewerApp/CoreLib/Events/ApplicationEvent.h"

#include "ViewerApp/CoreLib/CoreEngine.h"
#include "GlfwImage.h"
#include "ViewerApp/CoreLib/Events/KeyEvents.h"
#include "imgui_impl_glfw.h"
#include "ViewerApp/CoreLib/Events/MouseEvent.h"

#include <utility>

void disableTitlebar(GLFWwindow* window);

namespace Astra::UI::Core::Glfw {
    static void glfw_error_callback(int error, const char* description) {
        LOG_WARN("GlfwWindow Error {}: {}", error, description);
    }

    GlfwWindow::GlfwWindow(std::string pWindowName) : AWindow(std::move(pWindowName)) {
        // Setup GLFW window
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            throw AstraException("GlfwWindow: Init failed");
        }
    }

    GlfwWindow::~GlfwWindow() {
        if (m_glfwWindow) {
            glfwDestroyWindow(m_glfwWindow);
        }
        glfwTerminate();
    }

    void GlfwWindow::initWindow() {
        LOG_DEBUG("GlfwWindow: initWindow");

        auto monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        glfwDefaultWindowHints();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

        m_glfwWindow = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, title.c_str(), nullptr, nullptr);
        AstraException::assertV(m_glfwWindow, "GlfwWindow: failed to create window");

        disableTitlebar(m_glfwWindow);
        glfwMakeContextCurrent(m_glfwWindow);
        glfwSwapInterval(1);

        glfwSetWindowPos(m_glfwWindow, 200, 200);
        glfwMaximizeWindow(m_glfwWindow);

        GlfwImage icon("resources/icons/AppIcon.png");
        glfwSetWindowIcon(m_glfwWindow, 1, &icon.image);

        initCallbacks();

        LOG_DEBUG("GlfwWindow: initWindow end");
    }

    void GlfwWindow::initCallbacks() {

        glfwSetWindowCloseCallback(m_glfwWindow, [](GLFWwindow*) {
            UI::Core::Events::Get().OnEvent<WindowCloseEvent>();
        });

        glfwSetWindowFocusCallback(m_glfwWindow, [](GLFWwindow*, int pFocused) {
            UI::Core::Events::Get().OnEvent<WindowFocusEvent>(pFocused);
        });

        glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow*, int pKey, int, int pAction, int) {
            switch (pAction) {
                case GLFW_PRESS: {
                    UI::Core::Events::Get().OnEvent<KeyPressedEvent>((KeyCode) pKey);
                    break;
                }
                case GLFW_RELEASE: {
                    UI::Core::Events::Get().OnEvent<KeyReleasedEvent>((KeyCode) pKey);
                    break;
                }
                case GLFW_REPEAT: {
                    UI::Core::Events::Get().OnEvent<KeyPressedEvent>((KeyCode) pKey, true);
                    break;
                }
                default:break;
            }
        });

        glfwSetDropCallback(m_glfwWindow, [](GLFWwindow*, int count, const char** paths){
            auto dropEvent = CreateRef<WindowDropEvent>();
            for (int i = 0; i < count; i++) {
                dropEvent->AddPath(paths[i]);
            }
            UI::Core::Events::Get().PushEvent(dropEvent);
        });

        glfwSetMouseButtonCallback(m_glfwWindow, [](GLFWwindow *pWindow, int pButton, int pAction, int mods) {
            switch (pAction) {
                case GLFW_PRESS: {
                    UI::Core::Events::Get().OnEvent<MouseButtonPressedEvent>((MouseCode) pButton);
                    break;
                }
                case GLFW_RELEASE: {
                    UI::Core::Events::Get().OnEvent<MouseButtonReleasedEvent>((MouseCode) pButton);
                    break;
                }
                default:break;
            }
        });

        glfwSetScrollCallback(m_glfwWindow, [](GLFWwindow *pWindow, double xOffset, double yOffset) {
            UI::Core::Events::Get().OnEvent<MouseScrolledEvent>((float) xOffset, (float) yOffset);
        });

        glfwSetCursorPosCallback(m_glfwWindow, [](GLFWwindow *pWindow, double xPos, double yPos) {
            UI::Core::Events::Get().OnEvent<MouseMovedEvent>((float) xPos, (float) yPos);
        });
    }

    bool GlfwWindow::isWindowMaximized() const {
        return (bool) glfwGetWindowAttrib(m_glfwWindow, GLFW_MAXIMIZED);
    }

    void GlfwWindow::maximizeWindow() const {
        glfwMaximizeWindow(m_glfwWindow);
    }

    void GlfwWindow::restoreWindow() const {
        glfwRestoreWindow(m_glfwWindow);
    }

    void GlfwWindow::iconifyWindow() const {
        glfwIconifyWindow(m_glfwWindow);
    }

    void GlfwWindow::getWindowPos(int& pPosX, int& pPosY) const {
        glfwGetWindowPos(m_glfwWindow, &pPosX, &pPosY);
    }

    void GlfwWindow::getWindowSize(int& pSizeX, int& pSizeY) const {
        glfwGetWindowSize(m_glfwWindow, &pSizeX, &pSizeY);
    }

    void GlfwWindow::setWindowPos(int pPosX, int pPosY) const {
        glfwSetWindowPos(m_glfwWindow, pPosX, pPosY);
    }

    void GlfwWindow::setWindowSize(int pSizeX, int pSizeY) const {
        glfwSetWindowSize(m_glfwWindow, pSizeX, pSizeY);
    }

    void GlfwWindow::initImGui() {
        ImGui_ImplGlfw_InitForOpenGL(m_glfwWindow, true);
    }

    void GlfwWindow::shutdownImGui() {
        ImGui_ImplGlfw_Shutdown();
    }

    void GlfwWindow::newFrameImGui() {
        ImGui_ImplGlfw_NewFrame();
    }

    void GlfwWindow::endFrameImGui() {
        glfwSwapBuffers(m_glfwWindow);
    }

    void GlfwWindow::viewportImGui() {
        glfwMakeContextCurrent(m_glfwWindow);
    }
}
