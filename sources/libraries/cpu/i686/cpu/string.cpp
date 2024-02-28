#include "cpu.h"

int CpuInternal::movsb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::movsb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::movsw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::movsw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::movsd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::movsd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        {
            DWORD addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        }
        {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        }
        m_reg32[6] += add;
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::stosb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, src = m_reg8[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::stosb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, src = m_reg8[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::stosw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, src = m_reg16[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::stosw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, src = m_reg16[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::stosd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, src = m_reg32[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::stosd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, src = m_reg32[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::scasb16(int flags)
{
    int     count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1;
    BYTE dest = m_reg8[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else {
                    src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            m_reg16[14] += add;
            m_lr   = (int8_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB8;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::scasb32(int flags)
{
    int     count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1;
    BYTE dest = m_reg8[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else {
                    src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            m_reg32[7] += add;
            m_lr   = (int8_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB8;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::scasw16(int flags)
{
    int      count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2;
    WORD dest = m_reg16[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg16[14] += add;
            m_lr   = (int16_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB16;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::scasw32(int flags)
{
    int      count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2;
    WORD dest = m_reg16[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg32[7] += add;
            m_lr   = (int16_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB16;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::scasd16(int flags)
{
    int      count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4;
    DWORD dest = m_reg32[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else {
                    src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            m_reg16[14] += add;
            m_lr   = (int32_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB32;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::scasd32(int flags)
{
    int      count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4;
    DWORD dest = m_reg32[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else {
                    src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            m_reg32[7] += add;
            m_lr   = (int32_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB32;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::insb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 8 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_inb(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_inb(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::insb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 8 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_inb(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_inb(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
            {
                m_ram->Push(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::insw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 16 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_inw(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_inw(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::insw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 16 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_inw(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_inw(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                m_ram->Push(DataFormat::Word, m_tlb[addr_ >> 12] + addr_, data_);
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::insd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 32 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_ind(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_ind(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::insd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 32 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_ind(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_ind(m_reg16[4]);
        do {
            DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else {
                m_ram->Push(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_, data_);
            }
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::outsb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 8 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        cpu_outb(m_reg16[4], src);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        cpu_outb(m_reg16[4], src);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::outsb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 8 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        cpu_outb(m_reg16[4], src);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        cpu_outb(m_reg16[4], src);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::outsw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 16 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outw(m_reg16[4], src);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outw(m_reg16[4], src);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::outsw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 16 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outw(m_reg16[4], src);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outw(m_reg16[4], src);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::outsd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 32 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        cpu_outd(m_reg16[4], src);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        cpu_outd(m_reg16[4], src);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::outsd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 32 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        cpu_outd(m_reg16[4], src);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else {
                src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
            }
        } while (0);
        cpu_outd(m_reg16[4], src);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::cmpsb16(int flags)
{
    int     count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, seg_base = m_seg_base[flags >> 22 & 7];
    BYTE dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else {
                    src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            m_reg16[14] += add;
            m_reg16[12] += add;
            m_lr   = (int8_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB8;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::cmpsb32(int flags)
{
    int     count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, seg_base = m_seg_base[flags >> 22 & 7];
    BYTE dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else {
                    src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            m_reg32[7] += add;
            m_reg32[6] += add;
            m_lr   = (int8_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB8;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::cmpsw16(int flags)
{
    int      count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, seg_base = m_seg_base[flags >> 22 & 7];
    WORD dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg16[14] += add;
            m_reg16[12] += add;
            m_lr   = (int16_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB16;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::cmpsw32(int flags)
{
    int      count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, seg_base = m_seg_base[flags >> 22 & 7];
    WORD dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg32[7] += add;
            m_reg32[6] += add;
            m_lr   = (int16_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB16;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::cmpsd16(int flags)
{
    int      count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, seg_base = m_seg_base[flags >> 22 & 7];
    DWORD dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else {
                    dest = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            do {
                DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else {
                    src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            m_reg16[14] += add;
            m_reg16[12] += add;
            m_lr   = (int32_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB32;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else {
                        dest = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else {
                        dest = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::cmpsd32(int flags)
{
    int      count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, seg_base = m_seg_base[flags >> 22 & 7];
    DWORD dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else {
                    dest = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            do {
                DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else {
                    src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                }
            } while (0);
            m_reg32[7] += add;
            m_reg32[6] += add;
            m_lr   = (int32_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB32;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else {
                        dest = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else {
                        dest = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                do {
                    DWORD addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else {
                        src = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
                    }
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }

    util_abort();
    return 0;
}
int CpuInternal::lodsb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg8[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg8[0] = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg8[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg8[0] = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::lodsb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg8[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg8[0] = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg8[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg8[0] = m_ram->Fetch(DataFormat::Byte, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::lodsw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg16[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg16[0] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg16[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg16[0] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::lodsw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg16[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg16[0] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg16[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg16[0] = m_ram->Fetch(DataFormat::Word, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CpuInternal::lodsd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg32[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg32[0] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg32[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg32[0] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CpuInternal::lodsd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg32[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg32[0] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            DWORD addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg32[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg32[0] = m_ram->Fetch(DataFormat::DWord, m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
