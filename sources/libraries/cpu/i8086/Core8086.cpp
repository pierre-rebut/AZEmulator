//
// Created by pierr on 22/10/2023.
//

#include "Core8086.h"
#include "Opcodes.h"

#include <iostream>

namespace Astra::CPU::Lib::CPU8086 {

    const std::array<void (Core8086::*)(), 256> Core8086::OPCODE_TABLE = {
            &Core8086::op_00, &Core8086::op_01, &Core8086::op_02, &Core8086::op_03, &Core8086::op_04, &Core8086::op_05, &Core8086::op_06, &Core8086::op_07,
            &Core8086::op_08, &Core8086::op_09, &Core8086::op_0a, &Core8086::op_0b, &Core8086::op_0c, &Core8086::op_0d, &Core8086::op_0e, &Core8086::op_0f,
            &Core8086::op_10, &Core8086::op_11, &Core8086::op_12, &Core8086::op_13, &Core8086::op_14, &Core8086::op_15, &Core8086::op_16, &Core8086::op_17,
            &Core8086::op_18, &Core8086::op_19, &Core8086::op_1a, &Core8086::op_1b, &Core8086::op_1c, &Core8086::op_1d, &Core8086::op_1e, &Core8086::op_1f,
            &Core8086::op_20, &Core8086::op_21, &Core8086::op_22, &Core8086::op_23, &Core8086::op_24, &Core8086::op_25, &Core8086::op_26, &Core8086::op_27,
            &Core8086::op_28, &Core8086::op_29, &Core8086::op_2a, &Core8086::op_2b, &Core8086::op_2c, &Core8086::op_2d, &Core8086::op_2e, &Core8086::op_2f,
            &Core8086::op_30, &Core8086::op_31, &Core8086::op_32, &Core8086::op_33, &Core8086::op_34, &Core8086::op_35, &Core8086::op_36, &Core8086::op_37,
            &Core8086::op_38, &Core8086::op_39, &Core8086::op_3a, &Core8086::op_3b, &Core8086::op_3c, &Core8086::op_3d, &Core8086::op_3e, &Core8086::op_3f,
            &Core8086::op_40, &Core8086::op_41, &Core8086::op_42, &Core8086::op_43, &Core8086::op_44, &Core8086::op_45, &Core8086::op_46, &Core8086::op_47,
            &Core8086::op_48, &Core8086::op_49, &Core8086::op_4a, &Core8086::op_4b, &Core8086::op_4c, &Core8086::op_4d, &Core8086::op_4e, &Core8086::op_4f,
            &Core8086::op_50, &Core8086::op_51, &Core8086::op_52, &Core8086::op_53, &Core8086::op_54, &Core8086::op_55, &Core8086::op_56, &Core8086::op_57,
            &Core8086::op_58, &Core8086::op_59, &Core8086::op_5a, &Core8086::op_5b, &Core8086::op_5c, &Core8086::op_5d, &Core8086::op_5e, &Core8086::op_5f,
            &Core8086::op_60, &Core8086::op_61, &Core8086::op_62, &Core8086::op_63, &Core8086::op_64, &Core8086::op_65, &Core8086::op_66, &Core8086::op_67,
            &Core8086::op_68, &Core8086::op_69, &Core8086::op_6a, &Core8086::op_6b, &Core8086::op_6c, &Core8086::op_6d, &Core8086::op_6e, &Core8086::op_6f,
            &Core8086::op_70, &Core8086::op_71, &Core8086::op_72, &Core8086::op_73, &Core8086::op_74, &Core8086::op_75, &Core8086::op_76, &Core8086::op_77,
            &Core8086::op_78, &Core8086::op_79, &Core8086::op_7a, &Core8086::op_7b, &Core8086::op_7c, &Core8086::op_7d, &Core8086::op_7e, &Core8086::op_7f,
            &Core8086::op_80, &Core8086::op_81, &Core8086::op_82, &Core8086::op_83, &Core8086::op_84, &Core8086::op_85, &Core8086::op_86, &Core8086::op_87,
            &Core8086::op_88, &Core8086::op_89, &Core8086::op_8a, &Core8086::op_8b, &Core8086::op_8c, &Core8086::op_8d, &Core8086::op_8e, &Core8086::op_8f,
            &Core8086::op_90, &Core8086::op_91, &Core8086::op_92, &Core8086::op_93, &Core8086::op_94, &Core8086::op_95, &Core8086::op_96, &Core8086::op_97,
            &Core8086::op_98, &Core8086::op_99, &Core8086::op_9a, &Core8086::op_9b, &Core8086::op_9c, &Core8086::op_9d, &Core8086::op_9e, &Core8086::op_9f,
            &Core8086::op_a0, &Core8086::op_a1, &Core8086::op_a2, &Core8086::op_a3, &Core8086::op_a4, &Core8086::op_a5, &Core8086::op_a6, &Core8086::op_a7,
            &Core8086::op_a8, &Core8086::op_a9, &Core8086::op_aa, &Core8086::op_ab, &Core8086::op_ac, &Core8086::op_ad, &Core8086::op_ae, &Core8086::op_af,
            &Core8086::op_b0, &Core8086::op_b1, &Core8086::op_b2, &Core8086::op_b3, &Core8086::op_b4, &Core8086::op_b5, &Core8086::op_b6, &Core8086::op_b7,
            &Core8086::op_b8, &Core8086::op_b9, &Core8086::op_ba, &Core8086::op_bb, &Core8086::op_bc, &Core8086::op_bd, &Core8086::op_be, &Core8086::op_bf,
            &Core8086::op_c0, &Core8086::op_c1, &Core8086::op_c2, &Core8086::op_c3, &Core8086::op_c4, &Core8086::op_c5, &Core8086::op_c6, &Core8086::op_c7,
            &Core8086::op_c8, &Core8086::op_c9, &Core8086::op_ca, &Core8086::op_cb, &Core8086::op_cc, &Core8086::op_cd, &Core8086::op_ce, &Core8086::op_cf,
            &Core8086::op_d0, &Core8086::op_d1, &Core8086::op_d2, &Core8086::op_d3, &Core8086::op_d4, &Core8086::op_d5, &Core8086::op_d6, &Core8086::op_d7,
            &Core8086::op_d8, &Core8086::op_d9, &Core8086::op_da, &Core8086::op_db, &Core8086::op_dc, &Core8086::op_dd, &Core8086::op_de, &Core8086::op_df,
            &Core8086::op_e0, &Core8086::op_e1, &Core8086::op_e2, &Core8086::op_e3, &Core8086::op_e4, &Core8086::op_e5, &Core8086::op_e6, &Core8086::op_e7,
            &Core8086::op_e8, &Core8086::op_e9, &Core8086::op_ea, &Core8086::op_eb, &Core8086::op_ec, &Core8086::op_ed, &Core8086::op_ee, &Core8086::op_ef,
            &Core8086::op_f0, &Core8086::op_f1, &Core8086::op_f2, &Core8086::op_f3, &Core8086::op_f4, &Core8086::op_f5, &Core8086::op_f6, &Core8086::op_f7,
            &Core8086::op_f8, &Core8086::op_f9, &Core8086::op_fa, &Core8086::op_fb, &Core8086::op_fc, &Core8086::op_fd, &Core8086::op_fe, &Core8086::op_ff
    };

