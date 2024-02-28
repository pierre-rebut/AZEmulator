//
// Created by pierr on 14/01/2024.
//

#include "Core65c02.h"

namespace Astra::CPU::Lib::CPU {
    ///////////////////////////////////////////////////////////////////////////////
// INSTRUCTION IMPLEMENTATIONS

// Note: Ive started with the two most complicated instructions to emulate, which
// ironically is addition and subtraction! Ive tried to include a detailed
// explanation as to why they are so complex, yet so fundamental. Im also NOT
// going to do this through the explanation of 1 and 2's complement.

// Instruction: Add with Carry In
// Function:    A = A + M + C
// Flags Out:   C, V, N, Z
//
// Explanation:
// The purpose of this function is to add a value to the accumulator and a carry bit. If
// the result is > 255 there is an overflow setting the carry bit. Ths allows you to
// chain together ADC instructions to add numbers larger than 8-bits. This in itself is
// simple, however the 6502 supports the concepts of Negativity/Positivity and Signed Overflow.
//
// 10000100 = 128 + 4 = 132 in normal circumstances, we know this as unsigned and it allows
// us to represent numbers between 0 and 255 (given 8 bits). The 6502 can also interpret
// this word as something else if we assume those 8 bits represent the range -128 to +127,
// i.e. it has become signed.
//
// Since 132 > 127, it effectively wraps around, through -128, to -124. This wraparound is
// called overflow, and this is a useful to know as it indicates that the calculation has
// gone outside the permissable range, and therefore no longer makes numeric sense.
//
// Note the implementation of ADD is the same in binary, this is just about how the numbers
// are represented, so the word 10000100 can be both -124 and 132 depending upon the
// context the programming is using it in. We can prove this!
//
//  10000100 =  132  or  -124
// +00010001 = + 17      + 17
//  ========    ===       ===     See, both are valid additions, but our interpretation of
//  10010101 =  149  or  -107     the context changes the value, not the hardware!
//
// In principle under the -128 to 127 range:
// 10000000 = -128, 11111111 = -1, 00000000 = 0, 00000000 = +1, 01111111 = +127
// therefore negative numbers have the most significant set, positive numbers do not
//
// To assist us, the 6502 can set the overflow flag, if the result of the addition has
// wrapped around. V <- ~(A^M) & A^(A+M+C) :D lol, let's work out why!
//
// Let's suppose we have A = 30, M = 10 and C = 0
//          A = 30 = 00011110
//          M = 10 = 00001010+
//     RESULT = 40 = 00101000
//
// Here we have not gone out of range. The resulting significant bit has not changed.
// So let's make a truth table to understand when overflow has occurred. Here I take
// the MSB of each component, where R is RESULT.
//
// A  M  R | V | A^R | A^M |~(A^M) |
// 0  0  0 | 0 |  0  |  0  |   1   |
// 0  0  1 | 1 |  1  |  0  |   1   |
// 0  1  0 | 0 |  0  |  1  |   0   |
// 0  1  1 | 0 |  1  |  1  |   0   |  so V = ~(A^M) & (A^R)
// 1  0  0 | 0 |  1  |  1  |   0   |
// 1  0  1 | 0 |  0  |  1  |   0   |
// 1  1  0 | 1 |  1  |  0  |   1   |
// 1  1  1 | 0 |  0  |  0  |   1   |
//
// We can see how the above equation calculates V, based on A, M and R. V was chosen
// based on the following hypothesis:
//       Positive Number + Positive Number = Negative Result -> Overflow
//       Negative Number + Negative Number = Positive Result -> Overflow
//       Positive Number + Negative Number = Either Result -> Cannot Overflow
//       Positive Number + Positive Number = Positive Result -> OK! No Overflow
//       Negative Number + Negative Number = Negative Result -> OK! NO Overflow

