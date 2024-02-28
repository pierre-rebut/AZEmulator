//
// Created by pierr on 25/10/2023.
//

#include "Core8086.h"
#include <iostream>

namespace Astra::CPU::Lib::CPU8086 {

    void Core8086::opWordRegMem() {
        decode();
        int src = getRM(w, mod, rm);
        int res;
        int dst;

        switch (reg) {
            case 0b000:    // INC
                res = inc(w, src);
                setRM(w, mod, rm, res);
                clocks += mod == 0b11 ? 3 : 15;
                break;
            case 0b001:    // DEC
                res = dec(w, src);
                setRM(w, mod, rm, res);
                clocks += mod == 0b11 ? 3 : 15;
                break;
            case 0b010:    // CALL
                push(ip);
                ip = src;
                clocks += mod == 0b11 ? 16 : 21;
                break;
            case 0b011:    // CALL
                push(cs);
                push(ip);
                dst = getEA(mod, rm);
                ip = getMem(W, dst);
                cs = getMem(W, dst + 2);
                clocks += 37;
                break;
            case 0b100:    // JMP
                ip = src;
                clocks += mod == 0b11 ? 11 : 18;
                break;
            case 0b101:    // JMP
                dst = getEA(mod, rm);
                ip = getMem(W, dst);
                cs = getMem(W, dst + 2);
                clocks += 24;
                break;
            case 0b110:    // PUSH
                push(src);
                clocks += mod == 0b11 ? 11 : 16;
                break;
        }
    }

    void Core8086::opIncDecRegMemB() {
        decode();
        int src = getRM(w, mod, rm);
        int res;

        switch (reg) {
            case 0b000:    // INC
                res = inc(w, src);
                setRM(w, mod, rm, res);
                break;
            case 0b001:    // DEC
                res = dec(w, src);
                setRM(w, mod, rm, res);
                break;
        }
        clocks += mod == 0b11 ? 3 : 15;
    }

