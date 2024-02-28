//
// Created by pierr on 20/10/2023.
//

#include "CoreCart.h"
#include "Mapper_000.h"
#include "Mapper_001.h"

#include <iostream>

namespace Astra::CPU::Lib {
    CoreCart::CoreCart() {
        CoreCart::Reset();
    }

    void CoreCart::Reset() {
        m_cartCurrentAddress = 2; // skip status and size

        flags.isImageReady = false;
        m_mapper = nullptr;
        nMapperID = 0;
        nPRGBanks = 0;
        nCHRBanks = 0;
        vPRGMemory.clear();
        vCHRMemory.clear();

        cartSlotRead();

        if (m_mapper) {
            m_mapper->Reset();
        }
    }

    void CoreCart::Execute() {
        if (m_mapper->irqState()) {
            m_mapper->irqClear();
            m_deviceService->SendDeviceInterrupt(1);
        }
    }

    LARGE CoreCart::Fetch(DataFormat fmt, size_t address) {
        if (fmt == DataFormat::Byte) {
            return getByte(address);
        } else {
            return (WORD) getByte(address) | ((WORD) getByte(address + 1) << 8);
        }
    }

    BYTE CoreCart::getByte(size_t address) {
        if (!m_mapper) {
            return 0;
        }

        size_t mapped_addr = 0;
        BYTE data = 0;

        if (address < 0x4000) {
            if (m_mapper->ppuRead(address, mapped_addr)) {
                data = vCHRMemory.at(mapped_addr);
            }
        } else if (address == 0x4000) {
            MIRROR m = m_mapper->Mirror();
            if (m == MIRROR::HARDWARE) {
                m = hw_mirror;
            }
            data = (BYTE) m;
        } else {
            if (m_mapper->cpuRead(address, mapped_addr, data)) {
                if (mapped_addr != 0xFFFFFFFF) {
                    data = vPRGMemory.at(mapped_addr);
                }
            }
        }

        return data;
    }

    void CoreCart::Push(DataFormat fmt, size_t address, LARGE value) {
        if (fmt == DataFormat::Byte) {
            setByte(address, value & 0xFF);
        } else {
            setByte(address, value & 0xFF);
            setByte(address + 1, (value >> 8) & 0xFF);
        }
    }

    void CoreCart::setByte(size_t address, BYTE value) {
        if (!m_mapper) {
            return;
        }

        size_t mapped_addr = 0;

        if (address < 0x4000) {
            if (m_mapper->ppuWrite(address, mapped_addr)) {
                vCHRMemory.at(mapped_addr) = value;
            }
        } else if (address == 0x4000) {
            m_mapper->ScanLine();
        } else {
            if (m_mapper->cpuWrite(address, mapped_addr, value)) {
                if (mapped_addr != 0xFFFFFFFF) {
                    vPRGMemory.at(mapped_addr) = value;
                }
            }
        }
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{0x4001, 0},
                                                                    {0x8000, 0x8000}};

    const std::vector<std::pair<size_t, size_t>>* CoreCart::GetDeviceAddressList() const {
        return &addrList;
    }

    struct CartHeader
    {
        char name[4];
        BYTE prg_rom_chunks;
        BYTE chr_rom_chunks;
        BYTE mapper1;
        BYTE mapper2;
        BYTE prg_ram_size;
        BYTE tv_system1;
        BYTE tv_system2;
        char unused[5];
    };

    bool CoreCart::cartSlotRead() {
        if (!m_cartSlot || m_cartSlot->Fetch(DataFormat::Byte, 0) == 0) {
            return false;
        }

        // iNES Format Header
        CartHeader header{0};

        readFromDisk((char*) &header, sizeof(CartHeader));

        if (header.name[0] != 'N' || header.name[1] != 'E' || header.name[2] != 'S') {
            return false;
        }

        // If a "trainer" exists we just need to read past
        // it before we get to the good stuff
        if (header.mapper1 & 0x04) {
            std::cout << "past trainer" << std::endl;
            m_cartSlot->Push(DataFormat::Word, 2, 512 + sizeof(CartHeader));
        }

        // Determine Mapper ID
        nMapperID = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
        hw_mirror = (header.mapper1 & 0x01) ? MIRROR::VERTICAL : MIRROR::HORIZONTAL;

        // "Discover" File Format
        BYTE nFileType = 1;
        if ((header.mapper2 & 0x0C) == 0x08) { nFileType = 2; }

        if (nFileType == 1) {
            nPRGBanks = header.prg_rom_chunks;
            vPRGMemory.resize(nPRGBanks * 16384);
            readFromDisk((char*) vPRGMemory.data(), vPRGMemory.size());

            nCHRBanks = header.chr_rom_chunks;
            if (nCHRBanks == 0) {
                // Create CHR RAM
                vCHRMemory.resize(8192);
            } else {
                // Allocate for ROM
                vCHRMemory.resize(nCHRBanks * 8192);
            }
            readFromDisk((char*) vCHRMemory.data(), vCHRMemory.size());
        } else if (nFileType == 2) {
            nPRGBanks = ((header.prg_ram_size & 0x07) << 8) | header.prg_rom_chunks;
            vPRGMemory.resize(nPRGBanks * 16384);
            readFromDisk((char*) vPRGMemory.data(), vPRGMemory.size());

            nCHRBanks = ((header.prg_ram_size & 0x38) << 8) | header.chr_rom_chunks;
            vCHRMemory.resize(nCHRBanks * 8192);
            readFromDisk((char*) vCHRMemory.data(), vCHRMemory.size());
        }

        // Load appropriate mapper
        switch (nMapperID) {
            case 0:
                m_mapper = std::make_shared<Mapper_000>(nPRGBanks, nCHRBanks);
                break;
            case 1:
                m_mapper = std::make_shared<Mapper_001>(nPRGBanks, nCHRBanks);
                break;
                /*case 2:
                    m_mapper = std::make_shared<Mapper_002>(nPRGBanks, nCHRBanks);
                    break;
                case 3:
                    m_mapper = std::make_shared<Mapper_003>(nPRGBanks, nCHRBanks);
                    break;
                case 4:
                    m_mapper = std::make_shared<Mapper_004>(nPRGBanks, nCHRBanks);
                    break;
                case 66:
                    m_mapper = std::make_shared<Mapper_066>(nPRGBanks, nCHRBanks);
                    break;
                */
            default:
                break;
        }

        flags.isImageReady = true;
        return true;
    }

    int CoreCart::FetchReadOnly(size_t address) const {
        if (!m_mapper) {
            return -1;
        }

        if (address <= 0x1FFF) {
            return (int) vCHRMemory[address];
        } else if (address >= 0x8000) {
            address &= (nPRGBanks > 1 ? 0x7FFF : 0x3FFF);
            return (int) vPRGMemory[address];
        } else {
            return -1;
        }
    }

    void CoreCart::readFromDisk(char* buffer, int bufferSize) {
        for (int i = 0; i < bufferSize; i++) {
            *(buffer + i) = (BYTE) m_cartSlot->Fetch(DataFormat::Byte, m_cartCurrentAddress);
            m_cartCurrentAddress++;
        }
    }
}
