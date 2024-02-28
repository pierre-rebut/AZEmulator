//
// Created by pierr on 22/10/2023.
//
#pragma once

#include <array>

#include "EngineLib/data/Types.h"
#include "EngineLib/core/ICpuCore.h"
#include "EngineLib/data/IDevice.h"
#include "EngineLib/data/Base.h"

namespace Astra::CPU::Lib::CPU8086 {

    class Core8086 : public ICpuCore
    {
    private:
        friend class Factory;

        enum Flags8086
        {
            CF = 1,
            PF = 1 << 2,
            AF = 1 << 4,
            ZF = 1 << 6,
            SF = 1 << 7,
            TF = 1 << 8,
            IF = 1 << 9,
            DF = 1 << 10,
            OF = 1 << 11
        };
        enum Type8086
        {
            B = 0b0,
            W = 0b1
        };

        enum Register
        {
            AX = 0b000,
            CX = 0b001,
            DX = 0b010,
            BX = 0b011
        };

        union{
            struct{
                bool CF : 1;
                bool reserved_1 : 1;	/* always 1 */
                bool PF : 1;
                bool reserved_2 : 1; /* always 0 */
                bool AF : 1;
                bool reserved_3 : 1;	/* always 0 */
                bool ZF : 1;
                bool SF : 1;
                bool TF : 1;
                bool IF : 1;
                bool DF : 1;
                bool OF : 1;
                bool IOPL : 2;
                bool NT : 1;
                bool reserved_4 : 1;	/* 1 on 8086, 0 above */
            }f_bits;
            WORD flags = 0;
        };

        static const std::array<void (Core8086::*)(), 256> OPCODE_TABLE;

        static const std::array<int, 2> BITS;
        static const std::array<int, 2> SIGN;
        static const std::array<int, 2> MASK;

        union InternalRegister {
            struct {
                BYTE l;
                BYTE h;
            };
            WORD x = 0;
        };

        InternalRegister ax;
        InternalRegister bx;
        InternalRegister cx;
        InternalRegister dx;

        WORD sp = 0;
        WORD bp = 0;
        WORD si = 0;
        WORD di = 0;
        WORD cs = 0;
        WORD ds = 0;
        WORD ss = 0;
        WORD es = 0;
        WORD os = 0;
        WORD ip = 0;
        BYTE rep = 0;

        std::array<BYTE, 6> queue{};

        BYTE op = 0;
        bool d = false;
        bool w = false;
        BYTE mod = 0;
        BYTE reg = 0;
        BYTE rm = 0;
        int ea = 0;

        long long clocks = 0;

        Ref<IDevice> m_mem = nullptr;
        Ref<IDevice> m_dev = nullptr;

    public:
        bool IsInit() const override { return m_mem && m_dev; };
        inline bool IsComplete() const override {return clocks == 0 && rep == 0;}

        void Reset() override;
        void Execute() override;
        void Interrupt(bool isNmi, int interruptId) override;

        std::vector<int> DebugExecute() const override;

    private:
        void cycle_opcode();

        static int getAddr(int seg, int off) {
            return (seg << 4) + off;
        }

        static bool msb(int w, int x) {
            return (x & SIGN[w]) == SIGN[w];
        }

        static int shift(int x, int n) {
            return n >= 0 ? x << n : x >> -n;
        }
        static int signconv(int w, int x) {
            return x << (32 - BITS[w]) >> (32 - BITS[w]);
        }

        int adc(int w, int dst, int src);
        int add(int w, int dst, int src);
        int sbb(int w, int dst, int src);
        int sub(int w, int dst, int src);

        void callInt(int type);
        int dec(int w, int dst);
        void decode();

        int getEA(int mod, int rm);

        int getMem(int w);
        int getMem(int w, int addr);
        int getReg(int w, int reg) const;
        int getRM(int w, int mod, int rm);
        int getSegReg(int reg) const;

        void setFlags(int w, int res);
        void setMem(int w, int addr, int val);
        void setReg(int type, int pReg, WORD val);
        void setRM(int w, int mod, int rm, int val);
        void setSegReg(int reg, int val);

        int inc(int w, int dst);
        void logic(int w, int res);

        int pop();
        void push(int val);
        int portIn(int w, int port);
        void portOut(int w, int port, int val);

        void preOpcodeCycle();

        void opUnknown();
        void opMovRegMem();
        void opMovImmToRegMem();
        void opMovImmToReg();
        void opMovMemToAcc();
        void opMovRegMemSegment();
        void opPushReg();
        void opPushSegReg();
        void opPopReg();
        void opPopSegReg();
        void opXchgRegMem();
        void opXchgRegWithAcc();
        void opXlat();
        void opInAccImm();
        void opInAccDx();
        void opOutAccImm();
        void opOutAccDx();
        void opLeaRegMem();
        void opLdsRegMem();
        void opLesRegMem();
        void opLahf();
        void opSahf();
        void opPushf();
        void opPopf();
        void opAddRegMem();
        void opAddImmToAcc();
        void opAdcRegMem();
        void opAdcImmWithAcc();
        void opIncReg();
        void opAaa();
        void opDaa();
        void opSubRegMem();
        void opSubImmFromAcc();
        void opSbbRegMem();
        void opSbbImmToAcc();
        void opDecReg();
        void opCmpRegMem();
        void opCmpAccWithImm();
        void opAas();
        void opDas();
        void opAam();
        void opAad();
        void opCbw();
        void opCwd();
        void opAndRegMem();
        void opAndImmToAcc();
        void opOrRegMem();
        void opOrImmToAcc();
        void opXorRegMem();
        void opXorImmToAcc();
        void opTestRegMem();
        void opTestImmAndAcc();
        void opMovString();
        void opCmpString();
        void opScaString();
        void opLodString();
        void opStoString();
        void opCallNearProc();
        void opCallFarProc();
        void opRetIntraseg();
        void opRetIntrasegImm();
        void opRetInterseg();
        void opRetIntersegImm();
        void opJmpNear();
        void opJmpShort();
        void opJmpFar();
        void opJoShort();
        void opJneShort();
        void opJbJnaeJcShort();
        void opJnbJaeJncShort();
        void opJneJnzShort();
        void opJbeJnaShort();
        void opJnbeJaShort();
        void opJnsShort();
        void opJpJpeShort();
        void opJnpJpoShort();
        void opJlJngeShort();
        void opJnlJgeShort();
        void opJleJngShort();
        void opJnleJgShort();
        void opLoopShort();
        void opLoopeLoopzShort();
        void opLoopneLoopnzShort();
        void opJcxzShort();
        void opIntImm();
        void opInto();
        void opIret();
        void opClc();
        void opCmc();
        void opStc();
        void opCld();
        void opStd();
        void opCli();
        void opSti();
        void opHlt();
        void opNoop();
        void opLock();
        void opEsc();
        void opAluImmToRegMem();
        void opPopRegMem();
        void opAluComplexToRegMem();
        void opAluMultiRegMem();
        void opIncDecRegMemB();
        void opWordRegMem();
        void opJeJzShort();
        void opJsShort();
        void opBreakpoint();
    };

}
