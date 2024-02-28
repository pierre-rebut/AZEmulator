//
// Created by pierr on 05/09/2023.
//

#pragma once

#include <array>
#include <cstdint>

#include <string>

#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"
#include "EngineLib/core/ICpuCore.h"

//flag modifier macros
#define setcarry() status |= FLAG_CARRY
#define clearcarry() status &= (~FLAG_CARRY)
#define setzero() status |= FLAG_ZERO
#define clearzero() status &= (~FLAG_ZERO)
#define setinterrupt() status |= FLAG_INTERRUPT
#define clearinterrupt() status &= (~FLAG_INTERRUPT)
#define setdecimal() status |= FLAG_DECIMAL
#define cleardecimal() status &= (~FLAG_DECIMAL)
#define setbreak() status |= FLAG_BREAK
#define clearbreak() status &= (~FLAG_BREAK)
#define setconstant() status |= FLAG_CONSTANT
#define clearconstant() status &= (~FLAG_CONSTANT)
#define setoverflow() status |= FLAG_OVERFLOW
#define clearoverflow() status &= (~FLAG_OVERFLOW)
#define setsign() status |= FLAG_SIGN
#define clearsign() status &= (~FLAG_SIGN)

//flag calculation macros
#define zerocalc(n) {\
    if ((n) & 0x00FF) clearzero();\
        else setzero();\
}

#define signcalc(n) {\
    if ((n) & 0x0080) setsign();\
        else clearsign();\
}

#define carrycalc(n) {\
    if ((n) & 0xFF00) setcarry();\
        else clearcarry();\
}

#define overflowcalc(n, m, o) { /* n = result, m = accumulator, o = memory */ \
    if (((n) ^ (WORD)(m)) & ((n) ^ (o)) & 0x0080) setoverflow();\
        else clearoverflow();\
}

namespace Astra::CPU::Lib::CPU {

    class Core65c02 : public ICpuCore
    {
    public:
        static constexpr const int FLAG_CARRY = 0x01;
        static constexpr const int FLAG_ZERO = 0x02;
        static constexpr const int FLAG_INTERRUPT = 0x04;
        static constexpr const int FLAG_DECIMAL = 0x08;
        static constexpr const int FLAG_BREAK = 0x10;
        static constexpr const int FLAG_CONSTANT = 0x20;
        static constexpr const int FLAG_OVERFLOW = 0x40;
        static constexpr const int FLAG_SIGN = 0x80;

    private:
        friend class Factory;

        struct INSTRUCTION
        {
            std::string name;
            BYTE (Core65c02::*operate )(void) = nullptr;
            BYTE (Core65c02::*addrmode)(void) = nullptr;
            BYTE cycles = 0;
        };

        static const std::array<INSTRUCTION, 256> OPCODE_TABLE;

        BYTE a = 0x00;        // Accumulator Register
        BYTE x = 0x00;        // X Register
        BYTE y = 0x00;        // Y Register
        BYTE stkp = 0x00;        // Stack Pointer (points to location on bus)
        WORD pc = 0x0000;    // Program Counter

        union {
            struct
            {
                bool C: 1 = false;    // Carry Bit
                bool Z: 1 = false;    // Zero
                bool I: 1 = false;    // Disable Interrupts
                bool D: 1 = false;    // Decimal Mode (unused in this implementation)
                bool B: 1 = false;    // Break
                bool U: 1 = false;    // Unused
                bool V: 1 = false;    // Overflow
                bool N: 1 = false;    // Negative
            } statusBit;
            BYTE status = 0;
        };       // Status Register

        Ref<IDevice> m_mem = nullptr;

        BYTE fetched = 0x00;   // Represents the working input value to the ALU
        WORD temp = 0x0000; // A convenience variable used everywhere
        WORD addr_abs = 0x0000; // All used memory addresses end up in here
        WORD addr_rel = 0x00;   // Represents absolute address following a branch
        BYTE opcode = 0x00;   // Is the instruction byte
        BYTE cycles = 0;       // Counts how many cycles the instruction has remaining