    BYTE Core65c02::ADC() {
        fetch();

        if (is65c02 && (status & FLAG_DECIMAL)) {

            WORD tmp = ((WORD) a & 0x0F) + (fetched & 0x0F) + (WORD) (status & FLAG_CARRY);
            WORD tmp2 = ((WORD) a & 0xF0) + (fetched & 0xF0);

            if (tmp > 0x09) {
                tmp2 += 0x10;
                tmp += 0x06;
            }
            if (tmp2 > 0x90) {
                tmp2 += 0x60;
            }

            carrycalc(tmp2)
            temp = (tmp & 0x0F) | (tmp2 & 0xF0);

            cycles++;
        } else {
            temp = (WORD) a + (WORD) fetched + (WORD) (status & FLAG_CARRY);

            carrycalc(temp);
            overflowcalc(temp, a, fetched);
        }

        zerocalc(temp);
        signcalc(temp);

        a = temp & 0x00FF;

        // This instruction has the potential to require an additional clock cycle
        return 1;
    }


// Instruction: Subtraction with Borrow In
// Function:    A = A - M - (1 - C)
// Flags Out:   C, V, N, Z
//
// Explanation:
// Given the explanation for ADC above, we can reorganise our data
// to use the same computation for addition, for subtraction by multiplying
// the data by -1, i.e. make it negative
//
// A = A - M - (1 - C)  ->  A = A + -1 * (M - (1 - C))  ->  A = A + (-M + 1 + C)
//
// To make a signed positive number negative, we can invert the bits and add 1
// (OK, I lied, a little bit of 1 and 2s complement :P)
//
//  5 = 00000101
// -5 = 11111010 + 00000001 = 11111011 (or 251 in our 0 to 255 range)
//
// The range is actually unimportant, because if I take the value 15, and add 251
// to it, given we wrap around at 256, the result is 10, so it has effectively
// subtracted 5, which was the original intention. (15 + 251) % 256 = 10
//
// Note that the equation above used (1-C), but this got converted to + 1 + C.
// This means we already have the +1, so all we need to do is invert the bits
// of M, the data(!) therfore we can simply add, exactly the same way we did
// before.

    BYTE Core65c02::SBC() {
        fetch();

        if (is65c02 && (status & FLAG_DECIMAL)) {
            temp = (WORD) a - (fetched & 0x0f) + (status & FLAG_CARRY) - 1;

            if ((temp & 0x0f) > (a & 0x0f)) {
                temp -= 6;
            }

            temp -= (fetched & 0xf0);
            if ((temp & 0xfff0) > ((WORD) a & 0xf0)) {
                temp -= 0x60;
            }

            if (temp <= (WORD) a) {
                setcarry();
            } else {
                clearcarry();
            }

            cycles++;
        } else {
            WORD value = ((WORD) fetched) ^ 0x00FF;
            temp = (WORD) a + value + (WORD)(status & FLAG_CARRY);

            carrycalc(temp);
            overflowcalc(temp, a, value);
        }

        zerocalc(temp);
        signcalc(temp);

        a = temp & 0xFF;
        return 1;
    }

// OK! Complicated operations are done! the following are much simpler
// and conventional. The typical order of events is:
// 1) Fetch the data you are working with
// 2) Perform calculation
// 3) Store the result in desired place
// 4) Set Flags of the status register
// 5) Return if instruction has potential to require additional
//    clock cycle


// Instruction: Bitwise Logic AND
// Function:    A = A & M
// Flags Out:   N, Z
    BYTE Core65c02::AND() {
        fetch();
        a = a & fetched;

        zerocalc(a);
        signcalc(a);

        return 1;
    }


// Instruction: Arithmetic Shift Left
// Function:    A = C <- (A << 1) <- 0
// Flags Out:   N, Z, C
    BYTE Core65c02::ASL() {
        fetch();
        temp = (WORD) fetched << 1;

        carrycalc(temp);
        zerocalc(temp);
        signcalc(temp);

        push(temp & 0xFF);
        return 0;
    }


// Instruction: Branch if Carry Clear
// Function:    if(C == 0) pc = address
    BYTE Core65c02::BCC() {
        if ((status & FLAG_CARRY) == 0) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }

        return 0;
    }


