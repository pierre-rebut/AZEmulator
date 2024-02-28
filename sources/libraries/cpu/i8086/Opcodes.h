//
// Created by pierr on 25/10/2023.
//
#pragma once

#define op_88 opMovRegMem    // mov reg8/mem8,reg8
#define op_89 opMovRegMem    // mov reg16/mem16,reg16
#define op_8a opMovRegMem    // mov reg8,reg8/mem8
#define op_8b opMovRegMem   // mov reg16,reg16/mem16

            // Immediate to Register/m_memory
#define op_c6 opMovImmToRegMem    // mov reg8/mem8,immed8
#define op_c7 opMovImmToRegMem   // mov reg16/mem16,immed16

            // Immediate to Register
#define op_b0 opMovImmToReg    // mov al,immed8
#define op_b1 opMovImmToReg    // mov cl,immed8
#define op_b2 opMovImmToReg    // mov dl,immed8
#define op_b3 opMovImmToReg    // mov bl,immed8
#define op_b4 opMovImmToReg    // mov ah,immed8
#define op_b5 opMovImmToReg    // mov ch,immed8
#define op_b6 opMovImmToReg    // mov dh,immed8
#define op_b7 opMovImmToReg    // mov bh,immed8
#define op_b8 opMovImmToReg    // mov ax,immed16
#define op_b9 opMovImmToReg    // mov cx,immed16
#define op_ba opMovImmToReg    // mov dx,immed16
#define op_bb opMovImmToReg    // mov bx,immed16
#define op_bc opMovImmToReg    // mov sp,immed16
#define op_bd opMovImmToReg    // mov bp,immed16
#define op_be opMovImmToReg    // mov si,immed16
#define op_bf opMovImmToReg    // mov di,immed16

            // m_memory to/from Accumulator
#define op_a0 opMovMemToAcc    // mov al,mem8
#define op_a1 opMovMemToAcc    // mov ax,mem16
#define op_a2 opMovMemToAcc    // mov mem8,al
#define op_a3 opMovMemToAcc    // mov mem16,ax

            // Register/m_memory to/from Segment Register
#define op_8c opMovRegMemSegment    // mov reg16/mem16,segreg
#define op_8e opMovRegMemSegment    // mov segreg,reg16/mem16

            // Register
#define op_50 opPushReg    // push ax
#define op_51 opPushReg    // push cx
#define op_52 opPushReg    // push dx
#define op_53 opPushReg    // push bx
#define op_54 opPushReg    // push sp
#define op_55 opPushReg    // push bp
#define op_56 opPushReg    // push si
#define op_57 opPushReg    // push di

            // Segment Register
#define op_06 opPushSegReg    // push es
#define op_0e opPushSegReg    // push cs
#define op_16 opPushSegReg    // push ss
#define op_1e opPushSegReg    // push ds

            // Register
#define op_58 opPopReg    // pop ax
#define op_59 opPopReg    // pop cx
#define op_5a opPopReg    // pop dx
#define op_5b opPopReg    // pop bx
#define op_5c opPopReg    // pop sp
#define op_5d opPopReg    // pop bp
#define op_5e opPopReg    // pop si
#define op_5f opPopReg   // pop di

            // Segment Register
#define op_07 opPopSegReg    // pop es
#define op_0f opPopSegReg    // pop cs
#define op_17 opPopSegReg    // pop ss
#define op_1f opPopSegReg    // pop ds

            // Register/m_memory with Register
#define op_86 opXchgRegMem    // xchg reg8,reg8/mem8
#define op_87 opXchgRegMem    // xchg reg16,reg16/mem16

            // Register with Accumulator
#define op_91 opXchgRegWithAcc    // xchg ax,cx
#define op_92 opXchgRegWithAcc    // xchg ax,dx
#define op_93 opXchgRegWithAcc    // xchg ax,bx
#define op_94 opXchgRegWithAcc    // xchg ax,sp
#define op_95 opXchgRegWithAcc    // xchg ax,bp
#define op_96 opXchgRegWithAcc    // xchg ax,si
#define op_97 opXchgRegWithAcc    // xchg ax,di

#define op_d7 opXlat     // opXlat source-table

            // Variable Port
#define op_e4 opInAccImm    // in al,immed8
#define op_e5 opInAccImm    // in ax,immed8

            // Fixed Port
#define op_ec opInAccDx    // in al,dx
#define op_ed opInAccDx    // in ax,dx

            // Variable Port
#define op_e6 opOutAccImm    // out al,immed8
#define op_e7 opOutAccImm    // out ax,immed8

            // Fixed Port
