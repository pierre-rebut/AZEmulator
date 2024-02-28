
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}/x86)

file(GLOB_RECURSE I8237_FILES intel8237/*.cpp)
add_library(intel8237 ${I8237_FILES})

file(GLOB_RECURSE I8253_FILES intel8253/*.cpp)
add_library(intel8253 ${I8253_FILES})

file(GLOB_RECURSE I8255_FILES intel8255/*.cpp)
add_library(intel8255 ${I8255_FILES})

file(GLOB_RECURSE I8259_FILES intel8259/*.cpp)
add_library(intel8259 ${I8259_FILES})