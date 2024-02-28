//
// Created by pierr on 19/01/2024.
//
#include <cstdint>
#include <iostream>
#include <iomanip>

#include "SdCard.h"

//#define VERBOSE 2

// MMC/SD command (SPI mode)
enum {
    CMD0   = 0,         // GO_IDLE_STATE
    CMD1   = 1,         // SEND_OP_COND
    ACMD41 = 0x80 | 41, // SEND_OP_COND (SDC)
    CMD8   = 8,         // SEND_IF_COND
    CMD9   = 9,         // SEND_CSD
    CMD10  = 10,        // SEND_CID
    CMD12  = 12,        // STOP_TRANSMISSION
    CMD13  = 13,        // SEND_STATUS
    ACMD13 = 0x80 | 13, // SD_STATUS (SDC)
    CMD16  = 16,        // SET_BLOCKLEN
    CMD17  = 17,        // READ_SINGLE_BLOCK
    CMD18  = 18,        // READ_MULTIPLE_BLOCK
    CMD23  = 23,        // SET_BLOCK_COUNT
    ACMD23 = 0x80 | 23, // SET_WR_BLK_ERASE_COUNT (SDC)
    CMD24  = 24,        // WRITE_BLOCK
    CMD25  = 25,        // WRITE_MULTIPLE_BLOCK
    CMD32  = 32,        // ERASE_WR_BLK_START
    CMD33  = 33,        // ERASE_WR_BLK_END
    CMD38  = 38,        // ERASE
    CMD55  = 55,        // APP_CMD
    CMD58  = 58,        // READ_OCR
};

namespace Astra::CPU::Lib::Monitors {

    void SdCard::reset() {
        rr = {
                0xff, // dummy
                0xff, // dummy
                0x00, // R1 response
                0xff, // dummy
                0xfe, // begin block
                0x40, // CSD_STRUCTURE [7:6] = 1, RESERVED [5:0] = 0
                0x0e, // TAAC = 0x0e
                0x00, // NSAC = 0x00
                0x32, // TRAN_SPEED = 0x32
                0x5b, // CCC (11:4)
                0x59, // CCC (3:0) [7-4], READ_BL_LEN [3-0]
                0x00, // READ_BL_PARTIAL [7], WRITE_BLK_MISALIGN [6], READ_BLK_MISALIGN [5], DSR_IMP [4], RESERVED [3:0]
                0x00, // RESERVED [7:6], C_SIZE(21:16) [5:0]
                0x00, // C_SIZE(15:8)
                0x00, // C_SIZE(7:0)
                0x7f, // RESERVED [7], ERASE_BLK_EN[6], SECTOR_SIZE(6:1) [5:0]
                0x80, // SECTOR_SIZE(0) [7], WP_GRP_SIZE [6:0]
                0x0a, // WP_GRP_ENABLE [7], RESERVED [6:5], R2W_FACTOR [4:2], WRITE_BL_LEN (3:2) [1:0]
                0x40, // WRITE_BL_LEN (1:0) [7:6], WRITE_BL_PARTIAL [5], RESERVED [4:0]
                0x00, // FILE_FORMAT_GRP [7] = 0, COPY [6], PERM_WRITE_PROTECT [5], TMP_WRITE_PROTECT [4], RESERVED [3:0]
                0x01 // CRC[7:1], ALWAYS_1 [0]
        };

        sdCardFile.reset();
        is_initialized = false;
    }

    void SdCard::select(bool select) {
        selected = select;
        rxbuf_idx = 0;
#if defined(VERBOSE) && VERBOSE >= 2
        std::cout << "*** SD card select: " << select << std::endl;
#endif
    }