    const std::array<int, 2> Core8086::BITS = {8, 16};
    const std::array<int, 2> Core8086::SIGN = {0x80, 0x8000};
    const std::array<int, 2> Core8086::MASK = {0xff, 0xffff};

    static const std::array<int, 256> PARITY = {
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1,
            0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0,
            0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
            1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1,
            0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1,
            1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
    };

    void Core8086::Reset() {
        flags = 0;
        ip = 0x0000;
        cs = 0xffff;
        ds = 0x0000;
        ss = 0x0000;
        es = 0x0000;

        ax.x = bx.x = cx.x = dx.x = 0;
        si = di = bp = sp = os = 0;

        op = mod = reg = rep = rm = w = ea = d = 0;

        for (int i = 0; i < 6; i++) {
            queue[i] = 0;
        }

        clocks = 0;
    }

    void Core8086::Execute() {
        if (clocks == 0) {
            if (rep == 0) {
                preOpcodeCycle();
            }

            cycle_opcode();
        } else {
            clocks--;
        }
    }

    void Core8086::Interrupt(bool isNmi, int interruptId) {
        if (isNmi || f_bits.IF) {
            callInt(interruptId);
        }
    }

    void Core8086::preOpcodeCycle() {
        if (f_bits.TF) {
            callInt(1);
            clocks += 50;
        }

        os = ds;
        bool loop = true;
        while (loop) {
            // Segment prefix check.
            switch (getMem(B)) {
                case 0x26:    // ES
                    os = es;
                    clocks += 2;
                    break;
                case 0x2e:    // CS
                    os = cs;
                    clocks += 2;
                    break;
                case 0x36:    // SS
                    os = ss;
                    clocks += 2;
                    break;
                case 0x3e:    // DS
                    os = ds;
                    clocks += 2;
                    break;
                case 0xf2:    // repne/repnz
                    rep = 2;
                    clocks += 9;
                    break;
                case 0xf3:    // rep/repe/repz
                    rep = 1;
                    clocks += 9;
                    break;
                default:
                    ip = ip - 1 & 0xffff;
                    loop = false;
            }
        }

        for (int i = 0; i < 3; ++i) {
            int addr = i * 2;
            auto val = getMem(W, getAddr(cs, ip + addr));

            queue[addr] = val & 0xFF;
            queue[addr + 1] = val >> 8;
        }

        op = queue[0];
        d = op >> 1 & 0b1;
        w = op & 0b1;
        ip = ip + 1 & 0xffff;

        switch (op) {
            case 0xa4:    // movs
            case 0xa5:
            case 0xaa:    // stos
            case 0xab:
                if (rep == 0)
                    ++clocks;
                break;
            case 0xa6:    // cmps
            case 0xa7:
            case 0xae:    // scas
            case 0xaf:
                break;
            case 0xac:    // lods
            case 0xad:
                if (rep == 0)
                    --clocks;
                break;
            default:
                rep = 0;
                break;
        }
    }