#define op_ee opOutAccDx    // out al,dx
#define op_ef opOutAccDx    // out ax,dx

#define op_8d opLeaRegMem   // lea reg16,mem16

#define op_c5 opLdsRegMem   // lds reg16,mem32

#define op_c4 opLesRegMem   // les reg16,mem32

#define op_9f opLahf        // opLahf

#define op_9e opSahf        // opSahf

#define op_9c opPushf       // pushf

#define op_9d opPopf        // popf

            // Reg./m_memory and Register to Either
#define op_00 opAddRegMem    // add reg8/mem8,reg8
#define op_01 opAddRegMem    // add reg16/mem16,reg16
#define op_02 opAddRegMem    // add reg8,reg8/mem8
#define op_03 opAddRegMem    // add reg16,reg16/mem16

            // Immediate to Accumulator
#define op_04 opAddImmToAcc    // add al,immed8
#define op_05 opAddImmToAcc    // add ax,immed16

            // Reg./m_memory with Register to Either
#define op_10 opAdcRegMem    // adc reg8/mem8,reg8
#define op_11 opAdcRegMem    // adc reg16/mem16,reg16
#define op_12 opAdcRegMem    // adc reg8,reg8/mem8
#define op_13 opAdcRegMem    // adc reg16,reg16/mem16

            // Immediate to Accumulator
#define op_14 opAdcImmWithAcc    // adc al,immed8
#define op_15 opAdcImmWithAcc    // adc ax,immed16

            // Register
#define op_40 opIncReg    // inc ax
#define op_41 opIncReg    // inc cx
#define op_42 opIncReg    // inc dx
#define op_43 opIncReg    // inc bx
#define op_44 opIncReg    // inc sp
#define op_45 opIncReg    // inc bp
#define op_46 opIncReg    // inc si
#define op_47 opIncReg    // inc di

#define op_37 opAaa    // aaa

#define op_27 opDaa    // daa

            // Reg./m_memory and Register to Either
#define op_28 opSubRegMem    // sub reg8/mem8,reg8
#define op_29 opSubRegMem    // sub reg16/mem16,reg16
#define op_2a opSubRegMem    // sub reg8,reg8/mem8
#define op_2b opSubRegMem    // sub reg16,reg16/mem16

            // Immediate from Accumulator
#define op_2c opSubImmFromAcc    // sub al,immed8
#define op_2d opSubImmFromAcc    // sub ax,immed16

            // Reg./m_memory with Register to Either
#define op_18 opSbbRegMem    // sbb reg8/mem8,reg8
#define op_19 opSbbRegMem    // sbb reg16/mem16,reg16
#define op_1a opSbbRegMem    // sbb reg8,reg8/mem8
#define op_1b opSbbRegMem    // sbb reg16,reg16/mem16

            // Immediate to Accumulator
#define op_1c opSbbImmToAcc    // sbb al,immed8
#define op_1d opSbbImmToAcc    // sbb ax,immed16

            // Register
#define op_48 opDecReg    // dec ax
#define op_49 opDecReg    // dec cx
#define op_4a opDecReg    // dec dx
#define op_4b opDecReg    // dec bx
#define op_4c opDecReg    // dec sp
#define op_4d opDecReg    // dec bp
#define op_4e opDecReg    // dec si
#define op_4f opDecReg    // dec di

            // Register/m_memory and Register
#define op_38 opCmpRegMem    // cmp reg8/mem8,reg8
#define op_39 opCmpRegMem    // cmp reg16/mem16,reg16
#define op_3a opCmpRegMem    // cmp reg8,reg8/mem8
#define op_3b opCmpRegMem    // cmp reg16,reg16/mem16

            // Immediate with Accumulator
#define op_3c opCmpAccWithImm    // cmp al,immed8
#define op_3d opCmpAccWithImm   // cmp ax,immed16

#define op_3f opAas     // AAS

#define op_2f opDas    // DAS

#define op_d4 opAam    // AAM

#define op_d5 opAad    // AAD

#define op_98 opCbw    // CBW

#define op_99 opCwd    // CWD

            // Register/m_memory and Register
#define op_20 opAndRegMem    // and reg8/mem8,reg8
#define op_21 opAndRegMem    // and reg16/mem16,reg16
#define op_22 opAndRegMem    // and reg8,reg8/mem8
#define op_23 opAndRegMem    // and reg16,reg16/mem16

            // Immediate to Accumulator
#define op_24 opAndImmToAcc    // and al,immed8
#define op_25 opAndImmToAcc    // and ax,immed16

            // Register/m_memory and Register
