
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}/nes)

file(GLOB_RECURSE PPU2C02_FILES ppu2C02/*.cpp)
add_library(ppu2C02 ${PPU2C02_FILES})

file(GLOB_RECURSE DMA_FILES dmaTransfert/*.cpp)
add_library(dmaTransfert ${DMA_FILES})

file(GLOB_RECURSE APU_FILES apu2A03/*.cpp)
add_library(apu2A03 ${APU_FILES})

file(GLOB_RECURSE CART_FILES cartNes/*.cpp)
add_library(cartNes ${CART_FILES})

file(GLOB_RECURSE GAMEPAD_FILES gamepad/*.cpp)
add_library(gamepad ${GAMEPAD_FILES})