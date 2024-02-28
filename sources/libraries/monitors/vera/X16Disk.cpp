//
// Created by pierr on 19/01/2024.
//

#include "X16Disk.h"

namespace Astra::CPU::Lib {
    void X16Disk::reset() {
        if (!m_diskDevice || m_diskDevice->Fetch(DataFormat::Byte, 0) == 0) {
            m_isReady = false;
            return;
        }

        m_diskSize = m_diskDevice->Fetch(DataFormat::Byte, 1);
        m_diskPos = 0;

        m_isReady = true;
    }

    size_t X16Disk::read(BYTE* data, size_t data_size, size_t data_count) {
        if(!m_isReady) {
            return 0;
        }

        size_t i;
        for (i = 0; i < data_count; i++) {
            if (m_diskPos == m_diskSize) {
                break;
            }

            data[i] = m_diskDevice->Fetch(DataFormat::Byte, m_diskPos + 2);
            m_diskPos++;
        }

        return i;
    }

    size_t X16Disk::write(const BYTE* data, size_t data_size, size_t data_count) {
        if(!m_isReady) {
            return 0;
        }

        size_t i;
        for (i = 0; i < data_count; i++) {
            if (m_diskPos == m_diskSize) {
                break;
            }

            m_diskDevice->Push(DataFormat::Byte, m_diskPos + 2, data[i]);
            m_diskPos++;
        }

        return i;
    }

    int X16Disk::seek(size_t pos, AstraDiskSeekDir origin) {
        if(!m_isReady) {
            return -1;
        }

        switch(origin) {
            case AstraDiskSeekDir::XSEEK_SET:
                m_diskPos = (pos > m_diskSize) ? m_diskSize : pos;
                break;
            case AstraDiskSeekDir::XSEEK_CUR:
                m_diskPos += pos;
                if(m_diskPos > m_diskSize) {
                    m_diskPos = m_diskSize;
                } else if (m_diskPos < 0) {
                    m_diskPos = 0;
                }
                break;
            case AstraDiskSeekDir::XSEEK_END:
                m_diskPos = m_diskSize - pos;
                if(m_diskPos < 0) {
                    m_diskPos = 0;
                }
                break;
            default:
                break;
        }

        return m_diskPos;
    }
}
