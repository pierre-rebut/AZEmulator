//
// Created by pierr on 14/01/2024.
//
#include "Serial.h"

#define MHZ 8
#define printf(...)

#define ACPTR(v) -2
#define LISTEN(v)
#define UNLSN()
#define UNTLK()
#define TALK(v)
#define SECOND(v)
#define TKSA(v)
#define CIOUT(v)

namespace Astra::CPU::Lib::X16 {

    void Serial::reset() {
        in.atn = 0;
        in.clk = 0;
        in.data = 0;
        out.clk = 1;
        out.data = 1;
    }

    static BYTE readByte(bool *eoi)
    {
        BYTE byte;
        int ret = ACPTR(&byte);
        *eoi = ret >= 0;
        return byte;
    }

    void Serial::step(int clocks) {
        bool print = false;

        if (old_atn == in.atn &&
            old_clk == readClk() &&
            old_data == readData()) {
            clocks_since_last_change += clocks;
            if (state == 2 && valid == true && bit == 0 && clocks_since_last_change > 200 * MHZ) {
                if (clocks_since_last_change < (200 + 60) * MHZ) {
                    printf("XXX EOI ACK\n");
                    out.data = 0;
                    eoi = true;
                } else {
                    printf("XXX EOI ACK END\n");
                    out.data = 1;
                    clocks_since_last_change = 0;
                }
                print = true;
            }
            if (state == 10 && clocks_since_last_change > 60 * MHZ) {
                out.clk = 1;
                state = 11;
                clocks_since_last_change = 0;
                print = true;
            } else if (state == 11 && readData() && !fnf) {
                clocks_since_last_change = 0;
                byte = readByte(&eoi);
                bit = 0;
                valid = true;
                if (eoi) {
                    printf("XXXEOI1\n");
                    state = 12;
                } else {
                    printf("XXXEOI0\n");
                    out.clk = 0;
                    state = 13;
                }
                print = true;
            } else if (state == 12 && clocks_since_last_change > 512 * MHZ) {
                // EOI delay
                // XXX we'd have to check for the ACK
                clocks_since_last_change = 0;
                out.clk = 0;
                state = 13;
                print = true;
            } else if (state == 13 && clocks_since_last_change > 60 * MHZ) {
                if (valid) {
                    // send bit
                    out.data = (byte >> bit) & 1;
                    out.clk = 1;
                    printf("*** BIT%d OUT: %d\n", bit, out.data);
                    bit++;
                    if (bit == 8) {
                        state = 14;
                    }
                } else {
                    out.clk = 0;
                }
                valid = !valid;
                clocks_since_last_change = 0;
                print = true;
            } else if (state == 14 && clocks_since_last_change > 60 * MHZ) {
                out.data = 1;
                out.clk = 0;
                state = 10;
                clocks_since_last_change = 0;
                print = true;
            }
        } else {
            clocks_since_last_change = 0;

            printf("-SERIAL IN { ATN:%d CLK:%d DATA:%d }\n", in.atn, old_clk, old_data);
            printf("+SERIAL IN { ATN:%d CLK:%d DATA:%d } --- IN { CLK:%d DATA:%d } OUT { CLK:%d DATA:%d } -- #%d\n", in.atn, readClk(), readData(), in.clk, in.data, out.clk, out.data, state);

            if (!during_atn && in.atn) {
                out.data = 0;
                state = 99;
                during_atn = true;
                printf("XXX START OF ATN\n");
            }

            switch(state) {
                case 0:
                    break;
                case 99:
                    // wait for CLK=0
                    if (!readClk()) {
                        state = 1;
                    }
                    break;
                case 1:
                    if (during_atn && !in.atn) {
                        // cancelled ATN
                        out.data = 1;
                        out.clk = 1;
                        during_atn = false;
                        printf("*** END OF ATN\n");
                        if (listening) {
                            // keep holding DATA to indicate we're here
                            out.data = 0;
                            printf("XXX START OF DATA RECEIVE\n");
                        } else if (talking) {
                            out.clk = 0;
                            state = 10;
                            printf("XXX START OF DATA SEND\n");
                        } else {
                            state = 0;
                        }
                        break;
                    }
                    // wait for CLK=1
                    if (readClk()) {
                        out.data = 1;
                        state = 2;
                        valid = true;
                        bit = 0;
                        byte = 0;
                        eoi = false;
                        printf("XXX START OF BYTE\n");
                    }
                    break;
                case 2:
                    if (during_atn && !in.atn) {
                        // cancelled ATN
                        out.data = 1;
                        out.clk = 1;
                        printf("*** XEND OF ATN\n");
                        state = 0;
                        break;
                    }
                    if (valid) {
                        // wait for CLK=0, data not valid
                        if (!readClk()) {
                            valid = false;
                            printf("XXX NOT VALID\n");
                        }
                    } else {
                        // wait for CLK=1, data valid
                        if (readClk()) {
                            bool b = readData();
                            byte |= (b << bit);
                            printf("*** BIT%d IN: %d\n", bit, b);
                            valid = true;
                            if (++bit == 8) {
                                printf("*** %s BYTE IN: %02x%s\n", during_atn ? "ATN" : "DATA", byte, eoi ? " (EOI)" : "");
                                if (during_atn) {
                                    printf("IEEE CMD %x\n", byte);
                                    switch (byte & 0x60) {
                                        case 0x20:
                                            if (byte == 0x3f) {
                                                UNLSN();
                                                listening = false;
                                            } else {
                                                LISTEN(byte);
                                                listening = true;
                                            }
                                            break;
                                        case 0x40:
                                            if (byte == 0x5f) {
                                                UNTLK();
                                                talking = false;
                                            } else {
                                                TALK(byte);
                                                talking = true;
                                            }
                                            break;
                                        case 0x60:
                                            if (listening) {
                                                SECOND(byte);
                                            } else { // talking
                                                TKSA(byte);
                                            }
                                            break;
                                    }
                                } else {
                                    CIOUT(byte);
                                }
                                out.data = 0;
                                state = 1;
                            }
                        }
                    }
                    break;
                case 3:
                    break;
            }
            print = true;
        }

        if (print) {
            printf(">SERIAL IN { ATN:%d CLK:%d DATA:%d } --- IN { CLK:%d DATA:%d } OUT { CLK:%d DATA:%d } -- #%d\n", in.atn, readClk(), readData(), in.clk, in.data, out.clk, out.data, state);
        }

        old_atn = in.atn;
        old_clk = readClk();
        old_data = readData();
    }

    bool Serial::readClk() const {
        return out.clk & in.clk;
    }

    bool Serial::readData() const {
        return out.data & in.data;
    }
}