    void Core8086::cycle_opcode() {
        if (rep > 0) {
            if (cx.x == 0) {
                rep = 0;
                return;
            }
            cx.x -= 1;
        }

        ea = -1;

        (this->*OPCODE_TABLE[op])();
    }

    int Core8086::adc(int type, int dst, int src) {
        int carry = f_bits.CF ? 1 : 0;
        int res = dst + src + carry & MASK[type];
        f_bits.CF = (carry == 1 ? res <= dst : res < dst);
        f_bits.AF = (((res ^ dst ^ src) & AF) > 0);
        f_bits.OF = ((shift((dst ^ src ^ -1) & (dst ^ res), 12 - BITS[type]) & OF) > 0);
        setFlags(type, res);
        return res;
    }

    int Core8086::add(int type, int dst, int src) {
        int res = dst + src & MASK[type];
        f_bits.CF = (res < dst);
        f_bits.AF = (((res ^ dst ^ src) & AF) > 0);
        f_bits.OF = ((shift((dst ^ src ^ -1) & (dst ^ res), 12 - BITS[type]) & OF) > 0);
        setFlags(type, res);
        return res;
    }

    int Core8086::sbb(int type, int dst, int src) {
        int carry = f_bits.CF ? 1 : 0;
        int res = dst - src - carry & MASK[type];
        f_bits.CF = (carry > 0 ? dst <= src : dst < src);
        f_bits.AF = (((res ^ dst ^ src) & AF) > 0);
        f_bits.OF = ((shift((dst ^ src) & (dst ^ res), 12 - BITS[type]) & OF) > 0);
        setFlags(type, res);
        return res;
    }

    int Core8086::sub(int type, int dst, int src) {
        int res = dst - src & MASK[type];
        f_bits.CF = (dst < src);
        f_bits.AF = (((res ^ dst ^ src) & AF) > 0);
        f_bits.OF = ((shift((dst ^ src) & (dst ^ res), 12 - BITS[type]) & OF) > 0);
        setFlags(type, res);
        return res;
    }

    void Core8086::callInt(int type) {
        std::cout << "int " << type << std::endl;

        push(flags);
        push(cs);
        push(ip);

        f_bits.IF = false;
        f_bits.TF = false;

        ip = (WORD) getMem(W, type * 4);
        cs = (WORD) getMem(W, type * 4 + 2);

        clocks = 61;
    }

