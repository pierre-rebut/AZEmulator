//
// Created by pierr on 16/10/2023.
//

#include "CoreGamepad.h"
#include "EngineLib/data/KeyCodes.h"

namespace Astra::CPU::Lib::Nes {

    void CoreGamepad::Reset() {
        m_controller = 0;
        m_controllerState = 0;
    }

    void CoreGamepad::Execute() {
        if (const DWORD keyCode = m_keyboard->Fetch(DataFormat::DWord, 0)) {
            const bool isKeyDown = keyCode >> 16;
            if (isKeyDown) {
                setControllerKey((KeyCode) (keyCode & 0xFFFF));
            } else {
                unsetControllerKey((KeyCode) (keyCode & 0xFFFF));
            }
        }
    }

    void CoreGamepad::setControllerKey(KeyCode keyCode) {
        switch (keyCode) {
            case KeyCode::Up: {
                m_controller |= 0x08;
                break;
            }
            case KeyCode::Down: {
                m_controller |= 0x04;
                break;
            }
            case KeyCode::Left: {
                m_controller |= 0x02;
                break;
            }
            case KeyCode::Right: {
                m_controller |= 0x01;
                break;
            }
            case KeyCode::A: {
                m_controller |= 0x20;
                break;
            }
            case KeyCode::S: {
                m_controller |= 0x10;
                break;
            }
            case KeyCode::Z: {
                m_controller |= 0x80;
                break;
            }
            case KeyCode::X: {
                m_controller |= 0x40;
                break;
            }
            default:
                break;
        }
    }

    void CoreGamepad::unsetControllerKey(KeyCode keyCode) {

        switch (keyCode) {
            case KeyCode::Up: {
                m_controller &= ~0x08;
                break;
            }
            case KeyCode::Down: {
                m_controller &= ~0x04;
                break;
            }
            case KeyCode::Left: {
                m_controller &= ~0x02;
                break;
            }
            case KeyCode::Right: {
                m_controller &= ~0x01;
                break;
            }
            case KeyCode::A: {
                m_controller &= ~0x20;
                break;
            }
            case KeyCode::S: {
                m_controller &= ~0x10;
                break;
            }
            case KeyCode::Z: {
                m_controller &= ~0x80;
                break;
            }
            case KeyCode::X: {
                m_controller &= ~0x40;
                break;
            }
            default:
                break;
        }
    }

    LARGE CoreGamepad::Fetch(DataFormat, size_t address) {
        BYTE data = (m_controllerState & 0x80) > 0;
        m_controllerState <<= 1;

        return data;
    }

    void CoreGamepad::Push(DataFormat, size_t address, LARGE value) {
        m_controllerState = m_controller;
    }

    static const std::vector<std::pair<size_t, size_t>> addrList = {{1, 0}};
    const std::vector<std::pair<size_t, size_t>>* CoreGamepad::GetDeviceAddressList() const {
        return &addrList;
    }
}
