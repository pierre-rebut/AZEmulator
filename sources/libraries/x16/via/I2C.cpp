//
// Created by pierr on 13/01/2024.
//
#include "I2C.h"
#include <iostream>

#define LOG_LEVEL 0

namespace Astra::CPU::Lib::X16 {
    void I2C::resetState() {
        state = STATE_STOP;
        read_mode = false;
        value = 0;
        count = 0;
    }

    BYTE I2C::read(BYTE device, BYTE offset) {
        BYTE value;
        switch (device) {
            case DEVICE_SMC:
                return smc.read(offset);
            case DEVICE_RTC:
                return rtc.read(offset);
            default:
                value = 0xff;
        }
#if LOG_LEVEL >= 1
        std::cout << std::hex << "I2C READ($" << device << ":$" << offset << ") = $" << value << std::endl;
#endif
        return value;
    }

    void I2C::write(BYTE device, BYTE offset, BYTE value) {
        switch (device) {
            case DEVICE_SMC:
                smc.write(offset, value);
                break;
            case DEVICE_RTC:
                rtc.write(offset, value);
                break;
                //        default:
                // no-op
        }
#if LOG_LEVEL >= 1
        std::cout << std::hex << "I2C WRITE($" << device << ":$" << offset << ") = $" << value << std::endl;
#endif
    }

    void I2C::step() {
        if (oldPort.clk_in != port.clk_in || oldPort.data_in != port.data_in) {
#if LOG_LEVEL >= 5
            printf("I2C(%d) C:%d D:%d\n", state, port.clk_in, port.data_in);
#endif
            if (state == STATE_STOP && port.clk_in == 0 && port.data_in == 0) {
#if LOG_LEVEL >= 2
                printf("I2C START\n");
#endif
                state = STATE_START;
            }

            if (state == 1 && port.clk_in == 1 &&port.data_in == 1 &&  oldPort.data_in == 0) {
#if LOG_LEVEL >= 2
                printf("I2C STOP\n");
#endif
                state = STATE_STOP;
                count = 0;
                read_mode = false;
            }

            if (state != STATE_STOP && port.clk_in == 1 && oldPort.clk_in == 0) {
                port.data_out = 1;
                if (state < 8) {
                    if (read_mode) {
                        if (state == 0) {
                            value = read(device, offset);
                        }
                        port.data_out = !!(value & 0x80);
                        value <<= 1;
#if LOG_LEVEL >= 4
                        printf("I2C OUT#%d: %d\n", state, port.data_out);
#endif
                        state++;
                    } else {
#if LOG_LEVEL >= 4
                        printf("I2C BIT#%d: %d\n", state, port.data_in);
#endif
                        value <<= 1;
                        value |= port.data_in;
                        state++;
                    }
                } else { // state == 8
                    if (read_mode) {
                        bool nack = port.data_in;
                        if (nack) {
#if LOG_LEVEL >= 3
                            printf("I2C OUT DONE (NACK)\n");
#endif
                            count = 0;
                            read_mode = false;
                        } else {
#if LOG_LEVEL >= 3
                            printf("I2C OUT DONE (ACK)\n");
#endif
                            if (!read_mode) offset++;							//Set I2C write bit by increasing offset by one; don't do that if we're in read_mode
                        }
                    } else {
                        bool ack = true;
                        switch (count) {
                            case 0:
                                device = value >> 1;
                                read_mode = value & 1;
                                if (device != DEVICE_SMC && device != DEVICE_RTC) {
                                    ack = false;
                                }
                                break;
                            case 1:
                                offset = value;
                                break;
                            default:
                                write(device, offset, value);
                                offset++;
                                break;
                        }
                        if (ack) {
#if LOG_LEVEL >= 3
                            std::cout << std::dec << "I2C ACK(" << count << ") $" << value << std::endl;
#endif
                            port.data_out = 0;
                            count++;
                        } else {
#if LOG_LEVEL >= 3
                            std::cout << std::dec << "I2C NACK(" << count << ") $" << value << std::endl;
#endif
                            count = 0;
                            read_mode = false;
                        }
                    }
                    state = STATE_START;
                }
            }
            oldPort = port;
        }
    }
}