    void Core8086::opAluMultiRegMem() {
        decode();
        int src = getRM(w, mod, rm);

        int res;
        int dst;

        switch (reg) {
            case 0b000:    // TEST
                dst = getMem(w);
                logic(w, dst & src);
                clocks += mod == 0b11 ? 5 : 11;
                break;
            case 0b010:    // NOT
                setRM(w, mod, rm, ~src);
                clocks += mod == 0b11 ? 3 : 16;
                break;
            case 0b011:    // NEG
                dst = sub(w, 0, src);
                f_bits.CF = (dst > 0);
                setRM(w, mod, rm, dst);
                clocks += mod == 0b11 ? 3 : 16;
                break;
            case 0b100:    // MUL
                if (w == B) {
                    dst = ax.l;
                    res = dst * src & 0xffff;
                    ax.x = res;
                    if (ax.h > 0) {
                        f_bits.CF = true;
                        f_bits.OF = true;
                    } else {
                        f_bits.CF = false;
                        f_bits.OF = false;
                    }
                    clocks += mod == 0b11 ? (77 - 70) / 2 : (83 - 76) / 2;
                } else {
                    dst = ax.x;
                    const long lres = (long) dst * (long) src & 0xffffffff;
                    ax.x = (WORD) lres;
                    dx.x = (WORD) (lres >> 16);
                    if (dx.x > 0) {
                        f_bits.CF = true;
                        f_bits.OF = true;
                    } else {
                        f_bits.CF = false;
                        f_bits.OF = false;
                    }
                    clocks += mod == 0b11 ? (133 - 118) / 2 : (139 - 124) / 2;
                }
                break;
            case 0b101:    // IMUL
                if (w == B) {
                    src = signconv(B, src);
                    dst = ax.l;
                    dst = signconv(B, dst);
                    res = dst * src & 0xffff;
                    ax.x = res;
                    if (ax.h > 0x00 && ax.h < 0xff) {
                        f_bits.CF = true;
                        f_bits.OF = true;
                    } else {
                        f_bits.CF = false;
                        f_bits.OF = false;
                    }
                    clocks += mod == 0b11 ? (98 - 80) / 2 : (154 - 128) / 2;
                } else {
                    src = signconv(W, src);
                    dst = ax.h << 8 | ax.l;
                    dst = signconv(W, dst);
                    const long lres = (long) dst * (long) src & 0xffffffff;
                    ax.x = (WORD) lres;
                    dx.x = (WORD) (lres >> 16);
                    if (dx.x > 0x0000 && dx.x < 0xffff) {
                        f_bits.CF = true;
                        f_bits.OF = true;
                    } else {
                        f_bits.CF = false;
                        f_bits.OF = false;
                    }
                    clocks += mod == 0b11 ? (104 - 86) / 2 : (160 - 134) / 2;
                }
                break;
            case 0b110:    // DIV
                if (src == 0) {
                    callInt(0);
                } else if (w == B) {
                    dst = ax.x;
                    res = dst / src & 0xffff;
                    if (res > 0xff) {
                        callInt(0);
                    } else {
                        ax.l = res & 0xff;
                        ax.h = dst % src & 0xff;
                    }
                    clocks += mod == 0b11 ? (90 - 80) / 2 : (96 - 86) / 2;
                } else {
                    const long ldst = (long) dx.x << 16 | ax.x;
                    long long lres = ldst / src & 0xffffffff;
                    if (lres > 0xffff) {
                        callInt(0);
                    } else {
                        ax.x = (WORD) lres;
                        lres = ldst % src & 0xffff;
                        dx.x = (WORD) lres;
                    }
                    clocks += mod == 0b11 ? (162 - 144) / 2 : (168 - 150) / 2;
                }
                break;
            case 0b111:    // IDIV
                if (src == 0) {
                    callInt(0);
                } else if (w == B) {
                    src = signconv(B, src);
                    dst = ax.x;
                    dst = signconv(W, dst);
                    res = dst / src & 0xffff;
                    if (res > 0x007f && res < 0xff81) {
                        callInt(0);
                    } else {
                        ax.l = res & 0xff;
                        ax.h = dst % src & 0xff;
                    }
                    clocks += mod == 0b11 ? (112 - 101) / 2 : (118 - 107) / 2;
                } else {
                    src = signconv(W, src);
                    long long ldst = (long) dx.x << 16 | ax.x;
                    // Do sign conversion manually.
                    ldst = ldst << 32 >> 32;
                    long long lres = ldst / src & 0xffffffff;
                    if (lres > 0x00007fff || lres < 0xffff8000) {
                        callInt(0);
                    } else {
                        ax.x = (WORD) lres;
                        lres = ldst % src & 0xffff;
                        dx.x = (WORD) lres;
                    }
                    clocks += mod == 0b11 ? (184 - 165) / 2 : (190 - 171) / 2;
                }
                break;
        }
    }

