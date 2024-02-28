set(LIB_OUTPUT_DIR ${APP_OUTPUT_DIR}/CoreLibrary)

include_directories(${CMAKE_SOURCE_DIR}/includes)

add_subdirectory(cpu)
add_subdirectory(nes)
add_subdirectory(x86)
add_subdirectory(x686)
add_subdirectory(monitors)
add_subdirectory(utils)
add_subdirectory(x16)