// Instruction: Branch if Carry Set
// Function:    if(C == 1) pc = address
    BYTE Core65c02::BCS() {
        if ((status & FLAG_CARRY) == FLAG_CARRY) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }

        return 0;
    }


// Instruction: Branch if Equal
// Function:    if(Z == 1) pc = address
    BYTE Core65c02::BEQ() {
        if ((status & FLAG_ZERO) == FLAG_ZERO) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }

        return 0;
    }

    BYTE Core65c02::BITS() {
        fetch();
        temp = a & fetched;

        zerocalc(temp);

        // 65C02 BIT #$xx only affects Z  See: http://6502.org/tutorials/65c02opcodes.html#2
        if (opcode != 0x89) {
            status = (status & 0x3F) | (fetched & 0xC0);
        }

        return 0;
    }


// Instruction: Branch if Negative
// Function:    if(N == 1) pc = address
    BYTE Core65c02::BMI() {
        if ((status & FLAG_SIGN) == FLAG_SIGN) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }
        return 0;
    }


// Instruction: Branch if Not Equal
// Function:    if(Z == 0) pc = address
    BYTE Core65c02::BNE() {
        if ((status & FLAG_ZERO) == 0) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }
        return 0;
    }


// Instruction: Branch if Positive
// Function:    if(N == 0) pc = address
    BYTE Core65c02::BPL() {
        if ((status & FLAG_SIGN) == 0) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }
        return 0;
    }

// Instruction: Break
// Function:    Program Sourced Interrupt
    BYTE Core65c02::BRK() {
        pc++;

        push16(pc);
        push8(status | FLAG_BREAK);

        setinterrupt();
        cleardecimal();

        pc = m_mem->Fetch(DataFormat::Word, 0xFFFE);

        return 0;
    }


// Instruction: Branch if Overflow Clear
// Function:    if(V == 0) pc = address
    BYTE Core65c02::BVC() {
        if ((status & FLAG_OVERFLOW) == 0) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }
        return 0;
    }


// Instruction: Branch if Overflow Set
// Function:    if(V == 1) pc = address
    BYTE Core65c02::BVS() {
        if ((status & FLAG_OVERFLOW) == FLAG_OVERFLOW) {
            cycles++;
            addr_abs = pc + addr_rel;

            if ((addr_abs & 0xFF00) != (pc & 0xFF00))
                cycles++;

            pc = addr_abs;
        }
        return 0;
    }


// Instruction: Clear Carry Flag
// Function:    C = 0
    BYTE Core65c02::CLC() {
        clearcarry();
        return 0;
    }


// Instruction: Clear Decimal Flag
// Function:    D = 0
    BYTE Core65c02::CLD() {
        cleardecimal();
        return 0;
    }


// Instruction: Disable Interrupts / Clear Interrupt Flag
// Function:    I = 0
    BYTE Core65c02::CLI() {
        clearinterrupt();
        return 0;
    }


// Instruction: Clear Overflow Flag
// Function:    V = 0
    BYTE Core65c02::CLV() {
        clearoverflow();
        return 0;
    }

// Instruction: Compare Accumulator
// Function:    C <- A >= M      Z <- (A - M) == 0
// Flags Out:   N, C, Z
    BYTE Core65c02::CMP() {
        fetch();
        temp = (WORD) a - (WORD) fetched;

        if (a >= fetched)
            setcarry();
        else
            clearcarry();

        zerocalc(temp);
        signcalc(temp);

        return 1;
    }


// Instruction: Compare X Register
// Function:    C <- X >= M      Z <- (X - M) == 0
// Flags Out:   N, C, Z
    BYTE Core65c02::CPX() {
        fetch();
        temp = (WORD) x - (WORD) fetched;

        if (x >= fetched)
            setcarry();
        else
            clearcarry();

        zerocalc(temp);
        signcalc(temp);

        return 0;
    }


