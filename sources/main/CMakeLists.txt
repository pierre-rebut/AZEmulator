
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_OUTPUT_DIR})

find_package(OpenGL REQUIRED)

include(${CMAKE_SOURCE_DIR}/vendor/external_projects.cmake)
include(${CMAKE_SOURCE_DIR}/vendor/external_libs.cmake)

include_directories(
        .
        ${CMAKE_SOURCE_DIR}/includes
        ${LIB_INCLUDE_DIR}
        ${spdlog_INCLUDE_DIR}
        ${yaml-cpp_INCLUDE_DIR}
        ${glfw_INCLUDE_DIR}
)

file(GLOB_RECURSE COMMONS_SOURCES Commons/*.cpp)
file(GLOB_RECURSE ENGINE_SOURCES CpuEngine/*.cpp)
file(GLOB_RECURSE VIEWER_SOURCES ViewerApp/*.cpp)

if (WIN32)
    set(VIEWER_SOURCES ${VIEWER_SOURCES} app.rc)
    #set(WIN32_APP WIN32)
endif ()

if (ENABLE_OPTICKCORE)
    set(OPTICKCORE_DEPS OptickCore-lib)
    set(OPTICKCORE_LIB OptickCore)
    set(PROFILING_MODE PROFILING_OPTICK OPTICK_EXPORTS)
elseif (ENABLE_PROFILING)
    set(PROFILING_MODE PROFILING_ENABLE)
else ()
    set(PROFILING_MODE)
endif ()

if(ENABLE_EXPORT_TRAD)
    set(EXPORT_TRAD EXPORT_TRAD)
else ()
    set(EXPORT_TRAD)
endif ()

## use WIN32 in add_executable(NAME WIN32 SOURCES) to disable console
## VIEWER
add_executable(${NAME} ${WIN32_APP} ${VIEWER_SOURCES} ${ENGINE_SOURCES} ${COMMONS_SOURCES})
add_dependencies(${NAME} spdlog-lib yaml-cpp-lib ${OPTICKCORE_DEPS})
target_link_libraries(${NAME} PUBLIC spdlog yaml-cpp Shared ${OPTICKCORE_LIB})
target_compile_definitions(${NAME} PUBLIC ASTRA_VERSION=${VERSION} PROJECT_NAME="${NAME}" ${PROFILING_MODE} LOG_ENABLE ${EXPORT_TRAD})

######## Config #####
add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/appData/
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})