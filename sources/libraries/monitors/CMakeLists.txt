
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}/monitors)

file(GLOB_RECURSE SCREEN_FILES worstscreen/*.cpp)
add_library(worstscreen ${SCREEN_FILES})

file(GLOB_RECURSE M6845_FILES motorola6845/*.cpp)
add_library(motorola6845 ${M6845_FILES})

file(GLOB_RECURSE VERA_FILES vera/*.cpp)
add_library(vera ${VERA_FILES})