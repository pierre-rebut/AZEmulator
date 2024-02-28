include(ExternalProject)

function(build_lib lib_name header_file final_lib)
    ExternalProject_Add(${lib_name}-lib
            SOURCE_DIR ${CMAKE_SOURCE_DIR}/vendor/${lib_name}
            PREFIX ${CMAKE_BINARY_DIR}/vendor/${lib_name}-prefix
            BUILD_COMMAND ${CMAKE_COMMAND} --build . --target ${lib_name}
            UPDATE_COMMAND ""
            INSTALL_COMMAND ""
            BUILD_BYPRODUCTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${final_lib}
            CMAKE_ARGS
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            )

    set(INC_VAR "${lib_name}_INCLUDE_DIR")
    find_path(${INC_VAR}
            NAMES ${header_file}
            PATHS
            "${CMAKE_SOURCE_DIR}/vendor/${lib_name}"
            "${CMAKE_SOURCE_DIR}/vendor/${lib_name}/include"
            NO_CMAKE_FIND_ROOT_PATH
            NO_DEFAULT_PATH)

    add_library(${lib_name} SHARED IMPORTED)
    add_dependencies(${lib_name} "${lib_name}-lib")
    set_target_properties(
            ${lib_name}
            PROPERTIES
            IMPORTED_CONFIGURATIONS ${CMAKE_BUILD_TYPE}
            IMPORTED_LOCATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${final_lib}
            IMPORTED_IMPLIB ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${final_lib}
            INTERFACE_INCLUDE_DIRECTORIES ${${INC_VAR}})
endfunction()

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(MODE "d")
endif ()

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "")
endif ()

if (WIN32)
    message(STATUS "Win32 lib")
    if (ENABLE_OPTICKCORE)
        build_lib(OptickCore "optick.h" libOptickCore.dll)
    endif ()
    build_lib(spdlog "spdlog/spdlog.h" libspdlog${MODE}.dll)
    build_lib(yaml-cpp "yaml-cpp/yaml.h" libyaml-cpp${MODE}.dll)
    build_lib(glfw "GLFW/glfw3.h" glfw3.dll)
elseif (UNIX)
    message(STATUS "Linux lib")
    if (ENABLE_OPTICKCORE)
        build_lib(OptickCore "optick.h" libOptickCore.so)
    endif ()
    build_lib(spdlog "spdlog/spdlog.h" libspdlog${MODE}.so)
    build_lib(yaml-cpp "yaml-cpp/yaml.h" libyaml-cpp${MODE}.so)
    build_lib(glfw "GLFW/glfw3.h" libglfw.so)
endif ()


message(STATUS "include dirs : ${glfw_INCLUDE_DIR} ${spdlog_INCLUDE_DIR} ${yaml-cpp_INCLUDE_DIR} ${OptickCore_INCLUDE_DIR}")