#define op_08 opOrRegMem    // or reg8/mem8,reg8
#define op_09 opOrRegMem    // or reg16/mem16,reg16
#define op_0a opOrRegMem    // or reg8,reg8/mem8
#define op_0b opOrRegMem    // or reg16,reg16/mem16

            // Immediate to Accumulator
#define op_0c opOrImmToAcc    // or al,immed8
#define op_0d opOrImmToAcc    // or ax,immed16

            // Register/m_memory and Register
#define op_30 opXorRegMem    // xor reg8/mem8,reg8
#define op_31 opXorRegMem    // xor reg16/mem16,reg16
#define op_32 opXorRegMem    // xor reg8,reg8/mem8
#define op_33 opXorRegMem    // xor reg16,reg16/mem16

            // Immediate to Accumulator
#define op_34 opXorImmToAcc    // xor al,immed8
#define op_35 opXorImmToAcc    // xor ax,immed16

            // Register/m_memory and Register
#define op_84 opTestRegMem    // test reg8/mem8,reg8
#define op_85 opTestRegMem    // test reg16/mem16,reg16

            // Immediate and Accumulator
#define op_a8 opTestImmAndAcc    // test al,immed8
#define op_a9 opTestImmAndAcc    // test ax,immed16

#define op_a4 opMovString    // movs dest-str8,src-str8
#define op_a5 opMovString    // movs dest-str16,src-str16

#define op_a6 opCmpString    // cmps dest-str8,src-str8
#define op_a7 opCmpString    // cmps dest-str16,src-str16

#define op_ae opScaString    // scas dest-str8
#define op_af opScaString    // scas dest-str16

#define op_ac opLodString    // lods src-str8
#define op_ad opLodString    // lods src-str16

#define op_aa opStoString    // stos dest-str8
#define op_ab opStoString    // stos dest-str16

            // Direct with Segment
#define op_e8 opCallNearProc   // call near-proc

            // Direct Intersegment
#define op_9a opCallFarProc   // call far-proc

            // Within Segment
#define op_c3 opRetIntraseg   // RET (intrasegment)

            // Within Seg Adding Immed to SP
#define op_c2 opRetIntrasegImm   // ret immed16 (intraseg)

            // Intersegment
#define op_cb opRetInterseg    // RET (intersegment)

            // Intersegment Adding Immediate to SP
#define op_ca opRetIntersegImm   // ret immed16 (intersegment)

            // Direct within Segment
#define op_e9 opJmpNear   // jmp near-label

            // Direct within Segment-Short
#define op_eb opJmpShort   // jmp short-label

            // Direct Intersegment
#define op_ea opJmpFar  // jmp far-label

#define op_70 opJoShort   // jo short-label

#define op_71 opJneShort   // jno short-label

#define op_72 opJbJnaeJcShort  // jb/jnae/jc short-label

#define op_73 opJnbJaeJncShort // jnb/jae/jnc short-label

#define op_74 opJeJzShort    // je/jz short-label

#define op_75 opJneJnzShort   // jne/jnz short-label

#define op_76 opJbeJnaShort  // jbe/jna short-label

#define op_77 opJnbeJaShort  // jnbe/ja short-label

#define op_78 opJsShort      // js short-label

#define op_79 opJnsShort     // jns short-label

#define op_7a opJpJpeShort   // jp/jpe short-label

#define op_7b opJnpJpoShort  // jnp/jpo short-label

#define op_7c opJlJngeShort  // jl/jnge short-label

#define op_7d opJnlJgeShort  // jnl/jge short-label

#define op_7e opJleJngShort  // jle/jng short-label

#define op_7f opJnleJgShort  // jnle/jg short-label

#define op_e2 opLoopShort    // loop short-label

#define op_e1 opLoopeLoopzShort   // loope/loopz short-label

#define op_e0 opLoopneLoopnzShort  // loopne/loopnz short-label

#define op_e3 opJcxzShort  // jcxz short-label

            // Type 3
#define op_cc opIntImm    // INT 3
            // Type Specified
#define op_cd opIntImm   // int immed8

#define op_ce opInto   // INTO

#define op_cf opIret   // IRET

#define op_f8 opClc   // CLC

#define op_f5 opCmc  // CMC

#define op_f9 opStc  // STC

#define op_fc opCld   // CLD

#define op_fd opStd   // STD

#define op_fa opCli   // CLI

#define op_fb opSti   // STI

#define op_f4 opHlt   // HLT

#define op_9b opNoop   // WAIT

