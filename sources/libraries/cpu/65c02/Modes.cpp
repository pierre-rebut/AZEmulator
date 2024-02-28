//
// Created by pierr on 14/01/2024.
//

#include "Core65c02.h"

namespace Astra::CPU::Lib::CPU {
    ///////////////////////////////////////////////////////////////////////////////
// ADDRESSING MODES

// The 6502 can address between 0x0000 - 0xFFFF. The high byte is often referred
// to as the "page", and the low byte is the offset into that page. This implies
// there are 256 pages, each containing 256 bytes.
//
// Several addressing modes have the potential to require an additional clock
// cycle if they cross a page boundary. This is combined with several instructions
// that enable this additional clock cycle. So each addressing function returns
// a flag saying it has potential, as does each instruction. If both instruction
// and address function return 1, then an additional clock cycle is required.


// Address Mode: Implied
// There is no additional data required for this instruction. The instruction
// does something very simple like like sets a status bit. However, we will
// target the accumulator, for instructions like PHA
    BYTE Core65c02::IMP() {
        fetched = a;
        return 0;
    }

// Address Mode: Immediate
// The instruction expects the next byte to be used as a value, so we'll prep
// the read address to point to the next byte
    BYTE Core65c02::IMM() {
        addr_abs = pc++;
        return 0;
    }


// Address Mode: Zero Page
// To save program bytes, zero page addressing allows you to absolutely address
// a location in first 0xFF bytes of address range. Clearly this only requires
// one byte instead of the usual two.
    BYTE Core65c02::ZP0() {
        addr_abs = m_mem->Fetch(DataFormat::Byte, pc);
        pc++;
        return 0;
    }


// Address Mode: Zero Page with X Offset
// Fundamentally the same as Zero Page addressing, but the contents of the X Register
// is added to the supplied single byte address. This is useful for iterating through
// ranges within the first page.
    BYTE Core65c02::ZPX() {
        addr_abs = (m_mem->Fetch(DataFormat::Byte, pc) + x) & 0xFF;
        pc++;
        return 0;
    }


// Address Mode: Zero Page with Y Offset
// Same as above but uses Y Register for offset
    BYTE Core65c02::ZPY() {
        addr_abs = (m_mem->Fetch(DataFormat::Byte, pc) + y) & 0xFF;
        pc++;
        return 0;
    }


// Address Mode: Relative
// This address mode is exclusive to branch instructions. The address
// must reside within -128 to +127 of the branch instruction, i.e.
// you cant directly branch to any address in the addressable range.
    BYTE Core65c02::REL() {
        addr_rel = m_mem->Fetch(DataFormat::Byte, pc);
        pc++;
        if (addr_rel & 0x80)
            addr_rel |= 0xFF00;
        return 0;
    }


// Address Mode: Absolute
// A full 16-bit address is loaded and used
    BYTE Core65c02::ABS() {
        addr_abs = m_mem->Fetch(DataFormat::Word, pc);
        pc += 2;

        return 0;
    }


// Address Mode: Absolute with X Offset
// Fundamentally the same as absolute addressing, but the contents of the X Register
// is added to the supplied two byte address. If the resulting address changes
// the page, an additional clock cycle is required
    BYTE Core65c02::ABX() {
        addr_abs = m_mem->Fetch(DataFormat::Word, pc);
        pc += 2;

        auto tmp = (addr_abs & 0xFF00);
        addr_abs += x;

        if ((addr_abs & 0xFF00) != tmp)
            return 1;
        else
            return 0;
    }


// Address Mode: Absolute with Y Offset
// Fundamentally the same as absolute addressing, but the contents of the Y Register
// is added to the supplied two byte address. If the resulting address changes
// the page, an additional clock cycle is required
    BYTE Core65c02::ABY() {
        addr_abs = m_mem->Fetch(DataFormat::Word, pc);
        pc += 2;

        auto tmp = (addr_abs & 0xFF00);
        addr_abs += y;

        if ((addr_abs & 0xFF00) != tmp)
            return 1;
        else
            return 0;
    }

// Note: The next 3 address modes use indirection (aka Pointers!)

// Address Mode: Indirect
// The supplied 16-bit address is read to get the actual 16-bit address. This is
// instruction is unusual in that it has a bug in the hardware! To emulate its
// function accurately, we also need to emulate this bug. If the low byte of the
// supplied address is 0xFF, then to read the high byte of the actual address
// we need to cross a page boundary. This doesnt actually work on the chip as
// designed, instead it wraps back around in the same page, yielding an
// invalid actual address
    BYTE Core65c02::IND() {
        using enum DataFormat;

        WORD ptr = m_mem->Fetch(Word, pc);
        pc += 2;

        WORD ptr2;
        if (!is65c02) {
            ptr2 = (ptr & 0xFF00) | ((ptr + 1) & 0x00FF); //replicate 6502 page-boundary wraparound bug
        } else {
            ptr2 = (ptr + 1) & 0xFFFF;
        }

        addr_abs = (WORD)m_mem->Fetch(Byte, ptr) | ((WORD)m_mem->Fetch(Byte, ptr2) << 8);

        return 0;
    }


// Address Mode: Indirect X
// The supplied 8-bit address is offset by X Register to index
// a location in page 0x00. The actual 16-bit address is read
// from this location
    BYTE Core65c02::IZX() {

        using enum Astra::CPU::DataFormat;

        WORD ptr = (((WORD)m_mem->Fetch(Byte, pc) + (WORD)x) & 0xFF); //zero-page wraparound for table pointer
        addr_abs = (WORD)m_mem->Fetch(Byte, ptr & 0xFF) | ((WORD)m_mem->Fetch(Byte, (ptr + 1) & 0xFF) << 8);
        pc++;

        return 0;
    }


// Address Mode: Indirect Y
// The supplied 8-bit address indexes a location in page 0x00. From
// here the actual 16-bit address is read, and the contents of
// Y Register is added to it to offset it. If the offset causes a
// change in page then an additional clock cycle is required.
    BYTE Core65c02::IZY() {
        using enum Astra::CPU::DataFormat;

        WORD eahelp = m_mem->Fetch(Byte, pc);
        pc++;

        WORD eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //zero-page wraparound
        addr_abs = (WORD)m_mem->Fetch(Byte, eahelp) | ((WORD)m_mem->Fetch(Byte, eahelp2) << 8);

        WORD startpage = addr_abs & 0xFF00;
        addr_abs += (WORD)y;

        if (startpage != (addr_abs & 0xFF00)) { //one cycle penlty for page-crossing on some opcodes
            return 1;
        }

        return 0;
    }