    int Core8086::dec(int type, int dst) {
        int res = dst - 1 & MASK[type];
        f_bits.AF = (((res ^ dst ^ 1) & AF) > 0);
        f_bits.OF = (res == SIGN[type] - 1);
        setFlags(type, res);
        return res;
    }

    void Core8086::decode() {
        mod = queue[1] >> 6 & 0b11;
        reg = queue[1] >> 3 & 0b111;
        rm = queue[1] & 0b111;
        if (mod == 0b01) {
            ip = ip + 2 & 0xffff;
        } else if (mod == 0b00 && rm == 0b110 || mod == 0b10) {
            ip = ip + 3 & 0xffff;
        } else {
            ip = ip + 1 & 0xffff;
        }
    }

    int Core8086::getEA(int pMod, int pRm) {
        int disp = 0;
        if (pMod == 0b01) {
            // 8-bit displacement follows
            clocks += 4;
            disp = queue[2];
        } else if (pMod == 0b10) {
            // 16-bit displacement follows
            clocks += 4;
            disp = queue[3] << 8 | queue[2];
        }
        int pEa = 0;
        switch (pRm) {
            case 0b000:    // EA = (BX) + (SI) + DISP
                clocks += 7;
                pEa = bx.x + si + disp;
                break;
            case 0b001:    // EA = (BX) + (DI) + DISP
                clocks += 8;
                pEa = bx.x + di + disp;
                break;
            case 0b010:    // EA = (BP) + (SI) + DISP
                clocks += 8;
                pEa = bp + si + disp;
                break;
            case 0b011:    // EA = (BP) + (DI) + DISP
                clocks += 7;
                pEa = bp + di + disp;
                break;
            case 0b100:    // EA = (SI) + DISP
                clocks += 5;
                pEa = si + disp;
                break;
            case 0b101:    // EA = (DI) + DISP
                clocks += 5;
                pEa = di + disp;
                break;
            case 0b110:
                if (pMod == 0b00) {
                    // Direct address
                    clocks += 6;
                    pEa = queue[3] << 8 | queue[2];
                } else {
                    clocks += 5;
                    pEa = bp + disp;
                }
                break;
            case 0b111:    // EA = (BX) + DISP
                clocks += 5;
                pEa = bx.x + disp;
                break;
            default:
                break;
        }
        return (os << 4) + (pEa & 0xffff);
    }

    int Core8086::getMem(int type) {
        int addr = getAddr(cs, ip);
        int val;
        if (type == W) {
            val = (int) m_mem->Fetch(DataFormat::Word, addr);
        } else {
            val = (int) m_mem->Fetch(DataFormat::Byte, addr);
        }
        ip = ip + 1 + type & 0xffff;
        return val;
    }

    int Core8086::getMem(int type, int addr) {
        if (type == W) {
            if ((addr & 0b1) == 0b1) {
                clocks += 4;
            }
            return (int) m_mem->Fetch(DataFormat::Word, addr);
        } else {
            return (int) m_mem->Fetch(DataFormat::Byte, addr);
        }
    }

    int Core8086::getReg(int type, int pReg) const {
        if (type == B) {
            switch (pReg) {
                case 0b000:
                    return ax.l;
                case 0b001:
                    return cx.l;
                case 0b010:
                    return dx.l;
                case 0b011:
                    return bx.l;
                case 0b100:
                    return ax.h;
                case 0b101:
                    return cx.h;
                case 0b110:
                    return dx.h;
                case 0b111:
                    return bx.h;
                default:
                    return 0;
            }
        } else {
            switch (pReg) {
                case 0b000:
                    return ax.x;
                case 0b001:
                    return cx.x;
                case 0b010:
                    return dx.x;
                case 0b011:
                    return bx.x;
                case 0b100:
                    return sp;
                case 0b101:
                    return bp;
                case 0b110:
                    return si;
                case 0b111:
                    return di;
                default:
                    return 0;
            }
        }
    }

    int Core8086::getRM(int type, int pMod, int pRm) {
        if (pMod == 0b11) {
            return getReg(type, pRm);
        } else {
            return getMem(type, ea > 0 ? ea : getEA(pMod, pRm));
        }
    }