    BYTE SdCard::handle(BYTE inbyte) {
        if (!selected || !sdCardFile.isAttached()) {
            return 0xFF;
        }

        BYTE outbyte = 0xFF;

        if (rxbuf_idx == 0 && inbyte == 0xFF) {
            // send response data
            if (response) {
                outbyte = response[response_counter++];
                if (response_counter == response_length) {
                    response = nullptr;
                }
            }

        } else {
            rxbuf[rxbuf_idx++] = inbyte;

            if ((rxbuf[0] & 0xC0) == 0x40 && rxbuf_idx == 6) {
                rxbuf_idx = 0;

                // Check for start-bit + transmission bit
                if ((rxbuf[0] & 0xC0) != 0x40) {
                    response = nullptr;
                    return 0xFF;
                }
                rxbuf[0] &= 0x3F;

                // Use upper command bit to indicate this is an ACMD
                if (is_acmd) {
                    rxbuf[0] |= 0x80;
                    is_acmd = false;
                }

                last_cmd = rxbuf[0];

#if defined(VERBOSE) && VERBOSE >= 2
                std::cout << "*** SD "  << ((rxbuf[0] & 0x80) ? "A" : "") << "CMD" << (int)(rxbuf[0] & 0x3F) << " -> Response:";
#endif
                switch (rxbuf[0]) {
                    case CMD0: {
                        // GO_IDLE_STATE: Resets the SD Memory Card
                        is_idle = true;
                        set_response_r1();
                        break;
                    }

                    case CMD8: {
                        // SEND_IF_COND: Sends SD Memory Card interface condition that includes host supply voltage
                        set_response_r7();
                        break;
                    }

                    case CMD9: {
                        // SEND_CSD: Sends card-specific data
                        set_response_csd();
                        break;
                    }

                    case ACMD41: {
                        // SD_SEND_OP_COND: Sends host capacity support information and activated the card's initialization process
                        is_idle = false;
                        is_initialized = true;
                        set_response_r1();
                        break;
                    }

                    case CMD13: {
                        // SEND_STATUS: Asks the selected card to send its status register
                        set_response_r2();
                        break;
                    }
                    case CMD16: {
                        // SET_BLOCKLEN: In case of non-SDHC card, this sets the block length. Block length of SDHC/SDXC cards are fixed to 512 bytes.
                        set_response_r1();
                        break;
                    }
                    case CMD17: {
                        // READ_SINGLE_BLOCK
                        DWORD lba = (rxbuf[1] << 24) | (rxbuf[2] << 16) | (rxbuf[3] << 8) | rxbuf[4];
                        static BYTE read_block_response[2 + 512 + 2];
                        read_block_response[0] = 0;
                        read_block_response[1] = 0xFE;
#ifdef VERBOSE
                        std::cout << "*** SD Reading LBA " << lba << std::endl;
#endif
                        if ((int64_t)lba * 512 >= sdCardFile.size()) {
                            read_block_response[1] = 0x08; // out of range
                            response_length = 2;
                        } else {
                            sdCardFile.seek((int64_t)lba * 512, AstraDiskSeekDir::XSEEK_SET);
                            int bytes_read = sdCardFile.read(&read_block_response[2], 1, 512);
                            if (bytes_read != 512) {
                                std::cout << "Warning: short read!" << std::endl;
                            }

                            response = read_block_response;
                            response_length = 2 + 512 + 2;
                        }
                        break;
                    }

                    case CMD24: {
                        // WRITE_BLOCK
                        lba = (rxbuf[1] << 24) | (rxbuf[2] << 16) | (rxbuf[3] << 8) | rxbuf[4];
                        if (rxbuf_idx > 4 && (int64_t)lba * 512 >= sdCardFile.size()) {
                            static BYTE bad_lba[2] = {0x00, 0x08};
                            response = bad_lba;
                            response_length = 2;
                        } else {
                            set_response_r1();
                        }
                        break;
                    }

                    case CMD55: {
                        // APP_CMD: Next command is an application specific command
                        is_acmd = true;
                        set_response_r1();
                        break;
                    }

                    case CMD58: {
                        // READ_OCR: Read the OCR register of the card
                        set_response_r3();
                        break;
                    }

                    default: {
                        set_response_r1();
                        break;
                    }
                }
                response_counter = 0;

#if defined(VERBOSE) && VERBOSE >= 2
                std::cout << std::hex;
                for (int i = 0; i < (response_length < 16 ? response_length : 16); i++) {
				std::cout << " " << std::setfill('0') << std::setw(2) << (int) response[i];
			}
			std::cout << std::dec << std::endl;
#endif

            } else if (rxbuf_idx == 515) {
                rxbuf_idx = 0;
                // Check for 'start block' byte
                if (last_cmd == CMD24 && rxbuf[0] == 0xFE) {
#ifdef VERBOSE
                    std::cout << "*** SD Writing LBA " << lba << std::endl;
#endif
                    if ((int64_t)lba * 512 >= sdCardFile.size()) {
                        // do nothing?
                    } else {
                        sdCardFile.seek((int64_t)lba * 512, AstraDiskSeekDir::XSEEK_SET);
                        int bytes_written = sdCardFile.write(rxbuf + 1, 1, 512);
                        if (bytes_written != 512) {
                            std::cout << "Warning: short write!" << std::endl;
                        }
                    }
                }
            }
        }
        return outbyte;
    }
}
