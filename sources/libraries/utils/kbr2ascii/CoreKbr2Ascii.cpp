//
// Created by pierr on 01/02/2024.
//
#include "CoreKbr2Ascii.h"
#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Lib::Utils {
    void CoreKbr2Ascii::Reset() {
        m_shift = false;
        m_ctrl = false;
        m_charList = {};
    }

    void CoreKbr2Ascii::Execute() {
        if (const DWORD keyCode = m_keyboard->Fetch(DataFormat::DWord, 0)) {
            const bool isKeyDown = keyCode >> 16;
            const auto keyVal = (KeyCode) (keyCode & 0xFFFF);

            switch (keyVal) {
                case KeyCode::LeftShift:
                    m_shift = isKeyDown;
                    break;
                case KeyCode::LeftControl:
                    m_ctrl = isKeyDown;
                    break;
                default: {
                    if (isKeyDown) {
                        insertKeyToBuffer(keyVal);
                    }
                    break;
                }
            }
        }
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{2, 0}};
    const std::vector<std::pair<size_t, size_t>>* CoreKbr2Ascii::GetDeviceAddressList() const {
        return &addrList;
    }

    LARGE CoreKbr2Ascii::Fetch(DataFormat fmt, size_t address) {
        if (address == 1) {
            return m_ctrl;
        }

        if (m_charList.empty()) {
            return 0;
        }

        auto ret = m_charList.front();
        m_charList.pop();
        return ret;
    }



    void CoreKbr2Ascii::insertKeyToBuffer(KeyCode keyCode) {
        if (m_charList.size() > 1024) {
            return;
        }

        switch (keyCode) {
            case KeyCode::Enter: {
                m_charList.emplace(0xD);
                m_charList.emplace('\n');
                break;
            }
            default:
                m_charList.emplace(keyCodeToAscii(keyCode));
                break;
        }
    }

    LARGE CoreKbr2Ascii::keyCodeToAscii(KeyCode code) {
        switch (code) {
            case KeyCode::Space: return ' ';
            case KeyCode::Apostrophe: return '\'';
            case KeyCode::Comma: return ',';
            case KeyCode::Minus: return '-';
            case KeyCode::Slash: return '/';
            case KeyCode::D0: return m_shift ? '0' : '@';
            case KeyCode::D1: return m_shift ? '1' : '&';
            case KeyCode::D2: return m_shift ? '2' : '~';
            case KeyCode::D3: return m_shift ? '3' : '"';
            case KeyCode::D4: return m_shift ? '4' : '\'';
            case KeyCode::D5: return m_shift ? '5' : '(';
            case KeyCode::D6: return m_shift ? '6' : '-';
            case KeyCode::D7: return m_shift ? '7' : '\\';
            case KeyCode::D8: return m_shift ? '8' : '_';
            case KeyCode::D9: return m_shift ? '9' : '^';
            case KeyCode::Semicolon: return '%';
            case KeyCode::Equal: return m_shift ? '+' : '=';
            case KeyCode::A: return isUpper() ? 'A' : 'a';
            case KeyCode::B: return isUpper() ? 'B' : 'b';
            case KeyCode::D: return isUpper() ? 'D' : 'd';
            case KeyCode::E: return isUpper() ? 'E' : 'e';
            case KeyCode::F: return isUpper() ? 'F' : 'f';
            case KeyCode::G: return isUpper() ? 'G' : 'g';
            case KeyCode::H: return isUpper() ? 'H' : 'h';
            case KeyCode::I: return isUpper() ? 'I' : 'i';
            case KeyCode::J: return isUpper() ? 'J' : 'j';
            case KeyCode::K: return isUpper() ? 'K' : 'k';
            case KeyCode::L: return isUpper() ? 'L' : 'l';
            case KeyCode::M: return isUpper() ? 'M' : 'm';
            case KeyCode::N: return isUpper() ? 'N' : 'n';
            case KeyCode::O: return isUpper() ? 'O' : 'o';
            case KeyCode::P: return isUpper() ? 'P' : 'p';
            case KeyCode::Q: return isUpper() ? 'Q' : 'q';
            case KeyCode::R: return isUpper() ? 'R' : 'r';
            case KeyCode::S: return isUpper() ? 'S' : 's';
            case KeyCode::T: return isUpper() ? 'T' : 't';
            case KeyCode::U: return isUpper() ? 'U' : 'u';
            case KeyCode::V: return isUpper() ? 'V' : 'v';
            case KeyCode::W: return isUpper() ? 'W' : 'w';
            case KeyCode::X: return isUpper() ? 'X' : 'x';
            case KeyCode::Y: return isUpper() ? 'Y' : 'y';
            case KeyCode::Z: return isUpper() ? 'Z' : 'z';
            case KeyCode::Backspace: return '\b';
            case KeyCode::C: {
                if (m_ctrl) {
                    return 3;
                }

                return isUpper() ? 'C' : 'c';
            }
            default:
                return 0;
        }
    }

    bool CoreKbr2Ascii::UpdateHardParameters(const std::vector<int>& hardParameters) {
        m_onlyUpper = hardParameters.at(0);
        return true;
    }
}