    void Core8086::opAluComplexToRegMem() {
        decode();
        int dst = getRM(w, mod, rm);
        int src = op == 0xd0 || op == 0xd1 ? 1 : cx.l;
        bool tempCF;
        switch (reg) {
            case 0b000:    // ROL
                for (int cnt = 0; cnt < src; ++cnt) {
                    tempCF = msb(w, dst);
                    dst <<= 1;
                    dst |= tempCF ? 0b1 : 0b0;
                    dst &= MASK[w];
                }
                f_bits.CF = ((dst & 0b1) == 0b1);
                if (src == 1) {
                    f_bits.OF = (msb(w, dst) ^ f_bits.CF);
                }
                break;
            case 0b001:    // ROR
                for (int cnt = 0; cnt < src; ++cnt) {
                    tempCF = (dst & 0b1) == 0b1;
                    dst >>= 1;
                    dst |= (tempCF ? 1 : 0) * SIGN[w];
                    dst &= MASK[w];
                }
                f_bits.CF = (msb(w, dst));
                if (src == 1) {
                    f_bits.OF = (msb(w, dst) ^ msb(w, dst << 1));
                }
                break;
            case 0b010:    // RCL
                for (int cnt = 0; cnt < src; ++cnt) {
                    tempCF = msb(w, dst);
                    dst <<= 1;
                    dst |= f_bits.CF ? 0b1 : 0b0;
                    dst &= MASK[w];
                    f_bits.CF = (tempCF);
                }
                if (src == 1) {
                    f_bits.OF = (msb(w, dst) ^ f_bits.CF);
                }
                break;
            case 0b011:    // RCR
                if (src == 1) {
                    f_bits.OF = (msb(w, dst) ^ f_bits.CF);
                }
                for (int cnt = 0; cnt < src; ++cnt) {
                    tempCF = (dst & 0b1) == 0b1;
                    dst >>= 1;
                    dst |= (f_bits.CF ? 1 : 0) * SIGN[w];
                    dst &= MASK[w];
                    f_bits.CF = (tempCF);
                }
                break;
            case 0b100:    // SAL/SHL
                for (int cnt = 0; cnt < src; ++cnt) {
                    f_bits.CF = ((dst & SIGN[w]) == SIGN[w]);
                    dst <<= 1;
                    dst &= MASK[w];
                }
                // Determine overflow.
                if (src == 1) {
                    f_bits.OF = (((dst & SIGN[w]) == SIGN[w]) ^ f_bits.CF);
                }
                if (src > 0) {
                    setFlags(w, dst);
                }
                break;
            case 0b101:    // SHR
                // Determine overflow.
                if (src == 1) {
                    f_bits.OF = ((dst & SIGN[w]) == SIGN[w]);
                }
                for (int cnt = 0; cnt < src; ++cnt) {
                    f_bits.CF = ((dst & 0b1) == 0b1);
                    dst >>= 1;
                    dst &= MASK[w];
                }
                if (src > 0) {
                    setFlags(w, dst);
                }
                break;
            case 0b111:    // SAR
                // Determine overflow.
                if (src == 1) {
                    f_bits.OF = false;
                }
                for (int cnt = 0; cnt < src; ++cnt) {
                    f_bits.CF = ((dst & 0b1) == 0b1);
                    dst = signconv(w, dst);
                    dst >>= 1;
                    dst &= MASK[w];
                }
                if (src > 0) {
                    setFlags(w, dst);
                }
                break;
        }
        setRM(w, mod, rm, dst);
        if (op == 0xd0 || op == 0xd1) {
            clocks += mod == 0b11 ? 2 : 15;
        } else {
            clocks += mod == 0b11 ? 8 + 4 * src : 20 + 4 * src;
        }
    }

    void Core8086::opPopRegMem() {
        decode();
        if (reg == 0b000) {    // POP
            int src = pop();
            setRM(w, mod, rm, src);
        }
        clocks += mod == 0b11 ? 8 : 17;
    }

    void Core8086::opAluImmToRegMem() {
        decode();
        int res;
        int dst = getRM(w, mod, rm);
        int src = getMem(B);
        if (op == 0x81) {
            src |= getMem(B) << 8;
        }
            // Perform sign extension if needed.
        else if (op == 0x83 && (src & SIGN[B]) > 0) {
            src |= 0xff00;
        }
        switch (reg) {
            case 0b000:    // ADD
                res = add(w, dst, src);
                setRM(w, mod, rm, res);
                break;
            case 0b001:    // OR
                if (op == 0x80 || op == 0x81) {
                    res = dst | src;
                    logic(w, res);
                    setRM(w, mod, rm, res);
                    break;
                }
                break;
            case 0b010:    // ADC
                res = adc(w, dst, src);
                setRM(w, mod, rm, res);
                break;
            case 0b011:    // SBB
                res = sbb(w, dst, src);
                setRM(w, mod, rm, res);
                break;
            case 0b100:    // AND
                if (op == 0x80 || op == 0x81) {
                    res = dst & src;
                    logic(w, res);
                    setRM(w, mod, rm, res);
                }
                break;
            case 0b101:    // SUB
                res = sub(w, dst, src);
                setRM(w, mod, rm, res);
                break;
            case 0b110:    // XOR
                if (op == 0x80 || op == 0x81) {
                    res = dst ^ src;
                    logic(w, res);
                    setRM(w, mod, rm, res);
                }
                break;
            case 0b111:    // CMP
                sub(w, dst, src);
                if (mod == 0b11) {
                    clocks -= 3;
                }
                break;
        }
        clocks += mod == 0b11 ? 4 : 17;
    }

    void Core8086::opEsc() {
        decode();
        clocks += mod == 0b11 ? 2 : 8;
    }

    void Core8086::opLock() { clocks += 2; }

    void Core8086::opNoop() { clocks += 3; }