        bool is65c02 = true;

    public:
        inline bool IsInit() const override { return m_mem.operator bool(); }

        inline bool IsComplete() const override { return cycles == 0; }

        void Reset() override;
        void Execute() override;

        void Interrupt(bool isNmi, int interruptId) override;

        std::vector<int> DebugExecute() const override;

        bool UpdateHardParameters(const std::vector<int>& hardParameters) override;

    private:
        BYTE fetch();
        void push(BYTE val);

        BYTE pull8();
        WORD pull16();
        void push8(BYTE val);
        void push16(WORD val);

        /** 6502 modes */

        BYTE IMP();
        BYTE IMM();
        BYTE ZP0();
        BYTE ZPX();
        BYTE ZPY();
        BYTE REL();
        BYTE ABS();
        BYTE ABX();
        BYTE ABY();
        BYTE IND();
        BYTE IZX();
        BYTE IZY();

        /** 65c02 new modes */

        BYTE IZ0();
        BYTE AINX();
        BYTE ZPREL();

        /** 6502 instructions set */

        BYTE ADC();
        BYTE AND();
        BYTE ASL();
        BYTE BCC();
        BYTE BCS();
        BYTE BEQ();
        BYTE BITS();
        BYTE BMI();
        BYTE BNE();
        BYTE BPL();
        BYTE BRK();
        BYTE BVC();
        BYTE BVS();
        BYTE CLC();
        BYTE CLD();
        BYTE CLI();
        BYTE CLV();
        BYTE CMP();
        BYTE CPX();
        BYTE CPY();
        BYTE DEC();
        BYTE DEX();
        BYTE DEY();
        BYTE EOR();
        BYTE INC();
        BYTE INX();
        BYTE INY();
        BYTE JMP();
        BYTE JSR();
        BYTE LDA();
        BYTE LDX();
        BYTE LDY();
        BYTE LSR();
        BYTE NOP();
        BYTE ORA();
        BYTE PHA();
        BYTE PHP();
        BYTE PLA();
        BYTE PLP();
        BYTE ROL();
        BYTE ROR();
        BYTE RTI();
        BYTE RTS();
        BYTE SBC();
        BYTE SEC();
        BYTE SED();
        BYTE SEI();
        BYTE STA();
        BYTE STX();
        BYTE STY();
        BYTE TAX();
        BYTE TAY();
        BYTE TSX();
        BYTE TXA();
        BYTE TXS();
        BYTE TYA();

        /** 65c02 new instruction set */

        BYTE BRA();

        BYTE PHX();
        BYTE PHY();
        BYTE PLX();
        BYTE PLY();

        BYTE STZ();
        BYTE TSB();
        BYTE TRB();

        void bbr(WORD bitmask);

        BYTE BBR0();
        BYTE BBR1();
        BYTE BBR2();
        BYTE BBR3();
        BYTE BBR4();
        BYTE BBR5();
        BYTE BBR6();
        BYTE BBR7();

        void bbs(WORD bitmask);

        BYTE BBS0();
        BYTE BBS1();
        BYTE BBS2();
        BYTE BBS3();
        BYTE BBS4();
        BYTE BBS5();
        BYTE BBS6();
        BYTE BBS7();

        BYTE RMB0();
        BYTE RMB1();
        BYTE RMB2();
        BYTE RMB3();
        BYTE RMB4();
        BYTE RMB5();
        BYTE RMB6();
        BYTE RMB7();

        BYTE SMB0();
        BYTE SMB1();
        BYTE SMB2();
        BYTE SMB3();
        BYTE SMB4();
        BYTE SMB5();
        BYTE SMB6();
        BYTE SMB7();

        BYTE WAI();
        BYTE DBG();
    };

} // Astra