// Instruction: Compare Y Register
// Function:    C <- Y >= M      Z <- (Y - M) == 0
// Flags Out:   N, C, Z
    BYTE Core65c02::CPY() {
        fetch();
        temp = (WORD) y - (WORD) fetched;

        if (y >= fetched)
            setcarry();
        else
            clearcarry();

        zerocalc(temp);
        signcalc(temp);

        return 0;
    }


// Instruction: Decrement Value at Memory Location
// Function:    M = M - 1
// Flags Out:   N, Z
    BYTE Core65c02::DEC() {
        fetch();
        temp = fetched - 1;

        zerocalc(temp);
        signcalc(temp);

        push(temp & 0xFF);
        return 0;
    }


// Instruction: Decrement X Register
// Function:    X = X - 1
// Flags Out:   N, Z
    BYTE Core65c02::DEX() {
        x--;

        zerocalc(x);
        signcalc(x);

        return 0;
    }


// Instruction: Decrement Y Register
// Function:    Y = Y - 1
// Flags Out:   N, Z
    BYTE Core65c02::DEY() {
        y--;

        zerocalc(y);
        signcalc(y);

        return 0;
    }


// Instruction: Bitwise Logic XOR
// Function:    A = A xor M
// Flags Out:   N, Z
    BYTE Core65c02::EOR() {
        fetch();
        temp = (WORD)a ^ (WORD)fetched;

        zerocalc(temp);
        signcalc(temp);

        a = temp & 0xFF;
        return 1;
    }


// Instruction: Increment Value at Memory Location
// Function:    M = M + 1
// Flags Out:   N, Z
    BYTE Core65c02::INC() {
        fetch();
        temp = (WORD)fetched + 1;

        zerocalc(temp);
        signcalc(temp);

        push(temp & 0xFF);
        return 0;
    }


// Instruction: Increment X Register
// Function:    X = X + 1
// Flags Out:   N, Z
    BYTE Core65c02::INX() {
        x++;

        zerocalc(x);
        signcalc(x);

        return 0;
    }


// Instruction: Increment Y Register
// Function:    Y = Y + 1
// Flags Out:   N, Z
    BYTE Core65c02::INY() {
        y++;

        zerocalc(y);
        signcalc(y);

        return 0;
    }


// Instruction: Jump To Location
// Function:    pc = address
    BYTE Core65c02::JMP() {
        pc = addr_abs;
        return 0;
    }


// Instruction: Jump To Sub-Routine
// Function:    Push current pc to stack, pc = address
    BYTE Core65c02::JSR() {
        push16(pc - 1);
        pc = addr_abs;
        return 0;
    }


// Instruction: Load The Accumulator
// Function:    A = M
// Flags Out:   N, Z
    BYTE Core65c02::LDA() {
        fetch();
        a = fetched;

        zerocalc(a);
        signcalc(a);

        return 1;
    }


// Instruction: Load The X Register
// Function:    X = M
// Flags Out:   N, Z
    BYTE Core65c02::LDX() {
        fetch();
        x = fetched;

        zerocalc(x);
        signcalc(x);

        return 1;
    }


// Instruction: Load The Y Register
// Function:    Y = M
// Flags Out:   N, Z
    BYTE Core65c02::LDY() {
        fetch();
        y = fetched;

        zerocalc(y);
        signcalc(y);

        return 1;
    }

    BYTE Core65c02::LSR() {
        fetch();
        temp = (WORD)fetched >> 1;

        if (fetched & 1)
            setcarry();
        else
            clearcarry();

        zerocalc(temp);
        signcalc(temp);

        push(temp & 0xFF);
        return 0;
    }

    BYTE Core65c02::NOP() {
        switch (opcode) {
            case 0x1C:
            case 0x3C:
            case 0x5C:
            case 0x7C:
            case 0xDC:
            case 0xFC:
                return 1;
        }
        return 0;
    }


// Instruction: Bitwise Logic OR
// Function:    A = A | M
// Flags Out:   N, Z
    BYTE Core65c02::ORA() {
        fetch();
        a = a | fetched;

        zerocalc(a);
        signcalc(a);

        return 1;
    }


// Instruction: Push Accumulator to Stack
// Function:    A -> stack
    BYTE Core65c02::PHA() {
        push8(a);
        return 0;
    }