    void Core8086::opHlt() {
        clocks += 2;
        std::cout << "halt" << std::endl;
        //m_runService->Stop();
    }

    void Core8086::opBreakpoint() {
        clocks = 0;
        m_runService->Breakpoint();
    }

    void Core8086::opSti() {
        f_bits.IF = true;
        clocks += 2;
    }

    void Core8086::opCli() {
        f_bits.IF = false;
        clocks += 2;
    }

    void Core8086::opStd() {
        f_bits.DF = true;
        clocks += 2;
    }

    void Core8086::opCld() {
        f_bits.DF = false;
        clocks += 2;
    }

    void Core8086::opStc() {
        f_bits.CF = true;
        clocks += 2;
    }

    void Core8086::opCmc() {
        f_bits.CF = (!f_bits.CF);
        clocks += 2;
    }

    void Core8086::opClc() {
        f_bits.CF = false;
        clocks += 2;
    }

    void Core8086::opIret() {
        ip = pop();
        cs = pop();
        flags = pop();

        clocks += 24;
    }

    void Core8086::opInto() {
        if (f_bits.OF) {
            callInt(4);
            clocks += 53;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opIntImm() {
        callInt(op == 0xcc ? 3 : getMem(B));
        clocks += op == 0xcc ? 52 : 51;
    }

    void Core8086::opJcxzShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (cx.x == 0) {
            ip = ip + dst & 0xffff;
            clocks += 18;
        } else {
            clocks += 6;
        }
    }

    void Core8086::opLoopneLoopnzShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        WORD src = cx.x - 1 & 0xffff;
        cx.x = src;
        if (src != 0 && !f_bits.ZF) {
            ip = ip + dst & 0xffff;
            clocks += 19;
        } else {
            clocks += 5;
        }
    }

