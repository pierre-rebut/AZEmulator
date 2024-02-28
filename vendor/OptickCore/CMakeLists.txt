cmake_minimum_required(VERSION 3.2)
project(Optick)

set(CMAKE_CXX_STANDARD 11)

# Sources
file(GLOB OPTICK_SRC "src/*.*")

# OptickCore
add_library(OptickCore SHARED ${OPTICK_SRC})
target_link_libraries(OptickCore wsock32)
target_compile_definitions(OptickCore PRIVATE OPTICK_EXPORTS=1 OPTICK_ENABLE_GPU=0)