// Instruction: Push Status Register to Stack
// Function:    status -> stack
// Note:        Break flag is set to 1 before push
    BYTE Core65c02::PHP() {
        push8(status | FLAG_BREAK);
        return 0;
    }


// Instruction: Pop Accumulator off Stack
// Function:    A <- stack
// Flags Out:   N, Z
    BYTE Core65c02::PLA() {
        a = pull8();

        zerocalc(a);
        signcalc(a);

        return 0;
    }


// Instruction: Pop Status Register off Stack
// Function:    Status <- stack
    BYTE Core65c02::PLP() {
        status = pull8() | FLAG_CONSTANT;
        return 0;
    }

    BYTE Core65c02::ROL() {
        fetch();

        temp = ((WORD)fetched << 1) | (WORD)(status & FLAG_CARRY);

        carrycalc(temp);
        zerocalc(temp);
        signcalc(temp);

        push(temp & 0xFF);
        return 0;
    }

    BYTE Core65c02::ROR() {
        fetch();
        temp = ((WORD)fetched >> 1) | ((WORD)(status & FLAG_CARRY) << 7);

        if (fetched & 1)
            setcarry();
        else
            clearcarry();

        zerocalc(temp);
        signcalc(temp);

        push(temp & 0xFF);
        return 0;
    }

    BYTE Core65c02::RTI() {
        status = pull8();
        pc = pull16();
        return 0;
    }

    BYTE Core65c02::RTS() {
        pc = pull16() + 1;
        return 0;
    }


// Instruction: Set Carry Flag
// Function:    C = 1
    BYTE Core65c02::SEC() {
        setcarry();
        return 0;
    }


// Instruction: Set Decimal Flag
// Function:    D = 1
    BYTE Core65c02::SED() {
        setdecimal();
        return 0;
    }


// Instruction: Set Interrupt Flag / Enable Interrupts
// Function:    I = 1
    BYTE Core65c02::SEI() {
        setinterrupt();
        return 0;
    }


// Instruction: Store Accumulator at Address
// Function:    M = A
    BYTE Core65c02::STA() {
        m_mem->Push(DataFormat::Byte, addr_abs, a);
        return 0;
    }


// Instruction: Store X Register at Address
// Function:    M = X
    BYTE Core65c02::STX() {
        m_mem->Push(DataFormat::Byte, addr_abs, x);
        return 0;
    }


// Instruction: Store Y Register at Address
// Function:    M = Y
    BYTE Core65c02::STY() {
        m_mem->Push(DataFormat::Byte, addr_abs, y);
        return 0;
    }


// Instruction: Transfer Accumulator to X Register
// Function:    X = A
// Flags Out:   N, Z
    BYTE Core65c02::TAX() {
        x = a;

        zerocalc(x);
        signcalc(x);

        return 0;
    }


// Instruction: Transfer Accumulator to Y Register
// Function:    Y = A
// Flags Out:   N, Z
    BYTE Core65c02::TAY() {
        y = a;

        zerocalc(y);
        signcalc(y);

        return 0;
    }


// Instruction: Transfer Stack Pointer to X Register
// Function:    X = stack pointer
// Flags Out:   N, Z
    BYTE Core65c02::TSX() {
        x = stkp;

        zerocalc(x);
        signcalc(x);

        return 0;
    }


// Instruction: Transfer X Register to Accumulator
// Function:    A = X
// Flags Out:   N, Z
    BYTE Core65c02::TXA() {
        a = x;

        zerocalc(a);
        signcalc(a);

        return 0;
    }


// Instruction: Transfer X Register to Stack Pointer
// Function:    stack pointer = X
    BYTE Core65c02::TXS() {
        stkp = x;
        return 0;
    }


// Instruction: Transfer Y Register to Accumulator
// Function:    A = Y
// Flags Out:   N, Z
    BYTE Core65c02::TYA() {
        a = y;

        zerocalc(a);
        signcalc(a);

        return 0;
    }
}