    void Core8086::opLoopeLoopzShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        WORD src = cx.x - 1 & 0xffff;
        cx.x = src;
        if (src != 0 && f_bits.ZF) {
            ip = ip + dst & 0xffff;
            clocks += 18;
        } else {
            clocks += 6;
        }
    }

    void Core8086::opLoopShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        WORD src = cx.x - 1 & 0xffff;
        cx.x = src;
        if (src != 0) {
            ip = ip + dst & 0xffff;
            clocks += 17;
        } else {
            clocks += 5;
        }
    }

    void Core8086::opJnleJgShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (!(f_bits.SF ^ f_bits.OF | f_bits.ZF)) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJleJngShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (f_bits.SF ^ f_bits.OF | f_bits.ZF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJnlJgeShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (!(f_bits.SF ^ f_bits.OF)) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJlJngeShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (f_bits.SF ^ f_bits.OF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJnpJpoShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (!f_bits.PF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJpJpeShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (f_bits.PF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJnsShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (!f_bits.SF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJnbeJaShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (!(f_bits.CF | f_bits.ZF)) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJbeJnaShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (f_bits.CF | f_bits.ZF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJneJnzShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (!f_bits.ZF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJnbJaeJncShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (!f_bits.CF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJbJnaeJcShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (f_bits.CF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJneShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (!f_bits.OF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJoShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (f_bits.OF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJmpFar() {
        int dst = getMem(W);
        int src = getMem(W);
        ip = dst;
        cs = src;
        clocks += 15;
    }

    void Core8086::opJmpShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        ip = ip + dst & 0xffff;
        clocks += 15;
    }

    void Core8086::opJmpNear() {
        int dst = getMem(W);
        dst = signconv(W, dst);
        ip = ip + dst & 0xffff;
        clocks += 15;
    }

    void Core8086::opRetIntersegImm() {
        int src = getMem(W);
        ip = pop();
        cs = pop();
        sp += src;
        clocks += 17;
    }

    void Core8086::opRetInterseg() {
        ip = pop();
        cs = pop();
        clocks += 18;
    }

    void Core8086::opRetIntrasegImm() {
        int src = getMem(W);
        ip = pop();
        sp += src;
        clocks += 12;
    }

    void Core8086::opRetIntraseg() {
        ip = pop();
        clocks += 8;
    }

    void Core8086::opCallFarProc() {
        int dst = getMem(W);
        int src = getMem(W);
        push(cs);
        push(ip);
        ip = dst;
        cs = src;
        clocks += 28;
    }

    void Core8086::opCallNearProc() {
        int dst = getMem(W);
        dst = signconv(W, dst);
        push(ip);
        ip = ip + dst & 0xffff;
        clocks += 19;
    }

    void Core8086::opStoString() {
        int src = getReg(w, AX);
        setMem(w, getAddr(es, di), src);
        di = di + (f_bits.DF ? -1 : 1) * (1 + w) & 0xffff;
        clocks += 10;
    }

    void Core8086::opLodString() {
        auto addr = getAddr(os, si);
        int src = getMem(w, addr);
        setReg(w, AX, src);
        si = si + (f_bits.DF ? -1 : 1) * (1 + w) & 0xffff;
        clocks += 13;
    }

    void Core8086::opScaString() {
        int dst = getMem(w, getAddr(es, di));
        int src = getReg(w, AX);
        sub(w, src, dst);
        di = di + (f_bits.DF ? -1 : 1) * (1 + w) & 0xffff;
        if (rep == 1 && !f_bits.ZF || rep == 2 && f_bits.ZF) {
            rep = 0;
        }
        clocks += 15;
    }

    void Core8086::opCmpString() {
        int dst = getMem(w, getAddr(es, di));
        int src = getMem(w, getAddr(os, si));
        sub(w, src, dst);
        si = si + (f_bits.DF ? -1 : 1) * (1 + w) & 0xffff;
        di = di + (f_bits.DF ? -1 : 1) * (1 + w) & 0xffff;
        if (rep == 1 && !f_bits.ZF || rep == 2 && f_bits.ZF) {
            rep = 0;
        }
        clocks += 22;
    }

    void Core8086::opMovString() {
        int src = getMem(w, getAddr(os, si));
        setMem(w, getAddr(es, di), src);
        si = si + (f_bits.DF ? -1 : 1) * (1 + w) & 0xffff;
        di = di + (f_bits.DF ? -1 : 1) * (1 + w) & 0xffff;
        clocks += 17;
    }

    void Core8086::opTestImmAndAcc() {
        int dst = getReg(w, AX);
        int src = getMem(w);
        logic(w, dst & src);
        clocks += 4;
    }

    void Core8086::opTestRegMem() {
        decode();
        int dst = getRM(w, mod, rm);
        int src = getReg(w, reg);
        logic(w, dst & src);
        clocks += mod == 0b11 ? 3 : 9;
    }

    void Core8086::opXorImmToAcc() {
        int dst = getReg(w, AX);
        int src = getMem(w);
        int res = dst ^ src;
        logic(w, res);
        setReg(w, AX, res);
        clocks += 4;
    }

    void Core8086::opXorRegMem() {
        decode();

        int dst;
        int src;

        if (!d) {
            dst = getRM(w, mod, rm);
            src = getReg(w, reg);
        } else {
            dst = getReg(w, reg);
            src = getRM(w, mod, rm);
        }
        int res = dst ^ src;
        logic(w, res);
        if (!d) {
            setRM(w, mod, rm, res);
            clocks += mod == 0b11 ? 3 : 16;
        } else {
            setReg(w, reg, res);
            clocks += mod == 0b11 ? 3 : 9;
        }
    }

    void Core8086::opOrImmToAcc() {
        int dst = getReg(w, AX);
        int src = getMem(w);
        int res = dst | src;
        logic(w, res);
        setReg(w, AX, res);
        clocks += 4;
    }

    void Core8086::opOrRegMem() {
        decode();
        int src;
        int dst;
        if (!d) {
            dst = getRM(w, mod, rm);
            src = getReg(w, reg);
        } else {
            dst = getReg(w, reg);
            src = getRM(w, mod, rm);
        }
        int res = dst | src;
        logic(w, res);
        if (!d) {
            setRM(w, mod, rm, res);
            clocks += mod == 0b11 ? 3 : 16;
        } else {
            setReg(w, reg, res);
            clocks += mod == 0b11 ? 3 : 9;
        }
    }

    void Core8086::opAndImmToAcc() {
        int dst = getReg(w, AX);
        int src = getMem(w);
        int res = dst & src;
        logic(w, res);
        setReg(w, AX, res);
        clocks += 4;
    }

    void Core8086::opAndRegMem() {
        decode();
        int src;
        int dst;
        if (!d) {
            dst = getRM(w, mod, rm);
            src = getReg(w, reg);
        } else {
            dst = getReg(w, reg);
            src = getRM(w, mod, rm);
        }
        int res = dst & src;
        logic(w, res);
        if (!d) {
            setRM(w, mod, rm, res);
            clocks += mod == 0b11 ? 3 : 16;
        } else {
            setReg(w, reg, res);
            clocks += mod == 0b11 ? 3 : 9;
        }
    }

    void Core8086::opCwd() {
        if ((ax.h & 0x80) == 0x80) {
            dx.x = 0xffff;
        } else {
            dx.x = 0x0000;
        }
        clocks += 5;
    }

    void Core8086::opCbw() {
        if ((ax.l & 0x80) == 0x80) {
            ax.h = 0xff;
        } else {
            ax.h = 0x00;
        }
        clocks += 2;
    }

    void Core8086::opAad() {
        int src = getMem(B);
        ax.l = ax.h * src + ax.l & 0xff;
        ax.h = 0;
        setFlags(B, ax.l);
        clocks += 60;
    }

    void Core8086::opAam() {
        int src = getMem(B);
        if (src == 0) {
            callInt(0);
        } else {
            ax.h = ax.l / src & 0xff;
            ax.l = ax.l % src & 0xff;
            setFlags(W, ax.x);
            clocks += 83;
        }
    }

    void Core8086::opDas() {
        int oldAL = ax.l;
        bool oldCF = f_bits.CF;
        f_bits.CF = false;
        if ((ax.l & 0xf) > 9 || f_bits.AF) {
            ax.l -= 6;
            f_bits.CF = (oldCF || (ax.l & 0xff) > 0);
            ax.l &= 0xff;
            f_bits.AF = true;
        } else {
            f_bits.AF = false;
        }
        if (oldAL > 0x99 || oldCF) {
            ax.l = ax.l - 0x60 & 0xff;
            f_bits.CF = true;
        } else {
            f_bits.CF = false;
        }
        setFlags(B, ax.l);
        clocks += 4;
    }

    void Core8086::opAas() {
        if ((ax.l & 0xf) > 9 || f_bits.AF) {
            ax.l -= 6;
            ax.h = ax.h - 1 & 0xff;
            f_bits.CF = true;
            f_bits.AF = true;
        } else {
            f_bits.CF = false;
            f_bits.AF = false;
        }
        ax.l &= 0xf;
        clocks += 4;
    }

    void Core8086::opCmpAccWithImm() {
        int dst = getReg(w, AX);
        int src = getMem(w);
        sub(w, dst, src);
        clocks += 4;
    }

    void Core8086::opCmpRegMem() {
        decode();
        int dst;
        int src;
        if (!d) {
            dst = getRM(w, mod, rm);
            src = getReg(w, reg);
        } else {
            dst = getReg(w, reg);
            src = getRM(w, mod, rm);
        }
        sub(w, dst, src);
        clocks += mod == 0b11 ? 3 : 9;
    }

    void Core8086::opDecReg() {
        reg = op & 0b111;
        int dst = getReg(W, reg);
        int res = dec(W, dst);
        setReg(W, reg, res);
        clocks += 2;
    }

    void Core8086::opSbbImmToAcc() {
        int dst = getReg(w, AX);
        int src = getMem(w);
        int res = sbb(w, dst, src);
        setReg(w, AX, res);
        clocks += 4;
    }

    void Core8086::opSbbRegMem() {
        decode();
        int src;
        int dst;
        if (!d) {
            dst = getRM(w, mod, rm);
            src = getReg(w, reg);
        } else {
            dst = getReg(w, reg);
            src = getRM(w, mod, rm);
        }
        int res = sbb(w, dst, src);
        if (!d) {
            setRM(w, mod, rm, res);
            clocks += mod == 0b11 ? 3 : 16;
        } else {
            setReg(w, reg, res);
            clocks += mod == 0b11 ? 3 : 9;
        }
    }

    void Core8086::opSubImmFromAcc() {
        int dst = getReg(w, AX);
        int src = getMem(w);
        int res = sub(w, dst, src);
        setReg(w, AX, res);
        clocks += 4;
    }

    void Core8086::opSubRegMem() {
        decode();
        int dst;
        int src;
        if (!d) {
            dst = getRM(w, mod, rm);
            src = getReg(w, reg);
        } else {
            dst = getReg(w, reg);
            src = getRM(w, mod, rm);
        }
        int res = sub(w, dst, src);
        if (!d) {
            setRM(w, mod, rm, res);
            clocks += mod == 0b11 ? 3 : 16;
        } else {
            setReg(w, reg, res);
            clocks += mod == 0b11 ? 3 : 9;
        }
    }

    void Core8086::opDaa() {
        int oldAL = ax.l;
        bool oldCF = f_bits.CF;
        f_bits.CF = false;
        if ((ax.l & 0xf) > 9 || f_bits.AF) {
            ax.l += 6;
            f_bits.CF = (oldCF || ax.l < 0);
            ax.l &= 0xff;
            f_bits.AF = true;
        } else {
            f_bits.AF = false;
        }
        if (oldAL > 0x99 || oldCF) {
            ax.l = ax.l + 0x60 & 0xff;
            f_bits.CF = true;
        } else {
            f_bits.CF = false;
        }
        setFlags(B, ax.l);
        clocks += 4;
    }

    void Core8086::opAaa() {
        if ((ax.l & 0xf) > 9 || f_bits.AF) {
            ax.l += 6;
            ax.h = ax.h + 1 & 0xff;
            f_bits.CF = true;
            f_bits.AF = true;
        } else {
            f_bits.CF = false;
            f_bits.AF = false;
        }
        ax.l &= 0xf;
        clocks += 4;
    }

    void Core8086::opIncReg() {
        reg = op & 0b111;
        int src = getReg(W, reg);
        int res = inc(W, src);
        setReg(W, reg, res);
        clocks += 2;
    }

    void Core8086::opAdcImmWithAcc() {
        int dst = getReg(w, AX);
        int src = getMem(w);
        int res = adc(w, dst, src);
        setReg(w, AX, res);
        clocks += 4;
    }

    void Core8086::opAdcRegMem() {
        decode();
        int src;
        int dst;
        if (!d) {
            dst = getRM(w, mod, rm);
            src = getReg(w, reg);
        } else {
            dst = getReg(w, reg);
            src = getRM(w, mod, rm);
        }
        int res = adc(w, dst, src);
        if (!d) {
            setRM(w, mod, rm, res);
            clocks += mod == 0b11 ? 3 : 16;
        } else {
            setReg(w, reg, res);
            clocks += mod == 0b11 ? 3 : 9;
        }
    }

    void Core8086::opAddImmToAcc() {
        int dst = getReg(w, 0);
        int src = getMem(w);
        int res = add(w, dst, src);
        setReg(w, AX, res);
        clocks += 4;
    }

    void Core8086::opAddRegMem() {
        decode();
        int dst;
        int src;
        if (!d) {
            dst = getRM(w, mod, rm);
            src = getReg(w, reg);
        } else {
            dst = getReg(w, reg);
            src = getRM(w, mod, rm);
        }
        int res = add(w, dst, src);
        if (!d) {
            setRM(w, mod, rm, res);
            clocks += mod == 0b11 ? 3 : 16;
        } else {
            setReg(w, reg, res);
            clocks += mod == 0b11 ? 3 : 9;
        }
    }

    void Core8086::opPopf() {
        flags = pop();
        clocks += 8;
    }

    void Core8086::opPushf() {
        push(flags);
        clocks += 10;
    }

    void Core8086::opSahf() {
        flags = flags & 0xff00 | ax.h;
        clocks += 4;
    }

    void Core8086::opLahf() {
        ax.h = flags & 0xff;
        clocks += 4;
    }

    void Core8086::opLesRegMem() {
        decode();
        int src = getEA(mod, rm);
        setReg(w, reg, getMem(W, src));
        es = getMem(W, src + 2);
        clocks += 16;
    }

    void Core8086::opLdsRegMem() {
        decode();
        int src = getEA(mod, rm);
        setReg(w, reg, getMem(W, src));
        ds = getMem(W, src + 2);
        clocks += 16;
    }

    void Core8086::opLeaRegMem() {
        decode();
        int src = getEA(mod, rm) - (os << 4);
        setReg(w, reg, src);
        clocks += 2;
    }

    void Core8086::opOutAccDx() {
        int src = dx.x;
        portOut(w, src, ax.x);
        clocks += 8;
        if (w == W && (src & 0b1) == 0b1) {
            clocks += 4;
        }
    }

    void Core8086::opOutAccImm() {
        int src = getMem(B);
        portOut(w, src, ax.x);
        clocks += 10;
        if (w == W && (src & 0b1) == 0b1) {
            clocks += 4;
        }
    }

    void Core8086::opInAccDx() {
        int src = dx.x;
        setReg(w, AX, portIn(w, src));
        clocks += 8;
        if (w == W && (src & 0b1) == 0b1) {
            clocks += 4;
        }
    }

    void Core8086::opInAccImm() {
        int src = getMem(B);
        setReg(w, AX, portIn(w, src));
        clocks += 10;
        if (w == W && (src & 0b1) == 0b1) {
            clocks += 4;
        }
    }

    void Core8086::opXlat() {
        ax.l = getMem(B, getAddr(os, bx.x + ax.l));
        clocks += 11;
    }

    void Core8086::opXchgRegWithAcc() {
        reg = op & 0b111;
        int dst = ax.x;
        ax.x = getReg(W, reg);
        setReg(W, reg, dst);
        clocks += 3;
    }

    void Core8086::opXchgRegMem() {
        decode();
        int dst = getReg(w, reg);
        int src = getRM(w, mod, rm);
        setReg(w, reg, src);
        setRM(w, mod, rm, dst);
        clocks += mod == 0b11 ? 3 : 17;
    }

    void Core8086::opPopSegReg() {
        reg = op >> 3 & 0b111;
        int src = pop();
        setSegReg(reg, src);
        clocks += 8;
    }

    void Core8086::opPopReg() {
        reg = op & 0b111;
        int src = pop();
        setReg(W, reg, src);
        clocks += 8;
    }

    void Core8086::opPushSegReg() {
        reg = op >> 3 & 0b111;
        int src = getSegReg(reg);
        push(src);
        clocks += 10;
    }

    void Core8086::opPushReg() {
        reg = op & 0b111;
        int src = getReg(W, reg);
        push(src);
        clocks += 11;
    }

    void Core8086::opMovRegMemSegment() {
        decode();
        if (!d) {
            int src = getSegReg(reg);
            setRM(W, mod, rm, src);
            clocks += mod == 0b11 ? 2 : 9;
        } else {
            int src = getRM(W, mod, rm);
            setSegReg(reg, src);
            clocks += mod == 0b11 ? 2 : 8;
        }
    }

    void Core8086::opMovMemToAcc() {
        int dst = getMem(W);
        if (!d) {
            int src = getMem(w, getAddr(os, dst));
            setReg(w, AX, src);
        } else {
            int src = getReg(w, AX);
            setMem(w, getAddr(os, dst), src);
        }
        clocks += 10;
    }

    void Core8086::opMovImmToReg() {
        w = op >> 3 & 0b1;
        reg = op & 0b111;
        int src = getMem(w);
        setReg(w, reg, src);
        clocks += 4;
    }

    void Core8086::opMovImmToRegMem() {
        decode();
        if (reg == 0b000) {
            int src = getMem(w);
            setRM(w, mod, rm, src);
        }
        clocks += mod == 0b11 ? 4 : 10;
    }

    void Core8086::opMovRegMem() {
        decode();
        if (!d) {
            int src = getReg(w, reg);
            setRM(w, mod, rm, src);
            clocks += mod == 0b11 ? 2 : 9;
        } else {
            int src = getRM(w, mod, rm);
            setReg(w, reg, src);
            clocks += mod == 0b11 ? 2 : 8;
        }
    }

    void Core8086::opJsShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (f_bits.SF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opJeJzShort() {
        int dst = getMem(B);
        dst = signconv(B, dst);
        if (f_bits.ZF) {
            ip = ip + dst & 0xffff;
            clocks += 16;
        } else {
            clocks += 4;
        }
    }

    void Core8086::opUnknown() {
        // do nothing
    }
}
