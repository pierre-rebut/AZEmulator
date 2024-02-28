set(LIB_NAME Shared)

set(LIB_SHARED_SRC
        ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_opengl3.cpp
        ${CMAKE_SOURCE_DIR}/vendor/imgui/misc/cpp/imgui_stdlib.cpp
        ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/vendor/stb_image/stb_image.cpp
        ${CMAKE_SOURCE_DIR}/vendor/tiny-file-dialogs/tinyfiledialogs.c
        ${CMAKE_SOURCE_DIR}/vendor/imnodes/imnodes.cpp
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
)

file(GLOB_RECURSE VENDOR_UTILS ${CMAKE_SOURCE_DIR}/vendor/utils/*.cpp)

set(LIB_INCLUDE_DIR
        ${CMAKE_SOURCE_DIR}/vendor/imgui
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
        ${CMAKE_SOURCE_DIR}/vendor/imgui/misc/cpp
        ${CMAKE_SOURCE_DIR}/vendor/stb_image
        ${CMAKE_SOURCE_DIR}/vendor/tiny-file-dialogs
        ${CMAKE_SOURCE_DIR}/vendor/imnodes
        ${CMAKE_SOURCE_DIR}/vendor/utils
        ${glfw_INCLUDE_DIR}
)

if (BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
else ()
    set(LIB_TYPE STATIC)
endif ()

include_directories(${LIB_INCLUDE_DIR})
add_library(${LIB_NAME} ${LIB_TYPE} ${LIB_SHARED_SRC} ${VENDOR_UTILS})
add_dependencies(${LIB_NAME} glfw-lib)
target_link_libraries(${LIB_NAME} PUBLIC glfw OpenGL::GL)
target_compile_definitions(${LIB_NAME} PUBLIC -DImTextureID=ImU64)
