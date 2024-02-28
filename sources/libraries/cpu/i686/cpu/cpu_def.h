#include "EngineLib/data/Types.h"
using namespace Astra::CPU;

#define ES       0
#define CS       1
#define SS       2
#define DS       3
#define FS       4
#define GS       5
#define SEG_TR   6
#define SEG_GDTR 7
#define SEG_LDTR 8
#define SEG_IDTR 9
#define EAX      0
#define ECX      1
#define EDX      2
#define EBX      3
#define ESP      4
#define EBP      5
#define ESI      6
#define EDI      7
#define EZR      8
#define ETMP     9
#define AX       0
#define CX       2
#define DX       4
#define BX       6
#define SP       8
#define BP       10
#define SI       12
#define DI       14
#define ZR       16
#define TMP      18
#define AL       0
#define CL       4
#define DL       8
#define BL       12
#define AH       1
#define CH       5
#define DH       9
#define BH       13
#define ZR8      32

#define CR0_PE         1
#define CR0_MP         2
#define CR0_EM         4
#define CR0_TS         8
#define CR0_ET         16
#define CR0_NE         32
#define CR0_WP         65536
#define CR0_NW         (1 << 29)
#define CR0_CD         (1 << 30)
#define CR0_PG         (1 << 31)
#define CR4_VME        (1 << 0)
#define CR4_PVI        (1 << 1)
#define CR4_TSD        (1 << 2)
#define CR4_DE         (1 << 3)
#define CR4_PSE        (1 << 4)
#define CR4_PAE        (1 << 5)
#define CR4_MCE        (1 << 6)
#define CR4_PGE        (1 << 7)
#define CR4_PCE        (1 << 8)
#define CR4_OSFXSR     (1 << 9)
#define CR4_OSXMMEXCPT (1 << 10)
#define CR4_UMIP       (1 << 11)
#define CR4_LA57       (1 << 12)
#define CR4_VMXE       (1 << 13)
#define CR4_SMXE       (1 << 14)
#define CR4_FSGSBASE   (1 << 16)
#define CR4_PCIDE      (1 << 17)
#define CR4_OSXSAVE    (1 << 18)
#define CR4_SMEP       (1 << 20)
#define CR4_SMAP       (1 << 21)
#define CR4_PKE        (1 << 22)

#define EFLAGS_CF   0x01
#define EFLAGS_PF   0x04
#define EFLAGS_AF   0x10
#define EFLAGS_ZF   0x40
#define EFLAGS_SF   0x80
#define EFLAGS_TF   0x100
#define EFLAGS_IF   0x200
#define EFLAGS_DF   0x400
#define EFLAGS_OF   0x800
#define EFLAGS_IOPL 0x3000
#define EFLAGS_NT   0x4000
#define EFLAGS_RF   0x10000
#define EFLAGS_VM   0x20000
#define EFLAGS_AC   0x40000
#define EFLAGS_VIF  0x80000
#define EFLAGS_VIP  0x100000
#define EFLAGS_ID   0x200000

#define TRACE_INFO_ENTRIES (64 * 1024)
#define TRACE_CACHE_SIZE   (TRACE_INFO_ENTRIES * 8)
#define MAX_TRACE_SIZE     32
#define MAX_TLB_ENTRIES    8192

#define EXIT_STATUS_NORMAL 0
#define EXIT_STATUS_IRQ    1
#define EXIT_STATUS_ASYNC  2
#define EXIT_STATUS_HLT    3
#define MEM_RDONLY         1
#define U64(x)             x##ULL
#define FLOAT128
#define FLOAT16
#define FLOATX80

#define FLOATX80_PI_EXP   (0x4000)
#define FLOAT_PI_HI       (U64(0xc90fdaa22168c234))
#define FLOAT_PI_LO       (U64(0xC000000000000000))
#define FLOATX80_PI2_EXP  (0x3FFF)
#define FLOATX80_PI4_EXP  (0x3FFE)
#define FLOATX80_3PI4_EXP (0x4000)
#define FLOAT_3PI4_HI     (U64(0x96cbe3f9990e91a7))
#define FLOAT_3PI4_LO     (U64(0x9000000000000000))
#define FLOAT_LN2INV_EXP  (0x3FFF)
#define FLOAT_LN2INV_HI   (U64(0xb8aa3b295c17f0bb))
#define FLOAT_LN2INV_LO   (U64(0xC000000000000000))
#define RAISE_SW_C1       0x0200
#define optype            struct CpuInternal::decoded_instruction *

typedef WORD float16;
typedef DWORD float32;
typedef LARGE float64;