#define op_d8 opEsc    // esc 0,source
#define op_d9 opEsc    // esc 1,source
#define op_da opEsc    // esc 2,source
#define op_db opEsc    // esc 3,source
#define op_dc opEsc    // esc 4,source
#define op_dd opEsc    // esc 5,source
#define op_de opEsc    // esc 6,source
#define op_df opEsc   // esc 7,source

#define op_f0 opLock    // LOCK

#define op_90 opNoop   // NOP

#define op_80 opAluImmToRegMem
            // add reg8/mem8,immed8
            // or reg8/mem8,immed8
            // adc reg8/mem8,immed8
            // sbb reg8/mem8,immed8
            // and reg8/mem8,immed8
            // sub reg8/mem8,immed8
            // xor reg8/mem8,immed8
            // cmp reg8/mem8,immed8
#define op_81 opAluImmToRegMem
            // add reg16/mem16,immed16
            // or reg16/mem16,immed16
            // adc reg16/mem16,immed16
            // sbb reg16/mem16,immed16
            // and reg16/mem16,immed16
            // sub reg16/mem16,immed16
            // xor reg16/mem16,immed16
            // cmp reg16/mem16,immed16
#define op_82 opAluImmToRegMem
            // add reg8/mem8,immed8
            // adc reg8/mem8,immed8
            // sbb reg8/mem8,immed8
            // sub reg8/mem8,immed8
            // cmp reg8/mem8,immed8
#define op_83 opAluImmToRegMem
            // add reg16/mem16,immed8
            // adc reg16/mem16,immed8
            // sbb reg16/mem16,immed8
            // sub reg16/mem16,immed8
            // cmp reg16/mem16,immed8

#define op_8f opPopRegMem // POP REG16/MEM16

#define op_d0 opAluComplexToRegMem
            // rol reg8/mem8,1
            // ror reg8/mem8,1
            // rcl reg8/mem8,1
            // rcr reg8/mem8,1
            // sal/shl reg8/mem8,1
            // shr reg8/mem8,1
            // sar reg8/mem8,1
#define op_d1 opAluComplexToRegMem
            // rol reg16/mem16,1
            // ror reg16/mem16,1
            // rcl reg16/mem16,1
            // rcr reg16/mem16,1
            // sal/shl reg16/mem16,1
            // shr reg16/mem16,1
            // sar reg16/mem16,1
#define op_d2 opAluComplexToRegMem
            // rol reg8/mem8,cl
            // ror reg8/mem8,cl
            // rcl reg8/mem8,cl
            // rcr reg8/mem8,cl
            // sal/shl reg8/mem8,cl
            // shr reg8/mem8,cl
            // sar reg8/mem8,cl
#define op_d3 opAluComplexToRegMem
            // rol reg16/mem16,cl
            // ror reg16/mem16,cl
            // rcl reg16/mem16,cl
            // rcr reg16/mem16,cl
            // sal/shl reg16/mem16,cl
            // shr reg16/mem16,cl
            // sar reg16/mem16,cl

#define op_f6 opAluMultiRegMem
            // test reg8/mem8,immed8
            // not reg8/mem8
            // neg reg8/mem8
            // mul reg8/mem8
            // imul reg8/mem8
            // div reg8/mem8
            // idiv reg8/mem8
#define op_f7 opAluMultiRegMem
            // test reg16/mem16,immed16
            // not reg16/mem16
            // neg reg16/mem16
            // mul reg16/mem16
            // imul reg16/mem16
            // div reg16/mem16
            // idiv reg16/mem16

#define op_fe opIncDecRegMemB
            // inc reg8/mem8
            // dec reg8/mem8

#define op_ff opWordRegMem

#define op_f1 opBreakpoint

#define op_26 opUnknown
#define op_2e opUnknown
#define op_36 opUnknown
#define op_3e opUnknown
#define op_60 opUnknown
#define op_61 opUnknown
#define op_62 opUnknown
#define op_63 opUnknown
#define op_64 opUnknown
#define op_65 opUnknown
#define op_66 opUnknown
#define op_67 opUnknown
#define op_68 opUnknown
#define op_69 opUnknown
#define op_6a opUnknown
#define op_6b opUnknown
#define op_6c opUnknown
#define op_6d opUnknown
#define op_6e opUnknown
#define op_6f opUnknown
#define op_c0 opUnknown
#define op_c1 opUnknown
#define op_c8 opUnknown
#define op_c9 opUnknown
#define op_d6 opUnknown
#define op_f2 opUnknown
#define op_f3 opUnknown