
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}/utils)

file(GLOB_RECURSE RAMBANK_FILES rambank/*.cpp)
add_library(rambank ${RAMBANK_FILES})

file(GLOB_RECURSE ROMBANK_FILES rombank/*.cpp)
add_library(rombank ${ROMBANK_FILES})

file(GLOB_RECURSE KBR2ASCCI_FILES kbr2ascii/*.cpp)
add_library(kbr2ascii ${KBR2ASCCI_FILES})


