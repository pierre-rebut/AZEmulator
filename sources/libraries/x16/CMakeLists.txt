
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}/x16)

file(GLOB_RECURSE VIA_FILES via/*.cpp)
add_library(via ${VIA_FILES})

file(GLOB_RECURSE EMU_FILES emu/*.cpp)
add_library(emu ${EMU_FILES})

file(GLOB_RECURSE YAMAHA_FILES yamaha/*.cpp)
add_library(yamaha ${YAMAHA_FILES})

file(GLOB_RECURSE ROMBANK_FILES rombank/*.cpp)
add_library(rombankx16 ${ROMBANK_FILES})



