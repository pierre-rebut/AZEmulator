//
// Created by pierr on 13/01/2024.
//
#include <iostream>
#include "Smc.h"

#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Lib::X16 {
    void Smc::reset() {
        smc_requested_reset = false;
        keyboard.reset();
        mouse.reset();
        mouse_device_id = 3;
    }

    BYTE Smc::read(BYTE a) {
        BYTE mouse_id;
        BYTE mouse_size;

        switch (a) {
            // Offset that returns one byte from the keyboard buffer
            case 7:
                return keyboard.next();

                // Offset that returns three bytes from mouse buffer (one movement packet) or a single zero if there is not complete packet in the buffer
                // mse_count keeps track of which one of the three bytes it's sending
            case 0x21:
                mouse_id = mouse_device_id;
                if (mouse_id == 3 || mouse_id == 4) {
                    mouse_size = 4;
                } else {
                    mouse_size = 3;
                }

                if (mse_count == 0 && mouse.count() >= mouse_size) {        // If start of packet, check if there are at least three bytes in the buffer
                    mse_count++;
                    return mouse.next();
                } else if (mse_count >
                           0) {                                // If we have already started sending bytes, assume there is enough data in the buffer
                    mse_count++;
                    if (mse_count == mouse_size) mse_count = 0;
                    return mouse.next();
                } else {                                                    // Return a single zero if no complete packet available
                    mse_count = 0;
                    return 0x00;
                }

            case 0x22:
                return mouse_device_id;

            default:
                return 0xff;
        }
    }

    void Smc::write(BYTE a, BYTE v) {
        switch (a) {
            case 1:
                if (v == 0) {
                    std::cout << "SMC Power Off." << std::endl;
                } else if (v == 1) {
                    smc_requested_reset = true;
                }
                break;
            case 2:
                if (v == 0) {
                    smc_requested_reset = true;
                }
                break;
            case 3:
                if (v == 0) {
                    sendNmi = true;
                }
                break;
            case 4:
                // TODO power LED
                break;
            case 5:
                activity_led = v;
                break;

            case 0x20: {
                switch (v) {
                    case 0:
                    case 3:
                        mouse_device_id = v;
                        break;
                    case 4:
                        mouse_device_id = 3;
                        break;
                    default:
                        mouse_device_id = 0;
                        break;
                }
                mouse.reset();
                break;
            }
            default:
                break;
        }
    }

    void Smc::step() {
        if (const DWORD keyCode = m_keyboard->Fetch(DataFormat::DWord, 0)) {
            const bool isKeyDown = keyCode >> 16;
            int x16KeyCode = keyCodeToX16KeyCode((KeyCode) (keyCode & 0xFFFF));
            if (isKeyDown) {
                if (x16KeyCode & EXTENDED_FLAG) {
                    keyboard.add(0x7f);
                }
                keyboard.add(x16KeyCode & 0xff);
            } else {
                x16KeyCode = x16KeyCode | 0b10000000;
                if (x16KeyCode & EXTENDED_FLAG) {
                    keyboard.add(0xff);
                }
                keyboard.add(x16KeyCode & 0xff);
            }
        }

        if (const LARGE mouseCode = m_mouse->Fetch(DataFormat::Large, 0)) {

            const BYTE mouseBtn = mouseCode & 0xFF;
            short mouseX = (mouseCode >> 8) & 0xFFFF;
            short mouseY = (mouseCode >> 24) & 0xFFFF;
            char wheel = (mouseCode >> 40) & 0xFF;

            do {
                int send_diff_x = mouseX > 255 ? 255 : (mouseX < -256 ? -256 : mouseX);
                int send_diff_y = mouseY > 255 ? 255 : (mouseY < -256 ? -256 : mouseY);

                mouseSend(send_diff_x, send_diff_y, mouseBtn & 0b111, wheel);

                mouseX -= send_diff_x;
                mouseY -= send_diff_y;
                wheel = 0;
            } while (mouseX != 0 && mouseY != 0);
        }
    }

    void Smc::mouseSend(int x, int y, BYTE button, char wheel) {
        auto size = 3;
        if (mouse_device_id == 3 || mouse_device_id == 4) {
            size = 4;
        }

        if (mouse.count() >= BUFFER_SIZE - size) {
            return;
        }

        BYTE byte0 =
                ((y >> 9) & 1) << 5 |
                ((x >> 9) & 1) << 4 |
                1 << 3 |
                button;

        mouse.add(byte0);
        mouse.add(x);
        mouse.add(y);

        if (mouse_device_id == 3 || mouse_device_id == 4) {
            mouse.add(wheel);
        }
    }

    int Smc::keyCodeToX16KeyCode(KeyCode code) {
        switch (code) {
            using
            enum KeyCode;
            case GraveAccent:
                return 1;
            case Backspace:
                return 15;
            case Tab:
                return 16;
            case Enter:
                return 43;
            case Pause:
                return 126;
            case Escape:
#ifdef ESC_IS_BREAK
                return 126;
#else
                return 110;
#endif
            case Space:
                return 61;
            case Apostrophe:
                return 41;
            case Comma:
                return 53;
            case Minus:
                return 12;
            case Period:
                return 54;
            case Slash:
                return 55;
            case D0:
                return 11;
            case D1:
                return 2;
            case D2:
                return 3;
            case D3:
                return 4;
            case D4:
                return 5;
            case D5:
                return 6;
            case D6:
                return 7;
            case D7:
                return 8;
            case D8:
                return 9;
            case D9:
                return 10;
            case Semicolon:
                return 40;
            case Equal:
                return 13;
            case LeftBracket:
                return 27;
            case Backslash:
                return 29;
            case RightBracket:
                return 28;
            case A:
                return 31;
            case B:
                return 50;
            case C:
                return 48;
            case D:
                return 33;
            case E:
                return 19;
            case F:
                return 34;
            case G:
                return 35;
            case H:
                return 36;
            case I:
                return 24;
            case J:
                return 37;
            case K:
                return 38;
            case L:
                return 39;
            case M:
                return 52;
            case N:
                return 51;
            case O:
                return 25;
            case P:
                return 26;
            case Q:
                return 17;
            case R:
                return 20;
            case S:
                return 32;
            case T:
                return 21;
            case U:
                return 23;
            case V:
                return 49;
            case W:
                return 18;
            case X:
                return 47;
            case Y:
                return 22;
            case Z:
                return 46;
            case Delete:
                return 76;
            case Up:
                return 83;
            case Down:
                return 84;
            case Right:
                return 89;
            case Left:
                return 79;
            case Insert:
                return 75;
            case Home:
                return 80;
            case End:
                return 81;
            case PageUp:
                return 85;
            case PageDown:
                return 86;
            case F1:
                return 112;
            case F2:
                return 113;
            case F3:
                return 114;
            case F4:
                return 115;
            case F5:
                return 116;
            case F6:
                return 117;
            case F7:
                return 118;
            case F8:
                return 119;
            case F9:
                return 120;
            case F10:
                return 121;
            case F11:
                return 122;
            case F12:
                return 123;
            case ScrollLock:
                return 125;
            case RightShift:
                return 57;
            case LeftShift:
                return 44;
            case CapsLock:
                return 30;
            case LeftControl:
                return 58;
            case RightControl:
                return 64;
            case LeftAlt:
                return 60;
            case RightAlt:
                return 62;
            case LeftSuper:
                return 59;
            case RightSuper:
                return 63;
            case Menu: // Menu
                return 65;
            case KPEnter:
                return 108;
            case KP0:
                return 99;
            case KP1:
                return 93;
            case KP2:
                return 98;
            case KP3:
                return 103;
            case KP4:
                return 92;
            case KP5:
                return 97;
            case KP6:
                return 102;
            case KP7:
                return 91;
            case KP8:
                return 96;
            case KP9:
                return 101;
            case KPDecimal:
                return 104;
            case KPAdd:
                return 106;
            case KPSubtract:
                return 105;
            case KPMultiply:
                return 100;
            case KPDivide:
                return 95;
            case NumLock:
                return 90;
            default:
                return 0;
        }
    }

    //////   RingBuffer

    void Smc::RingBuffer::add(BYTE value) {
        BYTE next = (head + 1) & (BUFFER_SIZE - 1);        //Next available index
        if (next != tail) {                                //next = tail => buffer full
            buffer[head] = value;                    //Set new value
            head = next;                                //Update buffer head pointer
        }
    }

    BYTE Smc::RingBuffer::next() {
        BYTE value = 0;                                    //Prepare to return 0 of buffer is empty
        if (head != tail) {                            //head = tail => empty
            value = buffer[tail];                    //Get value
            tail = (tail + 1) & (BUFFER_SIZE - 1);        //Update buffer tail pointer
        }
        return value;
    }

    void Smc::RingBuffer::reset() {
        tail = head = 0;
    }

    int Smc::RingBuffer::count() const {
        return (BUFFER_SIZE + head - tail) & (BUFFER_SIZE - 1);
    }
}