    ///// New modes for 65c02

    // Address Mode: Indirect without indexation
    BYTE Core65c02::IZ0() {
        using enum Astra::CPU::DataFormat;

        WORD tmp = m_mem->Fetch(Byte, pc);
        pc++;

        WORD tmp2 = (tmp & 0xFF00) | ((tmp + 1) & 0x00FF); //zero-page wraparound
        addr_abs = (WORD)m_mem->Fetch(Byte, tmp) | ((WORD)m_mem->Fetch(Byte, tmp2) << 8);

        return 0;
    }

    // Address Mode: Absolute indexed branch
    BYTE Core65c02::AINX() {
        using enum DataFormat;

        WORD ptr = m_mem->Fetch(Word, pc);
        pc += 2;

        ptr = (ptr + (WORD)x) & 0xFFFF;

        WORD ptr2;
        if (!is65c02) {
            ptr2 = (ptr & 0xFF00) | ((ptr + 1) & 0x00FF); //replicate 6502 page-boundary wraparound bug
        } else {
            ptr2 = ptr + 1; // the 65c02 doesn't have the bug
        }

        addr_abs = (WORD)m_mem->Fetch(Byte, ptr) | ((WORD)m_mem->Fetch(Byte, ptr2) << 8);

        return 0;
    }

    // zero-page, relative for branch ops (8-bit immediatel value, sign-extended)
    BYTE Core65c02::ZPREL() {
        addr_abs = m_mem->Fetch(DataFormat::Byte, pc);
        addr_rel = m_mem->Fetch(DataFormat::Byte, pc + 1);

        if (addr_rel & 0x80) {
            addr_rel |= 0xFF00;
        }

        pc += 2;
        return 0;
    }
}