    int Core8086::getSegReg(int pReg) const {
        switch (pReg) {
            case 0b00:
                return es;
            case 0b01:
                return cs;
            case 0b10:
                return ss;
            case 0b11:
                return ds;
            default:
                return 0;
        }
    }

    void Core8086::setMem(int type, int addr, int val) {
        if (type == W) {
            if ((addr & 0b1) == 0b1) {
                clocks += 4;
            }
            m_mem->Push(DataFormat::Word, addr, val & 0xFFFF);
        } else {
            m_mem->Push(DataFormat::Byte, addr, val & 0xff);
        }
    }

    void Core8086::setReg(int type, int pReg, WORD val) {
        if (type == B) {
            switch (pReg) {
                case 0b000:
                    ax.l = val & 0xff;
                    break;
                case 0b001:
                    cx.l = val & 0xff;
                    break;
                case 0b010:
                    dx.l = val & 0xff;
                    break;
                case 0b011:
                    bx.l = val & 0xff;
                    break;
                case 0b100:
                    ax.h = val & 0xff;
                    break;
                case 0b101:
                    cx.h = val & 0xff;
                    break;
                case 0b110:
                    dx.h = val & 0xff;
                    break;
                case 0b111:
                    bx.h = val & 0xff;
                    break;
                default:
                    break;
            }
        } else {
            switch (pReg) {
                case 0b000:
                    ax.x = val & 0xffff;
                    break;
                case 0b001:
                    cx.x = val & 0xffff;
                    break;
                case 0b010:
                    dx.x = val & 0xffff;
                    break;
                case 0b011:
                    bx.x = val & 0xffff;
                    break;
                case 0b100:
                    sp = val & 0xffff;
                    break;
                case 0b101:
                    bp = val & 0xffff;
                    break;
                case 0b110:
                    si = val & 0xffff;
                    break;
                case 0b111:
                    di = val & 0xffff;
                    break;
                default:
                    break;
            }
        }
    }

    void Core8086::setRM(int type, int pMod, int pRm, int val) {
        if (pMod == 0b11) {
            setReg(type, pRm, val);
        } else {
            setMem(type, ea > 0 ? ea : getEA(pMod, pRm), val);
        }
    }

    void Core8086::setSegReg(int pReg, int val) {
        switch (pReg) {
            case 0b00:
                es = val & 0xffff;
                break;
            case 0b01:
                cs = val & 0xffff;
                break;
            case 0b10:
                ss = val & 0xffff;
                break;
            case 0b11:
                ds = val & 0xffff;
                break;
            default:
                break;
        }
    }

    void Core8086::setFlags(int type, int res) {
        f_bits.PF = (PARITY[res & 0xff] > 0);
        f_bits.ZF = (res == 0);
        f_bits.SF = ((shift(res, 8 - BITS[type]) & SF) > 0);
    }

    int Core8086::inc(int type, int dst) {
        int res = dst + 1 & MASK[type];
        f_bits.AF = (((res ^ dst ^ 1) & AF) > 0);
        f_bits.OF = (res == SIGN[type]);
        setFlags(type, res);
        return res;
    }

    void Core8086::logic(int type, int res) {
        f_bits.CF = false;
        f_bits.OF = false;
        setFlags(type, res);
    }

    int Core8086::pop() {
        int val = getMem(W, getAddr(ss, sp));
        sp = sp + 2 & 0xffff;
        return val;
    }

    void Core8086::push(int val) {
        sp = sp - 2 & 0xffff;
        setMem(W, getAddr(ss, sp), val);
    }

    int Core8086::portIn(int type, int port) {
        if (type == W) {
            return m_dev->Fetch(DataFormat::Word, port);
        } else {
            return m_dev->Fetch(DataFormat::Byte, port);
        }
    }

    void Core8086::portOut(int type, int port, int val) {
        if (type == W) {
            m_dev->Push(DataFormat::Word, port, val & 0xFFFF);
        } else {
            m_dev->Push(DataFormat::Byte, port, val & 0xFF);
        }
    }

    std::vector<int> Core8086::DebugExecute() const {
        return {
            ip, op, ax.x, bx.x, cx.x, dx.x, sp, bp, si, di, cs, ds, ss, es, os, flags
        };
    }
}