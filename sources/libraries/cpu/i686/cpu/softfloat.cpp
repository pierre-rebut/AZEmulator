#include "cpu.h"

WORD CpuInternal::shift16RightJamming(WORD a, int count)
{
    WORD z;
    if (count == 0) {
        z = a;
    } else if (count < 16) {
        z = (a >> count) | ((a << ((-count) & 15)) != 0);
    } else {
        z = (a != 0);
    }
    return z;
}
DWORD CpuInternal::shift32RightJamming(DWORD a, int count)
{
    DWORD z;
    if (count == 0) {
        z = a;
    } else if (count < 32) {
        z = (a >> count) | ((a << ((-count) & 31)) != 0);
    } else {
        z = (a != 0);
    }
    return z;
}
LARGE CpuInternal::shift64RightJamming(LARGE a, int count)
{
    LARGE z;
    if (count == 0) {
        z = a;
    } else if (count < 64) {
        z = (a >> count) | ((a << ((-count) & 63)) != 0);
    } else {
        z = (a != 0);
    }
    return z;
}
void CpuInternal::shift64ExtraRightJamming(LARGE a0, LARGE a1, int count, LARGE *z0Ptr, LARGE *z1Ptr)
{
    LARGE z0, z1;
    int      negCount = (-count) & 63;
    if (count == 0) {
        z1 = a1;
        z0 = a0;
    } else if (count < 64) {
        z1 = (a0 << negCount) | (a1 != 0);
        z0 = a0 >> count;
    } else {
        if (count == 64) {
            z1 = a0 | (a1 != 0);
        } else {
            z1 = ((a0 | a1) != 0);
        }
        z0 = 0;
    }
    *z1Ptr = z1;
    *z0Ptr = z0;
}
void CpuInternal::add128(LARGE a0, LARGE a1, LARGE b0, LARGE b1, LARGE *z0Ptr, LARGE *z1Ptr)
{
    LARGE z1 = a1 + b1;
    *z1Ptr      = z1;
    *z0Ptr      = a0 + b0 + (z1 < a1);
}
void CpuInternal::sub128(LARGE a0, LARGE a1, LARGE b0, LARGE b1, LARGE *z0Ptr, LARGE *z1Ptr)
{
    *z1Ptr = a1 - b1;
    *z0Ptr = a0 - b0 - (a1 < b1);
}
void CpuInternal::mul64To128(LARGE a, LARGE b, LARGE *z0Ptr, LARGE *z1Ptr)
{
    DWORD aHigh, aLow, bHigh, bLow;
    LARGE z0, zMiddleA, zMiddleB, z1;
    aLow     = (DWORD)a;
    aHigh    = (DWORD)(a >> 32);
    bLow     = (DWORD)b;
    bHigh    = (DWORD)(b >> 32);
    z1       = ((LARGE)aLow) * bLow;
    zMiddleA = ((LARGE)aLow) * bHigh;
    zMiddleB = ((LARGE)aHigh) * bLow;
    z0       = ((LARGE)aHigh) * bHigh;
    zMiddleA += zMiddleB;
    z0 += (((LARGE)(zMiddleA < zMiddleB)) << 32) + (zMiddleA >> 32);
    zMiddleA <<= 32;
    z1 += zMiddleA;
    z0 += (z1 < zMiddleA);
    *z1Ptr = z1;
    *z0Ptr = z0;
}
LARGE CpuInternal::estimateDiv128To64(LARGE a0, LARGE a1, LARGE b)
{
    LARGE b0, b1;
    LARGE rem0, rem1, term0, term1;
    LARGE z;
    if (b <= a0)
        return 0xFFFFFFFFFFFFFFFFULL;
    b0 = b >> 32;
    z  = (b0 << 32 <= a0) ? 0xFFFFFFFF00000000ULL : (a0 / b0) << 32;
    mul64To128(b, z, &term0, &term1);
    sub128(a0, a1, term0, term1, &rem0, &rem1);
    while (((int64_t)rem0) < 0) {
        z -= 0x100000000ULL;
        b1 = b << 32;
        add128(rem0, rem1, b0, b1, &rem0, &rem1);
    }
    rem0 = (rem0 << 32) | (rem1 >> 32);
    z |= (b0 << 32 <= rem0) ? 0xFFFFFFFF : rem0 / b0;
    return z;
}
DWORD CpuInternal::estimateSqrt32(int16_t aExp, DWORD a)
{
    static const WORD sqrtOddAdjustments[]  = {0x0004, 0x0022, 0x005D, 0x00B1, 0x011D, 0x019F, 0x0236, 0x02E0,
                                                   0x039C, 0x0468, 0x0545, 0x0631, 0x072B, 0x0832, 0x0946, 0x0A67};
    static const WORD sqrtEvenAdjustments[] = {0x0A2D, 0x08AF, 0x075A, 0x0629, 0x051A, 0x0429, 0x0356, 0x029E,
                                                   0x0200, 0x0179, 0x0109, 0x00AF, 0x0068, 0x0034, 0x0012, 0x0002};
    DWORD              z;
    int                   index = (a >> 27) & 15;
    if (aExp & 1) {
        z = 0x4000 + (a >> 17) - sqrtOddAdjustments[index];
        z = ((a / z) << 14) + (z << 15);
        a >>= 1;
    } else {
        z = 0x8000 + (a >> 17) - sqrtEvenAdjustments[index];
        z = a / z + z;
        z = (0x20000 <= z) ? 0xFFFF8000 : (z << 15);
        if (z <= a)
            return (DWORD)(((int32_t)a) >> 1);
    }
    return ((DWORD)((((LARGE)a) << 31) / z)) + (z >> 1);
}
int CpuInternal::countLeadingZeros16(WORD a)
{
    int shiftCount = 0;
    if (a < 0x100) {
        shiftCount += 8;
        a <<= 8;
    }
    shiftCount += countLeadingZeros8[a >> 8];
    return shiftCount;
}
int CpuInternal::countLeadingZeros32(DWORD a)
{
    int shiftCount = 0;
    if (a < 0x10000) {
        shiftCount += 16;
        a <<= 16;
    }
    if (a < 0x1000000) {
        shiftCount += 8;
        a <<= 8;
    }
    shiftCount += countLeadingZeros8[a >> 24];
    return shiftCount;
}
int CpuInternal::countLeadingZeros64(LARGE a)
{
    int shiftCount = 0;
    if (a < (LARGE)0x100000000ULL) {
        shiftCount += 32;
    } else {
        a >>= 32;
    }
    shiftCount += countLeadingZeros32((DWORD)(a));
    return shiftCount;
}
void CpuInternal::shift128Right(LARGE a0, LARGE a1, int count, LARGE *z0Ptr, LARGE *z1Ptr)
{
    LARGE z0, z1;
    int      negCount = (-count) & 63;
    if (count == 0) {
        z1 = a1;
        z0 = a0;
    } else if (count < 64) {
        z1 = (a0 << negCount) | (a1 >> count);
        z0 = a0 >> count;
    } else {
        z1 = (count < 128) ? (a0 >> (count & 63)) : 0;
        z0 = 0;
    }
    *z1Ptr = z1;
    *z0Ptr = z0;
}
void CpuInternal::shift128RightJamming(LARGE a0, LARGE a1, int count, LARGE *z0Ptr, LARGE *z1Ptr)
{
    LARGE z0, z1;
    int      negCount = (-count) & 63;
    if (count == 0) {
        z1 = a1;
        z0 = a0;
    } else if (count < 64) {
        z1 = (a0 << negCount) | (a1 >> count) | ((a1 << negCount) != 0);
        z0 = a0 >> count;
    } else {
        if (count == 64) {
            z1 = a0 | (a1 != 0);
        } else if (count < 128) {
            z1 = (a0 >> (count & 63)) | (((a0 << negCount) | a1) != 0);
        } else {
            z1 = ((a0 | a1) != 0);
        }
        z0 = 0;
    }
    *z1Ptr = z1;
    *z0Ptr = z0;
}
void CpuInternal::shortShift128Left(LARGE a0, LARGE a1, int count, LARGE *z0Ptr, LARGE *z1Ptr)
{
    *z1Ptr = a1 << count;
    *z0Ptr = (count == 0) ? a0 : (a0 << count) | (a1 >> ((-count) & 63));
}
void CpuInternal::add192(LARGE a0, LARGE a1, LARGE a2, LARGE b0, LARGE b1, LARGE b2, LARGE *z0Ptr,
                         LARGE *z1Ptr, LARGE *z2Ptr)
{
    LARGE z0, z1, z2;
    unsigned carry0, carry1;
    z2     = a2 + b2;
    carry1 = (z2 < a2);
    z1     = a1 + b1;
    carry0 = (z1 < a1);
    z0     = a0 + b0;
    z1 += carry1;
    z0 += (z1 < carry1);
    z0 += carry0;
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;
}
void CpuInternal::sub192(LARGE a0, LARGE a1, LARGE a2, LARGE b0, LARGE b1, LARGE b2, LARGE *z0Ptr,
                         LARGE *z1Ptr, LARGE *z2Ptr)
{
    LARGE z0, z1, z2;
    unsigned borrow0, borrow1;
    z2      = a2 - b2;
    borrow1 = (a2 < b2);
    z1      = a1 - b1;
    borrow0 = (a1 < b1);
    z0      = a0 - b0;
    z0 -= (z1 < borrow1);
    z1 -= borrow1;
    z0 -= borrow0;
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;
}
int CpuInternal::eq128(LARGE a0, LARGE a1, LARGE b0, LARGE b1)
{
    return (a0 == b0) && (a1 == b1);
}
int CpuInternal::le128(LARGE a0, LARGE a1, LARGE b0, LARGE b1)
{
    return (a0 < b0) || ((a0 == b0) && (a1 <= b1));
}
int CpuInternal::lt128(LARGE a0, LARGE a1, LARGE b0, LARGE b1)
{
    return (a0 < b0) || ((a0 == b0) && (a1 < b1));
}
void CpuInternal::mul128By64To192(LARGE a0, LARGE a1, LARGE b, LARGE *z0Ptr, LARGE *z1Ptr, LARGE *z2Ptr)
{
    LARGE z0, z1, z2, more1;
    mul64To128(a1, b, &z1, &z2);
    mul64To128(a0, b, &z0, &more1);
    add128(z0, more1, 0, z1, &z0, &z1);
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;
}
void CpuInternal::mul128To256(LARGE a0, LARGE a1, LARGE b0, LARGE b1, LARGE *z0Ptr, LARGE *z1Ptr,
                              LARGE *z2Ptr, LARGE *z3Ptr)
{
    LARGE z0, z1, z2, z3;
    LARGE more1, more2;
    mul64To128(a1, b1, &z2, &z3);
    mul64To128(a1, b0, &z1, &more2);
    add128(z1, more2, 0, z2, &z1, &z2);
    mul64To128(a0, b0, &z0, &more1);
    add128(z0, more1, 0, z1, &z0, &z1);
    mul64To128(a0, b1, &more1, &more2);
    add128(more1, more2, 0, z2, &more1, &z2);
    add128(z0, z1, 0, more1, &z0, &z1);
    *z3Ptr = z3;
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;
}
void CpuInternal::shift128ExtraRightJamming(LARGE a0, LARGE a1, LARGE a2, int count, LARGE *z0Ptr, LARGE *z1Ptr,
                                            LARGE *z2Ptr)
{
    LARGE z0, z1, z2;
    int      negCount = (-count) & 63;
    if (count == 0) {
        z2 = a2;
        z1 = a1;
        z0 = a0;
    } else {
        if (count < 64) {
            z2 = a1 << negCount;
            z1 = (a0 << negCount) | (a1 >> count);
            z0 = a0 >> count;
        } else {
            if (count == 64) {
                z2 = a1;
                z1 = a0;
            } else {
                a2 |= a1;
                if (count < 128) {
                    z2 = a0 << negCount;
                    z1 = a0 >> (count & 63);
                } else {
                    z2 = (count == 128) ? a0 : (a0 != 0);
                    z1 = 0;
                }
            }
            z0 = 0;
        }
        z2 |= (a2 != 0);
    }
    *z2Ptr = z2;
    *z1Ptr = z1;
    *z0Ptr = z0;
}
WORD CpuInternal::extractFloat16Frac(float16 a)
{
    return a & 0x3FF;
}
int16_t CpuInternal::extractFloat16Exp(float16 a)
{
    return (a >> 10) & 0x1F;
}
int CpuInternal::extractFloat16Sign(float16 a)
{
    return a >> 15;
}
float16 CpuInternal::packFloat16(int zSign, int zExp, WORD zSig)
{
    return (((WORD)zSign) << 15) + (((WORD)zExp) << 10) + zSig;
}
int CpuInternal::float16_is_nan(float16 a)
{
    return (0xF800 < (WORD)(a << 1));
}
int CpuInternal::float16_is_signaling_nan(float16 a)
{
    return (((a >> 9) & 0x3F) == 0x3E) && (a & 0x1FF);
}
int CpuInternal::float16_is_denormal(float16 a)
{
    return (extractFloat16Exp(a) == 0) && (extractFloat16Frac(a) != 0);
}
float16 CpuInternal::float16_denormal_to_zero(float16 a)
{
    if (float16_is_denormal(a))
        a &= 0x8000;
    return a;
}
CpuInternal::commonNaNT CpuInternal::float16ToCommonNaN(float16 a, float_status_t *status)
{
    CpuInternal::commonNaNT z;
    if (float16_is_signaling_nan(a))
        float_raise(status, float_flag_invalid);
    z.sign = a >> 15;
    z.lo   = 0;
    z.hi   = ((LARGE)a) << 54;
    return z;
}
float16 CpuInternal::commonNaNToFloat16(CpuInternal::commonNaNT a)
{
    return (((WORD)a.sign) << 15) | 0x7E00 | (WORD)(a.hi >> 54);
}
DWORD CpuInternal::extractFloat32Frac(float32 a)
{
    return a & 0x007FFFFF;
}
int16_t CpuInternal::extractFloat32Exp(float32 a)
{
    return (a >> 23) & 0xFF;
}
int CpuInternal::extractFloat32Sign(float32 a)
{
    return a >> 31;
}
float32 CpuInternal::packFloat32(int zSign, int16_t zExp, DWORD zSig)
{
    return (((DWORD)zSign) << 31) + (((DWORD)zExp) << 23) + zSig;
}
int CpuInternal::float32_is_nan(float32 a)
{
    return (0xFF000000 < (DWORD)(a << 1));
}
int CpuInternal::float32_is_signaling_nan(float32 a)
{
    return (((a >> 22) & 0x1FF) == 0x1FE) && (a & 0x003FFFFF);
}
int CpuInternal::float32_is_denormal(float32 a)
{
    return (extractFloat32Exp(a) == 0) && (extractFloat32Frac(a) != 0);
}
float32 CpuInternal::float32_denormal_to_zero(float32 a)
{
    if (float32_is_denormal(a))
        a &= 0x80000000;
    return a;
}
CpuInternal::commonNaNT CpuInternal::float32ToCommonNaN(float32 a, float_status_t *status)
{
    CpuInternal::commonNaNT z;
    if (float32_is_signaling_nan(a))
        float_raise(status, float_flag_invalid);
    z.sign = a >> 31;
    z.lo   = 0;
    z.hi   = ((LARGE)a) << 41;
    return z;
}
float32 CpuInternal::commonNaNToFloat32(CpuInternal::commonNaNT a)
{
    return (((DWORD)a.sign) << 31) | 0x7FC00000 | (DWORD)(a.hi >> 41);
}
float32 CpuInternal::propagateFloat32NaN(float32 a, float_status_t *status)
{
    if (float32_is_signaling_nan(a))
        float_raise(status, float_flag_invalid);
    return a | 0x00400000;
}
LARGE CpuInternal::extractFloat64Frac(float64 a)
{
    return a & 0x000FFFFFFFFFFFFFULL;
}
int16_t CpuInternal::extractFloat64Exp(float64 a)
{
    return (int16_t)(a >> 52) & 0x7FF;
}
int CpuInternal::extractFloat64Sign(float64 a)
{
    return (int)(a >> 63);
}
float64 CpuInternal::packFloat64(int zSign, int16_t zExp, LARGE zSig)
{
    return (((LARGE)zSign) << 63) + (((LARGE)zExp) << 52) + zSig;
}
int CpuInternal::float64_is_nan(float64 a)
{
    return (0xFFE0000000000000ULL < (LARGE)(a << 1));
}
int CpuInternal::float64_is_signaling_nan(float64 a)
{
    return (((a >> 51) & 0xFFF) == 0xFFE) && (a & 0x0007FFFFFFFFFFFFULL);
}
int CpuInternal::float64_is_denormal(float64 a)
{
    return (extractFloat64Exp(a) == 0) && (extractFloat64Frac(a) != 0);
}
float64 CpuInternal::float64_denormal_to_zero(float64 a)
{
    if (float64_is_denormal(a))
        a &= ((LARGE)(1) << 63);
    return a;
}
CpuInternal::commonNaNT CpuInternal::float64ToCommonNaN(float64 a, float_status_t *status)
{
    CpuInternal::commonNaNT z;
    if (float64_is_signaling_nan(a))
        float_raise(status, float_flag_invalid);
    z.sign = (int)(a >> 63);
    z.lo   = 0;
    z.hi   = a << 12;
    return z;
}
float64 CpuInternal::commonNaNToFloat64(CpuInternal::commonNaNT a)
{
    return (((LARGE)a.sign) << 63) | 0x7FF8000000000000ULL | (a.hi >> 12);
}
float64 CpuInternal::propagateFloat64NaN(float64 a, float_status_t *status)
{
    if (float64_is_signaling_nan(a))
        float_raise(status, float_flag_invalid);
    return a | 0x0008000000000000ULL;
}
LARGE CpuInternal::extractFloatx80Frac(floatx80 a)
{
    return a.fraction;
}
int32_t CpuInternal::extractFloatx80Exp(floatx80 a)
{
    return a.exp & 0x7FFF;
}
int CpuInternal::extractFloatx80Sign(floatx80 a)
{
    return a.exp >> 15;
}
floatx80 CpuInternal::packFloatx80(int zSign, int32_t zExp, LARGE zSig)
{
    floatx80 z;
    z.fraction = zSig;
    z.exp      = (zSign << 15) + zExp;
    return z;
}
int CpuInternal::floatx80_is_nan(floatx80 a)
{
    return ((a.exp & 0x7FFF) == 0x7FFF) && (int64_t)(a.fraction << 1);
}
int CpuInternal::floatx80_is_signaling_nan(floatx80 a)
{
    LARGE aLow = a.fraction & ~0x4000000000000000ULL;
    return ((a.exp & 0x7FFF) == 0x7FFF) && ((LARGE)(aLow << 1)) && (a.fraction == aLow);
}
int CpuInternal::floatx80_is_unsupported(floatx80 a)
{
    return ((a.exp & 0x7FFF) && !(a.fraction & 0x8000000000000000ULL));
}
CpuInternal::commonNaNT CpuInternal::floatx80ToCommonNaN(floatx80 a, float_status_t *status)
{
    CpuInternal::commonNaNT z;
    if (floatx80_is_signaling_nan(a))
        float_raise(status, float_flag_invalid);
    z.sign = a.exp >> 15;
    z.lo   = 0;
    z.hi   = a.fraction << 1;
    return z;
}
floatx80 CpuInternal::commonNaNToFloatx80(CpuInternal::commonNaNT a)
{
    floatx80 z;
    z.fraction = 0xC000000000000000ULL | (a.hi >> 1);
    z.exp      = (((WORD)a.sign) << 15) | 0x7FFF;
    return z;
}
floatx80 CpuInternal::propagateFloatx80NaN(floatx80 a, float_status_t *status)
{
    if (floatx80_is_signaling_nan(a))
        float_raise(status, float_flag_invalid);
    a.fraction |= 0xC000000000000000ULL;
    return a;
}
LARGE CpuInternal::extractFloat128Frac1(float128 a)
{
    return a.lo;
}
LARGE CpuInternal::extractFloat128Frac0(float128 a)
{
    return a.hi & 0x0000FFFFFFFFFFFFULL;
}
int32_t CpuInternal::extractFloat128Exp(float128 a)
{
    return ((int32_t)(a.hi >> 48)) & 0x7FFF;
}
int CpuInternal::extractFloat128Sign(float128 a)
{
    return (int)(a.hi >> 63);
}
float128 CpuInternal::packFloat128(int zSign, int32_t zExp, LARGE zSig0, LARGE zSig1)
{
    float128 z;
    z.lo = zSig1;
    z.hi = (((LARGE)zSign) << 63) + (((LARGE)zExp) << 48) + zSig0;
    return z;
}
float128 CpuInternal::packFloat128_simple(LARGE zHi, LARGE zLo)
{
    float128 z;
    z.lo = zLo;
    z.hi = zHi;
    return z;
}
int CpuInternal::float128_is_nan(float128 a)
{
    return (0xFFFE000000000000ULL <= (LARGE)(a.hi << 1)) && (a.lo || (a.hi & 0x0000FFFFFFFFFFFFULL));
}
int CpuInternal::float128_is_signaling_nan(float128 a)
{
    return (((a.hi >> 47) & 0xFFFF) == 0xFFFE) && (a.lo || (a.hi & 0x00007FFFFFFFFFFFULL));
}
CpuInternal::commonNaNT CpuInternal::float128ToCommonNaN(float128 a, float_status_t *status)
{
    CpuInternal::commonNaNT z;
    if (float128_is_signaling_nan(a))
        float_raise(status, float_flag_invalid);
    z.sign = (int)(a.hi >> 63);
    shortShift128Left(a.hi, a.lo, 16, &z.hi, &z.lo);
    return z;
}
float128 CpuInternal::commonNaNToFloat128(CpuInternal::commonNaNT a)
{
    float128 z;
    shift128Right(a.hi, a.lo, 16, &z.hi, &z.lo);
    z.hi |= (((LARGE)a.sign) << 63) | 0x7FFF800000000000ULL;
    return z;
}
float32 CpuInternal::int32_to_float32(int32_t a, float_status_t *status)
{
    if (a == 0)
        return 0;
    if (a == (int32_t)0x80000000)
        return packFloat32(1, 0x9E, 0);
    int zSign = (a < 0);
    return normalizeRoundAndPackFloat32(zSign, 0x9C, zSign ? -a : a, status);
}
float64 CpuInternal::int32_to_float64(int32_t a)
{
    if (a == 0)
        return 0;
    int      zSign      = (a < 0);
    DWORD absA       = zSign ? -a : a;
    int      shiftCount = countLeadingZeros32(absA) + 21;
    LARGE zSig       = absA;
    return packFloat64(zSign, 0x432 - shiftCount, zSig << shiftCount);
}
float32 CpuInternal::int64_to_float32(int64_t a, float_status_t *status)
{
    if (a == 0)
        return 0;
    int      zSign      = (a < 0);
    LARGE absA       = zSign ? -a : a;
    int      shiftCount = countLeadingZeros64(absA) - 40;
    if (0 <= shiftCount) {
        return packFloat32(zSign, 0x95 - shiftCount, (DWORD)(absA << shiftCount));
    } else {
        shiftCount += 7;
        if (shiftCount < 0) {
            absA = shift64RightJamming(absA, -shiftCount);
        } else {
            absA <<= shiftCount;
        }
        return roundAndPackFloat32(zSign, 0x9C - shiftCount, (DWORD)absA, status);
    }
}
float64 CpuInternal::int64_to_float64(int64_t a, float_status_t *status)
{
    if (a == 0)
        return 0;
    if (a == (int64_t)0x8000000000000000ULL) {
        return packFloat64(1, 0x43E, 0);
    }
    int zSign = (a < 0);
    return normalizeRoundAndPackFloat64(zSign, 0x43C, zSign ? -a : a, status);
}
float32 CpuInternal::uint32_to_float32(DWORD a, float_status_t *status)
{
    if (a == 0)
        return 0;
    if (a & 0x80000000)
        return normalizeRoundAndPackFloat32(0, 0x9D, a >> 1, status);
    return normalizeRoundAndPackFloat32(0, 0x9C, a, status);
}
float64 CpuInternal::uint32_to_float64(DWORD a)
{
    if (a == 0)
        return 0;
    int      shiftCount = countLeadingZeros32(a) + 21;
    LARGE zSig       = a;
    return packFloat64(0, 0x432 - shiftCount, zSig << shiftCount);
}
float32 CpuInternal::uint64_to_float32(LARGE a, float_status_t *status)
{
    if (a == 0)
        return 0;
    int shiftCount = countLeadingZeros64(a) - 40;
    if (0 <= shiftCount) {
        return packFloat32(0, 0x95 - shiftCount, (DWORD)(a << shiftCount));
    } else {
        shiftCount += 7;
        if (shiftCount < 0) {
            a = shift64RightJamming(a, -shiftCount);
        } else {
            a <<= shiftCount;
        }
        return roundAndPackFloat32(0, 0x9C - shiftCount, (DWORD)a, status);
    }
}
float64 CpuInternal::uint64_to_float64(LARGE a, float_status_t *status)
{
    if (a == 0)
        return 0;
    if (a & 0x8000000000000000ULL)
        return normalizeRoundAndPackFloat64(0, 0x43D, a >> 1, status);
    return normalizeRoundAndPackFloat64(0, 0x43C, a, status);
}
int32_t CpuInternal::float32_to_int32(float32 a, float_status_t *status)
{
    DWORD aSig  = extractFloat32Frac(a);
    int16_t  aExp  = extractFloat32Exp(a);
    int      aSign = extractFloat32Sign(a);
    if ((aExp == 0xFF) && aSig)
        aSign = 0;
    if (aExp)
        aSig |= 0x00800000;
    else {
        if (get_denormals_are_zeros(status))
            aSig = 0;
    }
    int      shiftCount = 0xAF - aExp;
    LARGE aSig64     = ((LARGE)(aSig)) << 32;
    if (0 < shiftCount)
        aSig64 = shift64RightJamming(aSig64, shiftCount);
    return roundAndPackInt32(aSign, aSig64, status);
}
int32_t CpuInternal::float32_to_int32_round_to_zero(float32 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    DWORD aSig;
    int32_t  z;
    aSig           = extractFloat32Frac(a);
    aExp           = extractFloat32Exp(a);
    aSign          = extractFloat32Sign(a);
    int shiftCount = aExp - 0x9E;
    if (0 <= shiftCount) {
        if (a != 0xCF000000) {
            float_raise(status, float_flag_invalid);
        }
        return (int32_t)(((int32_t)0x80000000));
    } else if (aExp <= 0x7E) {
        if (get_denormals_are_zeros(status) && aExp == 0)
            aSig = 0;
        if (aExp | aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    }
    aSig = (aSig | 0x800000) << 8;
    z    = aSig >> (-shiftCount);
    if ((DWORD)(aSig << (shiftCount & 31))) {
        float_raise(status, float_flag_inexact);
    }
    if (aSign)
        z = -z;
    return z;
}
DWORD CpuInternal::float32_to_uint32_round_to_zero(float32 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    DWORD aSig;
    aSig           = extractFloat32Frac(a);
    aExp           = extractFloat32Exp(a);
    aSign          = extractFloat32Sign(a);
    int shiftCount = aExp - 0x9E;
    if (aExp <= 0x7E) {
        if (get_denormals_are_zeros(status) && aExp == 0)
            aSig = 0;
        if (aExp | aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    } else if (0 < shiftCount || aSign) {
        float_raise(status, float_flag_invalid);
        return (0xffffffff);
    }
    aSig       = (aSig | 0x800000) << 8;
    DWORD z = aSig >> (-shiftCount);
    if (aSig << (shiftCount & 31)) {
        float_raise(status, float_flag_inexact);
    }
    return z;
}
int64_t CpuInternal::float32_to_int64(float32 a, float_status_t *status)
{
    LARGE aSig64, aSigExtra;
    DWORD aSig       = extractFloat32Frac(a);
    int16_t  aExp       = extractFloat32Exp(a);
    int      aSign      = extractFloat32Sign(a);
    int      shiftCount = 0xBE - aExp;
    if (shiftCount < 0) {
        float_raise(status, float_flag_invalid);
        return (int64_t)(0x8000000000000000ULL);
    }
    if (aExp)
        aSig |= 0x00800000;
    else {
        if (get_denormals_are_zeros(status))
            aSig = 0;
    }
    aSig64 = aSig;
    aSig64 <<= 40;
    shift64ExtraRightJamming(aSig64, 0, shiftCount, &aSig64, &aSigExtra);
    return roundAndPackInt64(aSign, aSig64, aSigExtra, status);
}
int64_t CpuInternal::float32_to_int64_round_to_zero(float32 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    DWORD aSig;
    LARGE aSig64;
    int64_t  z;
    aSig           = extractFloat32Frac(a);
    aExp           = extractFloat32Exp(a);
    aSign          = extractFloat32Sign(a);
    int shiftCount = aExp - 0xBE;
    if (0 <= shiftCount) {
        if (a != 0xDF000000) {
            float_raise(status, float_flag_invalid);
        }
        return (int64_t)(0x8000000000000000ULL);
    } else if (aExp <= 0x7E) {
        if (get_denormals_are_zeros(status) && aExp == 0)
            aSig = 0;
        if (aExp | aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    }
    aSig64 = aSig | 0x00800000;
    aSig64 <<= 40;
    z = aSig64 >> (-shiftCount);
    if ((LARGE)(aSig64 << (shiftCount & 63))) {
        float_raise(status, float_flag_inexact);
    }
    if (aSign)
        z = -z;
    return z;
}
LARGE CpuInternal::float32_to_uint64_round_to_zero(float32 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    DWORD aSig;
    LARGE aSig64;
    aSig           = extractFloat32Frac(a);
    aExp           = extractFloat32Exp(a);
    aSign          = extractFloat32Sign(a);
    int shiftCount = aExp - 0xBE;
    if (aExp <= 0x7E) {
        if (get_denormals_are_zeros(status) && aExp == 0)
            aSig = 0;
        if (aExp | aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    } else if (0 < shiftCount || aSign) {
        float_raise(status, float_flag_invalid);
        return 0xffffffffffffffffULL;
    }
    aSig64 = aSig | 0x00800000;
    aSig64 <<= 40;
    LARGE z = aSig64 >> (-shiftCount);
    if ((LARGE)(aSig64 << (shiftCount & 63))) {
        float_raise(status, float_flag_inexact);
    }
    return z;
}
LARGE CpuInternal::float32_to_uint64(float32 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp, shiftCount;
    DWORD aSig;
    LARGE aSig64, aSigExtra;
    aSig  = extractFloat32Frac(a);
    aExp  = extractFloat32Exp(a);
    aSign = extractFloat32Sign(a);
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
    }
    if ((aSign) && (aExp > 0x7E)) {
        float_raise(status, float_flag_invalid);
        return 0xffffffffffffffffULL;
    }
    shiftCount = 0xBE - aExp;
    if (aExp)
        aSig |= 0x00800000;
    if (shiftCount < 0) {
        float_raise(status, float_flag_invalid);
        return 0xffffffffffffffffULL;
    }
    aSig64 = aSig;
    aSig64 <<= 40;
    shift64ExtraRightJamming(aSig64, 0, shiftCount, &aSig64, &aSigExtra);
    return roundAndPackUint64(aSign, aSig64, aSigExtra, status);
}
DWORD CpuInternal::float32_to_uint32(float32 a, float_status_t *status)
{
    LARGE val_64 = float32_to_uint64(a, status);
    if (val_64 > 0xffffffff) {
        status->float_exception_flags = float_flag_invalid;
        return (0xffffffff);
    }
    return (DWORD)val_64;
}
float64 CpuInternal::float32_to_float64(float32 a, float_status_t *status)
{
    DWORD aSig  = extractFloat32Frac(a);
    int16_t  aExp  = extractFloat32Exp(a);
    int      aSign = extractFloat32Sign(a);
    if (aExp == 0xFF) {
        if (aSig)
            return CpuInternal::commonNaNToFloat64(float32ToCommonNaN(a, status));
        return packFloat64(aSign, 0x7FF, 0);
    }
    if (aExp == 0) {
        if (aSig == 0 || get_denormals_are_zeros(status))
            return packFloat64(aSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(aSig, &aExp, &aSig);
        --aExp;
    }
    return packFloat64(aSign, aExp + 0x380, ((LARGE)aSig) << 29);
}
float32 CpuInternal::float32_round_to_int_with_scale(float32 a, BYTE scale, float_status_t *status)
{
    DWORD lastBitMask, roundBitsMask;
    int      roundingMode = get_float_rounding_mode(status);
    int16_t  aExp         = extractFloat32Exp(a);
    scale &= 0xf;
    if ((aExp == 0xFF) && extractFloat32Frac(a)) {
        return propagateFloat32NaN(a, status);
    }
    aExp += scale;
    if (0x96 <= aExp) {
        return a;
    }
    if (get_denormals_are_zeros(status)) {
        a = float32_denormal_to_zero(a);
    }
    if (aExp <= 0x7E) {
        if ((DWORD)(a << 1) == 0)
            return a;
        float_raise(status, float_flag_inexact);
        int aSign = extractFloat32Sign(a);
        switch (roundingMode) {
            case float_round_nearest_even:
                if ((aExp == 0x7E) && extractFloat32Frac(a)) {
                    return packFloat32(aSign, 0x7F - scale, 0);
                }
                break;
            case float_round_down:
                return aSign ? packFloat32(1, 0x7F - scale, 0) : float32_positive_zero;
            case float_round_up:
                return aSign ? float32_negative_zero : packFloat32(0, 0x7F - scale, 0);
        }
        return packFloat32(aSign, 0, 0);
    }
    lastBitMask = 1;
    lastBitMask <<= 0x96 - aExp;
    roundBitsMask = lastBitMask - 1;
    float32 z     = a;
    if (roundingMode == float_round_nearest_even) {
        z += lastBitMask >> 1;
        if ((z & roundBitsMask) == 0)
            z &= ~lastBitMask;
    } else if (roundingMode != float_round_to_zero) {
        if (extractFloat32Sign(z) ^ (roundingMode == float_round_up)) {
            z += roundBitsMask;
        }
    }
    z &= ~roundBitsMask;
    if (z != a)
        float_raise(status, float_flag_inexact);
    return z;
}
float32 CpuInternal::float32_frc(float32 a, float_status_t *status)
{
    int      roundingMode = get_float_rounding_mode(status);
    int16_t  aExp         = extractFloat32Exp(a);
    DWORD aSig         = extractFloat32Frac(a);
    int      aSign        = extractFloat32Sign(a);
    if (aExp == 0xFF) {
        if (aSig)
            return propagateFloat32NaN(a, status);
        float_raise(status, float_flag_invalid);
        return float32_default_nan;
    }
    if (aExp >= 0x96) {
        return packFloat32(roundingMode == float_round_down, 0, 0);
    }
    if (aExp < 0x7F) {
        if (aExp == 0) {
            if (aSig == 0 || get_denormals_are_zeros(status))
                return packFloat32(roundingMode == float_round_down, 0, 0);
            float_raise(status, float_flag_denormal);
            if (!float_exception_masked(status, float_flag_underflow))
                float_raise(status, float_flag_underflow);
            if (get_flush_underflow_to_zero(status)) {
                float_raise(status, float_flag_underflow | float_flag_inexact);
                return packFloat32(aSign, 0, 0);
            }
        }
        return a;
    }
    DWORD lastBitMask   = 1 << (0x96 - aExp);
    DWORD roundBitsMask = lastBitMask - 1;
    aSig &= roundBitsMask;
    aSig <<= 7;
    aExp--;
    if (aSig == 0)
        return packFloat32(roundingMode == float_round_down, 0, 0);
    return normalizeRoundAndPackFloat32(aSign, aExp, aSig, status);
}
float32 CpuInternal::float32_getexp(float32 a, float_status_t *status)
{
    int16_t  aExp = extractFloat32Exp(a);
    DWORD aSig = extractFloat32Frac(a);
    if (aExp == 0xFF) {
        if (aSig)
            return propagateFloat32NaN(a, status);
        return float32_positive_inf;
    }
    if (aExp == 0) {
        if (aSig == 0 || get_denormals_are_zeros(status))
            return float32_negative_inf;
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(aSig, &aExp, &aSig);
    }
    return int32_to_float32(aExp - 0x7F, status);
}
float32 CpuInternal::float32_getmant(float32 a, float_status_t *status, int sign_ctrl, int interv)
{
    int16_t  aExp  = extractFloat32Exp(a);
    DWORD aSig  = extractFloat32Frac(a);
    int      aSign = extractFloat32Sign(a);
    if (aExp == 0xFF) {
        if (aSig)
            return propagateFloat32NaN(a, status);
        if (aSign) {
            if (sign_ctrl & 0x2) {
                float_raise(status, float_flag_invalid);
                return float32_default_nan;
            }
        }
        return packFloat32(~sign_ctrl & aSign, 0x7F, 0);
    }
    if (aExp == 0 && (aSig == 0 || get_denormals_are_zeros(status))) {
        return packFloat32(~sign_ctrl & aSign, 0x7F, 0);
    }
    if (aSign) {
        if (sign_ctrl & 0x2) {
            float_raise(status, float_flag_invalid);
            return float32_default_nan;
        }
    }
    if (aExp == 0) {
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(aSig, &aExp, &aSig);
        aSig &= 0x7FFFFF;
    }
    switch (interv) {
        case 0x0:
            aExp = 0x7F;
            break;
        case 0x1:
            aExp -= 0x7F;
            aExp = 0x7F - (aExp & 0x1);
            break;
        case 0x2:
            aExp = 0x7E;
            break;
        case 0x3:
            aExp = 0x7F - ((aSig >> 22) & 0x1);
            break;
    }
    return packFloat32(~sign_ctrl & aSign, aExp, aSig);
}
float32 CpuInternal::float32_scalef(float32 a, float32 b, float_status_t *status)
{
    DWORD aSig  = extractFloat32Frac(a);
    int16_t  aExp  = extractFloat32Exp(a);
    int      aSign = extractFloat32Sign(a);
    DWORD bSig  = extractFloat32Frac(b);
    int16_t  bExp  = extractFloat32Exp(b);
    int      bSign = extractFloat32Sign(b);
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    if (bExp == 0xFF) {
        if (bSig)
            return propagateFloat32NaN_two_args(a, b, status);
    }
    if (aExp == 0xFF) {
        if (aSig) {
            int aIsSignalingNaN = (aSig & 0x00400000) == 0;
            if (aIsSignalingNaN || bExp != 0xFF || bSig)
                return propagateFloat32NaN_two_args(a, b, status);
            return bSign ? 0 : float32_positive_inf;
        }
        if (bExp == 0xFF && bSign) {
            float_raise(status, float_flag_invalid);
            return float32_default_nan;
        }
        return a;
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if (bExp == 0xFF && !bSign) {
                float_raise(status, float_flag_invalid);
                return float32_default_nan;
            }
            return a;
        }
        float_raise(status, float_flag_denormal);
    }
    if ((bExp | bSig) == 0)
        return a;
    if (bExp == 0xFF) {
        if (bSign)
            return packFloat32(aSign, 0, 0);
        return packFloat32(aSign, 0xFF, 0);
    }
    if (bExp >= 0x8E) {
        return roundAndPackFloat32(aSign, bSign ? -0x7F : 0xFF, aSig, status);
    }
    int scale = 0;
    if (bExp <= 0x7E) {
        if (bExp == 0)
            float_raise(status, float_flag_denormal);
        scale = -bSign;
    } else {
        int shiftCount = bExp - 0x9E;
        bSig           = (bSig | 0x800000) << 8;
        scale          = bSig >> (-shiftCount);
        if (bSign) {
            if ((DWORD)(bSig << (shiftCount & 31)))
                scale++;
            scale = -scale;
        }
        if (scale > 0x200)
            scale = 0x200;
        if (scale < -0x200)
            scale = -0x200;
    }
    if (aExp != 0) {
        aSig |= 0x00800000;
    } else {
        aExp++;
    }
    aExp += scale - 1;
    aSig <<= 7;
    return normalizeRoundAndPackFloat32(aSign, aExp, aSig, status);
}
float32 CpuInternal::addFloat32Sigs(float32 a, float32 b, int zSign, float_status_t *status)
{
    int16_t  aExp, bExp, zExp;
    DWORD aSig, bSig, zSig;
    int16_t  expDiff;
    aSig = extractFloat32Frac(a);
    aExp = extractFloat32Exp(a);
    bSig = extractFloat32Frac(b);
    bExp = extractFloat32Exp(b);
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    expDiff = aExp - bExp;
    aSig <<= 6;
    bSig <<= 6;
    if (0 < expDiff) {
        if (aExp == 0xFF) {
            if (aSig)
                return propagateFloat32NaN_two_args(a, b, status);
            if (bSig && (bExp == 0))
                float_raise(status, float_flag_denormal);
            return a;
        }
        if ((aExp == 0) && aSig)
            float_raise(status, float_flag_denormal);
        if (bExp == 0) {
            if (bSig)
                float_raise(status, float_flag_denormal);
            --expDiff;
        } else
            bSig |= 0x20000000;
        bSig = shift32RightJamming(bSig, expDiff);
        zExp = aExp;
    } else if (expDiff < 0) {
        if (bExp == 0xFF) {
            if (bSig)
                return propagateFloat32NaN_two_args(a, b, status);
            if (aSig && (aExp == 0))
                float_raise(status, float_flag_denormal);
            return packFloat32(zSign, 0xFF, 0);
        }
        if ((bExp == 0) && bSig)
            float_raise(status, float_flag_denormal);
        if (aExp == 0) {
            if (aSig)
                float_raise(status, float_flag_denormal);
            ++expDiff;
        } else
            aSig |= 0x20000000;
        aSig = shift32RightJamming(aSig, -expDiff);
        zExp = bExp;
    } else {
        if (aExp == 0xFF) {
            if (aSig | bSig)
                return propagateFloat32NaN_two_args(a, b, status);
            return a;
        }
        if (aExp == 0) {
            zSig = (aSig + bSig) >> 6;
            if (aSig | bSig) {
                float_raise(status, float_flag_denormal);
                if (get_flush_underflow_to_zero(status) && (extractFloat32Frac(zSig) == zSig)) {
                    float_raise(status, float_flag_underflow | float_flag_inexact);
                    return packFloat32(zSign, 0, 0);
                }
                if (!float_exception_masked(status, float_flag_underflow)) {
                    if (extractFloat32Frac(zSig) == zSig)
                        float_raise(status, float_flag_underflow);
                }
            }
            return packFloat32(zSign, 0, zSig);
        }
        zSig = 0x40000000 + aSig + bSig;
        return roundAndPackFloat32(zSign, aExp, zSig, status);
    }
    aSig |= 0x20000000;
    zSig = (aSig + bSig) << 1;
    --zExp;
    if ((int32_t)zSig < 0) {
        zSig = aSig + bSig;
        ++zExp;
    }
    return roundAndPackFloat32(zSign, zExp, zSig, status);
}
float32 CpuInternal::subFloat32Sigs(float32 a, float32 b, int zSign, float_status_t *status)
{
    int16_t  aExp, bExp, zExp;
    DWORD aSig, bSig, zSig;
    int16_t  expDiff;
    aSig = extractFloat32Frac(a);
    aExp = extractFloat32Exp(a);
    bSig = extractFloat32Frac(b);
    bExp = extractFloat32Exp(b);
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    expDiff = aExp - bExp;
    aSig <<= 7;
    bSig <<= 7;
    if (0 < expDiff)
        goto aExpBigger;
    if (expDiff < 0)
        goto bExpBigger;
    if (aExp == 0xFF) {
        if (aSig | bSig)
            return propagateFloat32NaN_two_args(a, b, status);
        float_raise(status, float_flag_invalid);
        return float32_default_nan;
    }
    if (aExp == 0) {
        if (aSig | bSig)
            float_raise(status, float_flag_denormal);
        aExp = 1;
        bExp = 1;
    }
    if (bSig < aSig)
        goto aBigger;
    if (aSig < bSig)
        goto bBigger;
    return packFloat32(get_float_rounding_mode(status) == float_round_down, 0, 0);
bExpBigger:
    if (bExp == 0xFF) {
        if (bSig)
            return propagateFloat32NaN_two_args(a, b, status);
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat32(zSign ^ 1, 0xFF, 0);
    }
    if ((bExp == 0) && bSig)
        float_raise(status, float_flag_denormal);
    if (aExp == 0) {
        if (aSig)
            float_raise(status, float_flag_denormal);
        ++expDiff;
    } else
        aSig |= 0x40000000;
    aSig = shift32RightJamming(aSig, -expDiff);
    bSig |= 0x40000000;
bBigger:
    zSig = bSig - aSig;
    zExp = bExp;
    zSign ^= 1;
    goto normalizeRoundAndPack;
aExpBigger:
    if (aExp == 0xFF) {
        if (aSig)
            return propagateFloat32NaN_two_args(a, b, status);
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return a;
    }
    if ((aExp == 0) && aSig)
        float_raise(status, float_flag_denormal);
    if (bExp == 0) {
        if (bSig)
            float_raise(status, float_flag_denormal);
        --expDiff;
    } else
        bSig |= 0x40000000;
    bSig = shift32RightJamming(bSig, expDiff);
    aSig |= 0x40000000;
aBigger:
    zSig = aSig - bSig;
    zExp = aExp;
normalizeRoundAndPack:
    --zExp;
    return normalizeRoundAndPackFloat32(zSign, zExp, zSig, status);
}
float32 CpuInternal::float32_add(float32 a, float32 b, float_status_t *status)
{
    int aSign = extractFloat32Sign(a);
    int bSign = extractFloat32Sign(b);
    if (aSign == bSign) {
        return addFloat32Sigs(a, b, aSign, status);
    } else {
        return subFloat32Sigs(a, b, aSign, status);
    }
}
float32 CpuInternal::float32_sub(float32 a, float32 b, float_status_t *status)
{
    int aSign = extractFloat32Sign(a);
    int bSign = extractFloat32Sign(b);
    if (aSign == bSign) {
        return subFloat32Sigs(a, b, aSign, status);
    } else {
        return addFloat32Sigs(a, b, aSign, status);
    }
}
float32 CpuInternal::float32_mul(float32 a, float32 b, float_status_t *status)
{
    int      aSign, bSign, zSign;
    int16_t  aExp, bExp, zExp;
    DWORD aSig, bSig;
    LARGE zSig64;
    DWORD zSig;
    aSig  = extractFloat32Frac(a);
    aExp  = extractFloat32Exp(a);
    aSign = extractFloat32Sign(a);
    bSig  = extractFloat32Frac(b);
    bExp  = extractFloat32Exp(b);
    bSign = extractFloat32Sign(b);
    zSign = aSign ^ bSign;
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    if (aExp == 0xFF) {
        if (aSig || ((bExp == 0xFF) && bSig))
            return propagateFloat32NaN_two_args(a, b, status);
        if ((bExp | bSig) == 0) {
            float_raise(status, float_flag_invalid);
            return float32_default_nan;
        }
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat32(zSign, 0xFF, 0);
    }
    if (bExp == 0xFF) {
        if (bSig)
            return propagateFloat32NaN_two_args(a, b, status);
        if ((aExp | aSig) == 0) {
            float_raise(status, float_flag_invalid);
            return float32_default_nan;
        }
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat32(zSign, 0xFF, 0);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if (bSig && (bExp == 0))
                float_raise(status, float_flag_denormal);
            return packFloat32(zSign, 0, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        if (bSig == 0)
            return packFloat32(zSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(bSig, &bExp, &bSig);
    }
    zExp   = aExp + bExp - 0x7F;
    aSig   = (aSig | 0x00800000) << 7;
    bSig   = (bSig | 0x00800000) << 8;
    zSig64 = shift64RightJamming(((LARGE)aSig) * bSig, 32);
    zSig   = (DWORD)zSig64;
    if (0 <= (int32_t)(zSig << 1)) {
        zSig <<= 1;
        --zExp;
    }
    return roundAndPackFloat32(zSign, zExp, zSig, status);
}
float32 CpuInternal::float32_div(float32 a, float32 b, float_status_t *status)
{
    int      aSign, bSign, zSign;
    int16_t  aExp, bExp, zExp;
    DWORD aSig, bSig, zSig;
    aSig  = extractFloat32Frac(a);
    aExp  = extractFloat32Exp(a);
    aSign = extractFloat32Sign(a);
    bSig  = extractFloat32Frac(b);
    bExp  = extractFloat32Exp(b);
    bSign = extractFloat32Sign(b);
    zSign = aSign ^ bSign;
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    if (aExp == 0xFF) {
        if (aSig)
            return propagateFloat32NaN_two_args(a, b, status);
        if (bExp == 0xFF) {
            if (bSig)
                return propagateFloat32NaN_two_args(a, b, status);
            float_raise(status, float_flag_invalid);
            return float32_default_nan;
        }
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat32(zSign, 0xFF, 0);
    }
    if (bExp == 0xFF) {
        if (bSig)
            return propagateFloat32NaN_two_args(a, b, status);
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat32(zSign, 0, 0);
    }
    if (bExp == 0) {
        if (bSig == 0) {
            if ((aExp | aSig) == 0) {
                float_raise(status, float_flag_invalid);
                return float32_default_nan;
            }
            float_raise(status, float_flag_divbyzero);
            return packFloat32(zSign, 0xFF, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(bSig, &bExp, &bSig);
    }
    if (aExp == 0) {
        if (aSig == 0)
            return packFloat32(zSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(aSig, &aExp, &aSig);
    }
    zExp = aExp - bExp + 0x7D;
    aSig = (aSig | 0x00800000) << 7;
    bSig = (bSig | 0x00800000) << 8;
    if (bSig <= (aSig + aSig)) {
        aSig >>= 1;
        ++zExp;
    }
    zSig = (((LARGE)aSig) << 32) / bSig;
    if ((zSig & 0x3F) == 0) {
        zSig |= ((LARGE)bSig * zSig != ((LARGE)aSig) << 32);
    }
    return roundAndPackFloat32(zSign, zExp, zSig, status);
}
float32 CpuInternal::float32_sqrt(float32 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp, zExp;
    DWORD aSig, zSig;
    LARGE rem, term;
    aSig  = extractFloat32Frac(a);
    aExp  = extractFloat32Exp(a);
    aSign = extractFloat32Sign(a);
    if (aExp == 0xFF) {
        if (aSig)
            return propagateFloat32NaN(a, status);
        if (!aSign)
            return a;
        float_raise(status, float_flag_invalid);
        return float32_default_nan;
    }
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
    }
    if (aSign) {
        if ((aExp | aSig) == 0)
            return packFloat32(aSign, 0, 0);
        float_raise(status, float_flag_invalid);
        return float32_default_nan;
    }
    if (aExp == 0) {
        if (aSig == 0)
            return 0;
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(aSig, &aExp, &aSig);
    }
    zExp = ((aExp - 0x7F) >> 1) + 0x7E;
    aSig = (aSig | 0x00800000) << 8;
    zSig = estimateSqrt32(aExp, aSig) + 2;
    if ((zSig & 0x7F) <= 5) {
        if (zSig < 2) {
            zSig = 0x7FFFFFFF;
            goto roundAndPack;
        }
        aSig >>= aExp & 1;
        term = ((LARGE)zSig) * zSig;
        rem  = (((LARGE)aSig) << 32) - term;
        while ((int64_t)rem < 0) {
            --zSig;
            rem += (((LARGE)zSig) << 1) | 1;
        }
        zSig |= (rem != 0);
    }
    zSig = shift32RightJamming(zSig, 1);
roundAndPack:
    return roundAndPackFloat32(0, zExp, zSig, status);
}
float_class_t CpuInternal::float32_class(float32 a)
{
    int16_t  aExp  = extractFloat32Exp(a);
    DWORD aSig  = extractFloat32Frac(a);
    int      aSign = extractFloat32Sign(a);
    if (aExp == 0xFF) {
        if (aSig == 0)
            return (aSign) ? float_negative_inf : float_positive_inf;
        return (aSig & 0x00400000) ? float_QNaN : float_SNaN;
    }
    if (aExp == 0) {
        if (aSig == 0)
            return float_zero;
        return float_denormal;
    }
    return float_normalized;
}
int CpuInternal::float32_compare_internal(float32 a, float32 b, int quiet, float_status_t *status)
{
    if (get_denormals_are_zeros(status)) {
        a = float32_denormal_to_zero(a);
        b = float32_denormal_to_zero(b);
    }
    float_class_t aClass = float32_class(a);
    float_class_t bClass = float32_class(b);
    if (aClass == float_SNaN || bClass == float_SNaN) {
        float_raise(status, float_flag_invalid);
        return float_relation_unordered;
    }
    if (aClass == float_QNaN || bClass == float_QNaN) {
        if (!quiet)
            float_raise(status, float_flag_invalid);
        return float_relation_unordered;
    }
    if (aClass == float_denormal || bClass == float_denormal) {
        float_raise(status, float_flag_denormal);
    }
    if ((a == b) || ((DWORD)((a | b) << 1) == 0))
        return float_relation_equal;
    int aSign = extractFloat32Sign(a);
    int bSign = extractFloat32Sign(b);
    if (aSign != bSign)
        return (aSign) ? float_relation_less : float_relation_greater;
    if (aSign ^ (a < b))
        return float_relation_less;
    return float_relation_greater;
}
float32 CpuInternal::float32_min(float32 a, float32 b, float_status_t *status)
{
    if (get_denormals_are_zeros(status)) {
        a = float32_denormal_to_zero(a);
        b = float32_denormal_to_zero(b);
    }
    return (float32_compare(a, b, status) == float_relation_less) ? a : b;
}
float32 CpuInternal::float32_max(float32 a, float32 b, float_status_t *status)
{
    if (get_denormals_are_zeros(status)) {
        a = float32_denormal_to_zero(a);
        b = float32_denormal_to_zero(b);
    }
    return (float32_compare(a, b, status) == float_relation_greater) ? a : b;
}
float32 CpuInternal::float32_minmax(float32 a, float32 b, int is_max, int is_abs, float_status_t *status)
{
    if (get_denormals_are_zeros(status)) {
        a = float32_denormal_to_zero(a);
        b = float32_denormal_to_zero(b);
    }
    if (float32_is_nan(a) || float32_is_nan(b)) {
        if (float32_is_signaling_nan(a)) {
            return propagateFloat32NaN(a, status);
        }
        if (float32_is_signaling_nan(b)) {
            return propagateFloat32NaN(b, status);
        }
        if (!float32_is_nan(b)) {
            if (float32_is_denormal(b))
                float_raise(status, float_flag_denormal);
            return b;
        }
        if (!float32_is_nan(a)) {
            if (float32_is_denormal(a))
                float_raise(status, float_flag_denormal);
            return a;
        }
        return propagateFloat32NaN_two_args(a, b, status);
    }
    float32 tmp_a = a, tmp_b = b;
    if (is_abs) {
        tmp_a &= ~0x80000000;
        tmp_b &= ~0x80000000;
    }
    int aSign = extractFloat32Sign(tmp_a);
    int bSign = extractFloat32Sign(tmp_b);
    if (float32_is_denormal(a) || float32_is_denormal(b))
        float_raise(status, float_flag_denormal);
    if (aSign != bSign) {
        if (!is_max) {
            return aSign ? a : b;
        } else {
            return aSign ? b : a;
        }
    } else {
        if (!is_max) {
            return (aSign ^ (tmp_a < tmp_b)) ? a : b;
        } else {
            return (aSign ^ (tmp_a < tmp_b)) ? b : a;
        }
    }
}
int32_t CpuInternal::float64_to_int32(float64 a, float_status_t *status)
{
    LARGE aSig  = extractFloat64Frac(a);
    int16_t  aExp  = extractFloat64Exp(a);
    int      aSign = extractFloat64Sign(a);
    if ((aExp == 0x7FF) && aSig)
        aSign = 0;
    if (aExp)
        aSig |= 0x0010000000000000ULL;
    else {
        if (get_denormals_are_zeros(status))
            aSig = 0;
    }
    int shiftCount = 0x42C - aExp;
    if (0 < shiftCount)
        aSig = shift64RightJamming(aSig, shiftCount);
    return roundAndPackInt32(aSign, aSig, status);
}
int32_t CpuInternal::float64_to_int32_round_to_zero(float64 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    LARGE aSig, savedASig;
    int32_t  z;
    int      shiftCount;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    if (0x41E < aExp) {
        float_raise(status, float_flag_invalid);
        return (int32_t)(((int32_t)0x80000000));
    } else if (aExp < 0x3FF) {
        if (get_denormals_are_zeros(status) && aExp == 0)
            aSig = 0;
        if (aExp || aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    }
    aSig |= 0x0010000000000000ULL;
    shiftCount = 0x433 - aExp;
    savedASig  = aSig;
    aSig >>= shiftCount;
    z = (int32_t)aSig;
    if (aSign)
        z = -z;
    if ((z < 0) ^ aSign) {
        float_raise(status, float_flag_invalid);
        return (int32_t)(((int32_t)0x80000000));
    }
    if ((aSig << shiftCount) != savedASig) {
        float_raise(status, float_flag_inexact);
    }
    return z;
}
DWORD CpuInternal::float64_to_uint32_round_to_zero(float64 a, float_status_t *status)
{
    LARGE aSig  = extractFloat64Frac(a);
    int16_t  aExp  = extractFloat64Exp(a);
    int      aSign = extractFloat64Sign(a);
    if (aExp < 0x3FF) {
        if (get_denormals_are_zeros(status) && aExp == 0)
            aSig = 0;
        if (aExp || aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    }
    if (0x41E < aExp || aSign) {
        float_raise(status, float_flag_invalid);
        return (0xffffffff);
    }
    aSig |= 0x0010000000000000ULL;
    int      shiftCount = 0x433 - aExp;
    LARGE savedASig  = aSig;
    aSig >>= shiftCount;
    if ((aSig << shiftCount) != savedASig) {
        float_raise(status, float_flag_inexact);
    }
    return (DWORD)aSig;
}
int64_t CpuInternal::float64_to_int64(float64 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    LARGE aSig, aSigExtra;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    if (aExp)
        aSig |= 0x0010000000000000ULL;
    else {
        if (get_denormals_are_zeros(status))
            aSig = 0;
    }
    int shiftCount = 0x433 - aExp;
    if (shiftCount <= 0) {
        if (0x43E < aExp) {
            float_raise(status, float_flag_invalid);
            return (int64_t)(0x8000000000000000ULL);
        }
        aSigExtra = 0;
        aSig <<= -shiftCount;
    } else {
        shift64ExtraRightJamming(aSig, 0, shiftCount, &aSig, &aSigExtra);
    }
    return roundAndPackInt64(aSign, aSig, aSigExtra, status);
}
int64_t CpuInternal::float64_to_int64_round_to_zero(float64 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    LARGE aSig;
    int64_t  z;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    if (aExp)
        aSig |= 0x0010000000000000ULL;
    int shiftCount = aExp - 0x433;
    if (0 <= shiftCount) {
        if (0x43E <= aExp) {
            if (a != 0xC3E0000000000000ULL) {
                float_raise(status, float_flag_invalid);
            }
            return (int64_t)(0x8000000000000000ULL);
        }
        z = aSig << shiftCount;
    } else {
        if (aExp < 0x3FE) {
            if (get_denormals_are_zeros(status) && aExp == 0)
                aSig = 0;
            if (aExp | aSig)
                float_raise(status, float_flag_inexact);
            return 0;
        }
        z = aSig >> (-shiftCount);
        if ((LARGE)(aSig << (shiftCount & 63))) {
            float_raise(status, float_flag_inexact);
        }
    }
    if (aSign)
        z = -z;
    return z;
}
LARGE CpuInternal::float64_to_uint64_round_to_zero(float64 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    LARGE aSig, z;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    if (aExp < 0x3FE) {
        if (get_denormals_are_zeros(status) && aExp == 0)
            aSig = 0;
        if (aExp | aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    }
    if (0x43E <= aExp || aSign) {
        float_raise(status, float_flag_invalid);
        return 0xffffffffffffffffULL;
    }
    if (aExp)
        aSig |= 0x0010000000000000ULL;
    int shiftCount = aExp - 0x433;
    if (0 <= shiftCount) {
        z = aSig << shiftCount;
    } else {
        z = aSig >> (-shiftCount);
        if ((LARGE)(aSig << (shiftCount & 63))) {
            float_raise(status, float_flag_inexact);
        }
    }
    return z;
}
DWORD CpuInternal::float64_to_uint32(float64 a, float_status_t *status)
{
    LARGE val_64 = float64_to_uint64(a, status);
    if (val_64 > 0xffffffff) {
        status->float_exception_flags = float_flag_invalid;
        return (0xffffffff);
    }
    return (DWORD)val_64;
}
LARGE CpuInternal::float64_to_uint64(float64 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp, shiftCount;
    LARGE aSig, aSigExtra;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
    }
    if (aSign && (aExp > 0x3FE)) {
        float_raise(status, float_flag_invalid);
        return 0xffffffffffffffffULL;
    }
    if (aExp) {
        aSig |= 0x0010000000000000ULL;
    }
    shiftCount = 0x433 - aExp;
    if (shiftCount <= 0) {
        if (0x43E < aExp) {
            float_raise(status, float_flag_invalid);
            return 0xffffffffffffffffULL;
        }
        aSigExtra = 0;
        aSig <<= -shiftCount;
    } else {
        shift64ExtraRightJamming(aSig, 0, shiftCount, &aSig, &aSigExtra);
    }
    return roundAndPackUint64(aSign, aSig, aSigExtra, status);
}
float32 CpuInternal::float64_to_float32(float64 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp;
    LARGE aSig;
    DWORD zSig;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    if (aExp == 0x7FF) {
        if (aSig)
            return CpuInternal::commonNaNToFloat32(float64ToCommonNaN(a, status));
        return packFloat32(aSign, 0xFF, 0);
    }
    if (aExp == 0) {
        if (aSig == 0 || get_denormals_are_zeros(status))
            return packFloat32(aSign, 0, 0);
        float_raise(status, float_flag_denormal);
    }
    aSig = shift64RightJamming(aSig, 22);
    zSig = (DWORD)aSig;
    if (aExp || zSig) {
        zSig |= 0x40000000;
        aExp -= 0x381;
    }
    return roundAndPackFloat32(aSign, aExp, zSig, status);
}
float64 CpuInternal::float64_round_to_int_with_scale(float64 a, BYTE scale, float_status_t *status)
{
    LARGE lastBitMask, roundBitsMask;
    int      roundingMode = get_float_rounding_mode(status);
    int16_t  aExp         = extractFloat64Exp(a);
    scale &= 0xf;
    if ((aExp == 0x7FF) && extractFloat64Frac(a)) {
        return propagateFloat64NaN(a, status);
    }
    aExp += scale;
    if (0x433 <= aExp) {
        return a;
    }
    if (get_denormals_are_zeros(status)) {
        a = float64_denormal_to_zero(a);
    }
    if (aExp < 0x3FF) {
        if ((LARGE)(a << 1) == 0)
            return a;
        float_raise(status, float_flag_inexact);
        int aSign = extractFloat64Sign(a);
        switch (roundingMode) {
            case float_round_nearest_even:
                if ((aExp == 0x3FE) && extractFloat64Frac(a)) {
                    return packFloat64(aSign, 0x3FF - scale, 0);
                }
                break;
            case float_round_down:
                return aSign ? packFloat64(1, 0x3FF - scale, 0) : float64_positive_zero;
            case float_round_up:
                return aSign ? float64_negative_zero : packFloat64(0, 0x3FF - scale, 0);
        }
        return packFloat64(aSign, 0, 0);
    }
    lastBitMask = 1;
    lastBitMask <<= 0x433 - aExp;
    roundBitsMask = lastBitMask - 1;
    float64 z     = a;
    if (roundingMode == float_round_nearest_even) {
        z += lastBitMask >> 1;
        if ((z & roundBitsMask) == 0)
            z &= ~lastBitMask;
    } else if (roundingMode != float_round_to_zero) {
        if (extractFloat64Sign(z) ^ (roundingMode == float_round_up)) {
            z += roundBitsMask;
        }
    }
    z &= ~roundBitsMask;
    if (z != a)
        float_raise(status, float_flag_inexact);
    return z;
}
float64 CpuInternal::float64_frc(float64 a, float_status_t *status)
{
    int      roundingMode = get_float_rounding_mode(status);
    LARGE aSig         = extractFloat64Frac(a);
    int16_t  aExp         = extractFloat64Exp(a);
    int      aSign        = extractFloat64Sign(a);
    if (aExp == 0x7FF) {
        if (aSig)
            return propagateFloat64NaN(a, status);
        float_raise(status, float_flag_invalid);
        return float64_default_nan;
    }
    if (aExp >= 0x433) {
        return packFloat64(roundingMode == float_round_down, 0, 0);
    }
    if (aExp < 0x3FF) {
        if (aExp == 0) {
            if (aSig == 0 || get_denormals_are_zeros(status))
                return packFloat64(roundingMode == float_round_down, 0, 0);
            float_raise(status, float_flag_denormal);
            if (!float_exception_masked(status, float_flag_underflow))
                float_raise(status, float_flag_underflow);
            if (get_flush_underflow_to_zero(status)) {
                float_raise(status, float_flag_underflow | float_flag_inexact);
                return packFloat64(aSign, 0, 0);
            }
        }
        return a;
    }
    LARGE lastBitMask   = 1ULL << (0x433 - aExp);
    LARGE roundBitsMask = lastBitMask - 1;
    aSig &= roundBitsMask;
    aSig <<= 10;
    aExp--;
    if (aSig == 0)
        return packFloat64(roundingMode == float_round_down, 0, 0);
    return normalizeRoundAndPackFloat64(aSign, aExp, aSig, status);
}
float64 CpuInternal::float64_getexp(float64 a, float_status_t *status)
{
    int16_t  aExp = extractFloat64Exp(a);
    LARGE aSig = extractFloat64Frac(a);
    if (aExp == 0x7FF) {
        if (aSig)
            return propagateFloat64NaN(a, status);
        return float64_positive_inf;
    }
    if (aExp == 0) {
        if (aSig == 0 || get_denormals_are_zeros(status))
            return float64_negative_inf;
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(aSig, &aExp, &aSig);
    }
    return int32_to_float64(aExp - 0x3FF);
}
float64 CpuInternal::float64_getmant(float64 a, float_status_t *status, int sign_ctrl, int interv)
{
    int16_t  aExp  = extractFloat64Exp(a);
    LARGE aSig  = extractFloat64Frac(a);
    int      aSign = extractFloat64Sign(a);
    if (aExp == 0x7FF) {
        if (aSig)
            return propagateFloat64NaN(a, status);
        if (aSign) {
            if (sign_ctrl & 0x2) {
                float_raise(status, float_flag_invalid);
                return float64_default_nan;
            }
        }
        return packFloat64(~sign_ctrl & aSign, 0x3FF, 0);
    }
    if (aExp == 0 && (aSig == 0 || get_denormals_are_zeros(status))) {
        return packFloat64(~sign_ctrl & aSign, 0x3FF, 0);
    }
    if (aSign) {
        if (sign_ctrl & 0x2) {
            float_raise(status, float_flag_invalid);
            return float64_default_nan;
        }
    }
    if (aExp == 0) {
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(aSig, &aExp, &aSig);
        aSig &= 0xFFFFFFFFFFFFFFFFULL;
    }
    switch (interv) {
        case 0x0:
            aExp = 0x3FF;
            break;
        case 0x1:
            aExp -= 0x3FF;
            aExp = 0x3FF - (aExp & 0x1);
            break;
        case 0x2:
            aExp = 0x3FE;
            break;
        case 0x3:
            aExp = 0x3FF - ((aSig >> 51) & 0x1);
            break;
    }
    return packFloat64(~sign_ctrl & aSign, aExp, aSig);
}
float64 CpuInternal::float64_scalef(float64 a, float64 b, float_status_t *status)
{
    LARGE aSig  = extractFloat64Frac(a);
    int16_t  aExp  = extractFloat64Exp(a);
    int      aSign = extractFloat64Sign(a);
    LARGE bSig  = extractFloat64Frac(b);
    int16_t  bExp  = extractFloat64Exp(b);
    int      bSign = extractFloat64Sign(b);
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    if (bExp == 0x7FF) {
        if (bSig)
            return propagateFloat64NaN_two_args(a, b, status);
    }
    if (aExp == 0x7FF) {
        if (aSig) {
            int aIsSignalingNaN = (aSig & 0x0008000000000000ULL) == 0;
            if (aIsSignalingNaN || bExp != 0x7FF || bSig)
                return propagateFloat64NaN_two_args(a, b, status);
            return bSign ? 0 : float64_positive_inf;
        }
        if (bExp == 0x7FF && bSign) {
            float_raise(status, float_flag_invalid);
            return float64_default_nan;
        }
        return a;
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if (bExp == 0x7FF && !bSign) {
                float_raise(status, float_flag_invalid);
                return float64_default_nan;
            }
            return a;
        }
        float_raise(status, float_flag_denormal);
    }
    if ((bExp | bSig) == 0)
        return a;
    if (bExp == 0x7FF) {
        if (bSign)
            return packFloat64(aSign, 0, 0);
        return packFloat64(aSign, 0x7FF, 0);
    }
    if (0x40F <= bExp) {
        return roundAndPackFloat64(aSign, bSign ? -0x3FF : 0x7FF, aSig, status);
    }
    int scale = 0;
    if (bExp < 0x3FF) {
        if (bExp == 0)
            float_raise(status, float_flag_denormal);
        scale = -bSign;
    } else {
        bSig |= 0x0010000000000000ULL;
        int      shiftCount = 0x433 - bExp;
        LARGE savedBSig  = bSig;
        bSig >>= shiftCount;
        scale = (int32_t)bSig;
        if (bSign) {
            if ((bSig << shiftCount) != savedBSig)
                scale++;
            scale = -scale;
        }
        if (scale > 0x1000)
            scale = 0x1000;
        if (scale < -0x1000)
            scale = -0x1000;
    }
    if (aExp != 0) {
        aSig |= 0x0010000000000000ULL;
    } else {
        aExp++;
    }
    aExp += scale - 1;
    aSig <<= 10;
    return normalizeRoundAndPackFloat64(aSign, aExp, aSig, status);
}
float64 CpuInternal::addFloat64Sigs(float64 a, float64 b, int zSign, float_status_t *status)
{
    int16_t  aExp, bExp, zExp;
    LARGE aSig, bSig, zSig;
    int16_t  expDiff;
    aSig = extractFloat64Frac(a);
    aExp = extractFloat64Exp(a);
    bSig = extractFloat64Frac(b);
    bExp = extractFloat64Exp(b);
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    expDiff = aExp - bExp;
    aSig <<= 9;
    bSig <<= 9;
    if (0 < expDiff) {
        if (aExp == 0x7FF) {
            if (aSig)
                return propagateFloat64NaN_two_args(a, b, status);
            if (bSig && (bExp == 0))
                float_raise(status, float_flag_denormal);
            return a;
        }
        if ((aExp == 0) && aSig)
            float_raise(status, float_flag_denormal);
        if (bExp == 0) {
            if (bSig)
                float_raise(status, float_flag_denormal);
            --expDiff;
        } else
            bSig |= 0x2000000000000000ULL;
        bSig = shift64RightJamming(bSig, expDiff);
        zExp = aExp;
    } else if (expDiff < 0) {
        if (bExp == 0x7FF) {
            if (bSig)
                return propagateFloat64NaN_two_args(a, b, status);
            if (aSig && (aExp == 0))
                float_raise(status, float_flag_denormal);
            return packFloat64(zSign, 0x7FF, 0);
        }
        if ((bExp == 0) && bSig)
            float_raise(status, float_flag_denormal);
        if (aExp == 0) {
            if (aSig)
                float_raise(status, float_flag_denormal);
            ++expDiff;
        } else
            aSig |= 0x2000000000000000ULL;
        aSig = shift64RightJamming(aSig, -expDiff);
        zExp = bExp;
    } else {
        if (aExp == 0x7FF) {
            if (aSig | bSig)
                return propagateFloat64NaN_two_args(a, b, status);
            return a;
        }
        if (aExp == 0) {
            zSig = (aSig + bSig) >> 9;
            if (aSig | bSig) {
                float_raise(status, float_flag_denormal);
                if (get_flush_underflow_to_zero(status) && (extractFloat64Frac(zSig) == zSig)) {
                    float_raise(status, float_flag_underflow | float_flag_inexact);
                    return packFloat64(zSign, 0, 0);
                }
                if (!float_exception_masked(status, float_flag_underflow)) {
                    if (extractFloat64Frac(zSig) == zSig)
                        float_raise(status, float_flag_underflow);
                }
            }
            return packFloat64(zSign, 0, zSig);
        }
        zSig = 0x4000000000000000ULL + aSig + bSig;
        return roundAndPackFloat64(zSign, aExp, zSig, status);
    }
    aSig |= 0x2000000000000000ULL;
    zSig = (aSig + bSig) << 1;
    --zExp;
    if ((int64_t)zSig < 0) {
        zSig = aSig + bSig;
        ++zExp;
    }
    return roundAndPackFloat64(zSign, zExp, zSig, status);
}
float64 CpuInternal::subFloat64Sigs(float64 a, float64 b, int zSign, float_status_t *status)
{
    int16_t  aExp, bExp, zExp;
    LARGE aSig, bSig, zSig;
    int16_t  expDiff;
    aSig = extractFloat64Frac(a);
    aExp = extractFloat64Exp(a);
    bSig = extractFloat64Frac(b);
    bExp = extractFloat64Exp(b);
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    expDiff = aExp - bExp;
    aSig <<= 10;
    bSig <<= 10;
    if (0 < expDiff)
        goto aExpBigger;
    if (expDiff < 0)
        goto bExpBigger;
    if (aExp == 0x7FF) {
        if (aSig | bSig)
            return propagateFloat64NaN_two_args(a, b, status);
        float_raise(status, float_flag_invalid);
        return float64_default_nan;
    }
    if (aExp == 0) {
        if (aSig | bSig)
            float_raise(status, float_flag_denormal);
        aExp = 1;
        bExp = 1;
    }
    if (bSig < aSig)
        goto aBigger;
    if (aSig < bSig)
        goto bBigger;
    return packFloat64(get_float_rounding_mode(status) == float_round_down, 0, 0);
bExpBigger:
    if (bExp == 0x7FF) {
        if (bSig)
            return propagateFloat64NaN_two_args(a, b, status);
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat64(zSign ^ 1, 0x7FF, 0);
    }
    if ((bExp == 0) && bSig)
        float_raise(status, float_flag_denormal);
    if (aExp == 0) {
        if (aSig)
            float_raise(status, float_flag_denormal);
        ++expDiff;
    } else
        aSig |= 0x4000000000000000ULL;
    aSig = shift64RightJamming(aSig, -expDiff);
    bSig |= 0x4000000000000000ULL;
bBigger:
    zSig = bSig - aSig;
    zExp = bExp;
    zSign ^= 1;
    goto normalizeRoundAndPack;
aExpBigger:
    if (aExp == 0x7FF) {
        if (aSig)
            return propagateFloat64NaN_two_args(a, b, status);
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return a;
    }
    if ((aExp == 0) && aSig)
        float_raise(status, float_flag_denormal);
    if (bExp == 0) {
        if (bSig)
            float_raise(status, float_flag_denormal);
        --expDiff;
    } else
        bSig |= 0x4000000000000000ULL;
    bSig = shift64RightJamming(bSig, expDiff);
    aSig |= 0x4000000000000000ULL;
aBigger:
    zSig = aSig - bSig;
    zExp = aExp;
normalizeRoundAndPack:
    --zExp;
    return normalizeRoundAndPackFloat64(zSign, zExp, zSig, status);
}
float64 CpuInternal::float64_add(float64 a, float64 b, float_status_t *status)
{
    int aSign = extractFloat64Sign(a);
    int bSign = extractFloat64Sign(b);
    if (aSign == bSign) {
        return addFloat64Sigs(a, b, aSign, status);
    } else {
        return subFloat64Sigs(a, b, aSign, status);
    }
}
float64 CpuInternal::float64_sub(float64 a, float64 b, float_status_t *status)
{
    int aSign = extractFloat64Sign(a);
    int bSign = extractFloat64Sign(b);
    if (aSign == bSign) {
        return subFloat64Sigs(a, b, aSign, status);
    } else {
        return addFloat64Sigs(a, b, aSign, status);
    }
}
float64 CpuInternal::float64_mul(float64 a, float64 b, float_status_t *status)
{
    int      aSign, bSign, zSign;
    int16_t  aExp, bExp, zExp;
    LARGE aSig, bSig, zSig0, zSig1;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    bSig  = extractFloat64Frac(b);
    bExp  = extractFloat64Exp(b);
    bSign = extractFloat64Sign(b);
    zSign = aSign ^ bSign;
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    if (aExp == 0x7FF) {
        if (aSig || ((bExp == 0x7FF) && bSig)) {
            return propagateFloat64NaN_two_args(a, b, status);
        }
        if ((bExp | bSig) == 0) {
            float_raise(status, float_flag_invalid);
            return float64_default_nan;
        }
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat64(zSign, 0x7FF, 0);
    }
    if (bExp == 0x7FF) {
        if (bSig)
            return propagateFloat64NaN_two_args(a, b, status);
        if ((aExp | aSig) == 0) {
            float_raise(status, float_flag_invalid);
            return float64_default_nan;
        }
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat64(zSign, 0x7FF, 0);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if (bSig && (bExp == 0))
                float_raise(status, float_flag_denormal);
            return packFloat64(zSign, 0, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        if (bSig == 0)
            return packFloat64(zSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(bSig, &bExp, &bSig);
    }
    zExp = aExp + bExp - 0x3FF;
    aSig = (aSig | 0x0010000000000000ULL) << 10;
    bSig = (bSig | 0x0010000000000000ULL) << 11;
    mul64To128(aSig, bSig, &zSig0, &zSig1);
    zSig0 |= (zSig1 != 0);
    if (0 <= (int64_t)(zSig0 << 1)) {
        zSig0 <<= 1;
        --zExp;
    }
    return roundAndPackFloat64(zSign, zExp, zSig0, status);
}
float64 CpuInternal::float64_div(float64 a, float64 b, float_status_t *status)
{
    int      aSign, bSign, zSign;
    int16_t  aExp, bExp, zExp;
    LARGE aSig, bSig, zSig;
    LARGE rem0, rem1;
    LARGE term0, term1;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    bSig  = extractFloat64Frac(b);
    bExp  = extractFloat64Exp(b);
    bSign = extractFloat64Sign(b);
    zSign = aSign ^ bSign;
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
    }
    if (aExp == 0x7FF) {
        if (aSig)
            return propagateFloat64NaN_two_args(a, b, status);
        if (bExp == 0x7FF) {
            if (bSig)
                return propagateFloat64NaN_two_args(a, b, status);
            float_raise(status, float_flag_invalid);
            return float64_default_nan;
        }
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat64(zSign, 0x7FF, 0);
    }
    if (bExp == 0x7FF) {
        if (bSig)
            return propagateFloat64NaN_two_args(a, b, status);
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloat64(zSign, 0, 0);
    }
    if (bExp == 0) {
        if (bSig == 0) {
            if ((aExp | aSig) == 0) {
                float_raise(status, float_flag_invalid);
                return float64_default_nan;
            }
            float_raise(status, float_flag_divbyzero);
            return packFloat64(zSign, 0x7FF, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(bSig, &bExp, &bSig);
    }
    if (aExp == 0) {
        if (aSig == 0)
            return packFloat64(zSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(aSig, &aExp, &aSig);
    }
    zExp = aExp - bExp + 0x3FD;
    aSig = (aSig | 0x0010000000000000ULL) << 10;
    bSig = (bSig | 0x0010000000000000ULL) << 11;
    if (bSig <= (aSig + aSig)) {
        aSig >>= 1;
        ++zExp;
    }
    zSig = estimateDiv128To64(aSig, 0, bSig);
    if ((zSig & 0x1FF) <= 2) {
        mul64To128(bSig, zSig, &term0, &term1);
        sub128(aSig, 0, term0, term1, &rem0, &rem1);
        while ((int64_t)rem0 < 0) {
            --zSig;
            add128(rem0, rem1, 0, bSig, &rem0, &rem1);
        }
        zSig |= (rem1 != 0);
    }
    return roundAndPackFloat64(zSign, zExp, zSig, status);
}
float64 CpuInternal::float64_sqrt(float64 a, float_status_t *status)
{
    int      aSign;
    int16_t  aExp, zExp;
    LARGE aSig, zSig, doubleZSig;
    LARGE rem0, rem1, term0, term1;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    if (aExp == 0x7FF) {
        if (aSig)
            return propagateFloat64NaN(a, status);
        if (!aSign)
            return a;
        float_raise(status, float_flag_invalid);
        return float64_default_nan;
    }
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
    }
    if (aSign) {
        if ((aExp | aSig) == 0)
            return packFloat64(aSign, 0, 0);
        float_raise(status, float_flag_invalid);
        return float64_default_nan;
    }
    if (aExp == 0) {
        if (aSig == 0)
            return 0;
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(aSig, &aExp, &aSig);
    }
    zExp = ((aExp - 0x3FF) >> 1) + 0x3FE;
    aSig |= 0x0010000000000000ULL;
    zSig = estimateSqrt32(aExp, (DWORD)(aSig >> 21));
    aSig <<= 9 - (aExp & 1);
    zSig = estimateDiv128To64(aSig, 0, zSig << 32) + (zSig << 30);
    if ((zSig & 0x1FF) <= 5) {
        doubleZSig = zSig << 1;
        mul64To128(zSig, zSig, &term0, &term1);
        sub128(aSig, 0, term0, term1, &rem0, &rem1);
        while ((int64_t)rem0 < 0) {
            --zSig;
            doubleZSig -= 2;
            add128(rem0, rem1, zSig >> 63, doubleZSig | 1, &rem0, &rem1);
        }
        zSig |= ((rem0 | rem1) != 0);
    }
    return roundAndPackFloat64(0, zExp, zSig, status);
}
float_class_t CpuInternal::float64_class(float64 a)
{
    int16_t  aExp  = extractFloat64Exp(a);
    LARGE aSig  = extractFloat64Frac(a);
    int      aSign = extractFloat64Sign(a);
    if (aExp == 0x7FF) {
        if (aSig == 0)
            return (aSign) ? float_negative_inf : float_positive_inf;
        return (aSig & 0x0008000000000000ULL) ? float_QNaN : float_SNaN;
    }
    if (aExp == 0) {
        if (aSig == 0)
            return float_zero;
        return float_denormal;
    }
    return float_normalized;
}
int CpuInternal::float64_compare_internal(float64 a, float64 b, int quiet, float_status_t *status)
{
    if (get_denormals_are_zeros(status)) {
        a = float64_denormal_to_zero(a);
        b = float64_denormal_to_zero(b);
    }
    float_class_t aClass = float64_class(a);
    float_class_t bClass = float64_class(b);
    if (aClass == float_SNaN || bClass == float_SNaN) {
        float_raise(status, float_flag_invalid);
        return float_relation_unordered;
    }
    if (aClass == float_QNaN || bClass == float_QNaN) {
        if (!quiet)
            float_raise(status, float_flag_invalid);
        return float_relation_unordered;
    }
    if (aClass == float_denormal || bClass == float_denormal) {
        float_raise(status, float_flag_denormal);
    }
    if ((a == b) || ((LARGE)((a | b) << 1) == 0))
        return float_relation_equal;
    int aSign = extractFloat64Sign(a);
    int bSign = extractFloat64Sign(b);
    if (aSign != bSign)
        return (aSign) ? float_relation_less : float_relation_greater;
    if (aSign ^ (a < b))
        return float_relation_less;
    return float_relation_greater;
}
float64 CpuInternal::float64_min(float64 a, float64 b, float_status_t *status)
{
    if (get_denormals_are_zeros(status)) {
        a = float64_denormal_to_zero(a);
        b = float64_denormal_to_zero(b);
    }
    return (float64_compare(a, b, status) == float_relation_less) ? a : b;
}
float64 CpuInternal::float64_max(float64 a, float64 b, float_status_t *status)
{
    if (get_denormals_are_zeros(status)) {
        a = float64_denormal_to_zero(a);
        b = float64_denormal_to_zero(b);
    }
    return (float64_compare(a, b, status) == float_relation_greater) ? a : b;
}
float64 CpuInternal::float64_minmax(float64 a, float64 b, int is_max, int is_abs, float_status_t *status)
{
    if (get_denormals_are_zeros(status)) {
        a = float64_denormal_to_zero(a);
        b = float64_denormal_to_zero(b);
    }
    if (float64_is_nan(a) || float64_is_nan(b)) {
        if (float64_is_signaling_nan(a)) {
            return propagateFloat64NaN(a, status);
        }
        if (float64_is_signaling_nan(b)) {
            return propagateFloat64NaN(b, status);
        }
        if (!float64_is_nan(b)) {
            if (float64_is_denormal(b))
                float_raise(status, float_flag_denormal);
            return b;
        }
        if (!float64_is_nan(a)) {
            if (float64_is_denormal(a))
                float_raise(status, float_flag_denormal);
            return a;
        }
        return propagateFloat64NaN_two_args(a, b, status);
    }
    float64 tmp_a = a, tmp_b = b;
    if (is_abs) {
        tmp_a &= ~0x8000000000000000ULL;
        tmp_b &= ~0x8000000000000000ULL;
    }
    int aSign = extractFloat64Sign(tmp_a);
    int bSign = extractFloat64Sign(tmp_b);
    if (float64_is_denormal(a) || float64_is_denormal(b))
        float_raise(status, float_flag_denormal);
    if (aSign != bSign) {
        if (!is_max) {
            return aSign ? a : b;
        } else {
            return aSign ? b : a;
        }
    } else {
        if (!is_max) {
            return (aSign ^ (tmp_a < tmp_b)) ? a : b;
        } else {
            return (aSign ^ (tmp_a < tmp_b)) ? b : a;
        }
    }
}
floatx80 CpuInternal::int32_to_floatx80(int32_t a)
{
    if (a == 0)
        return packFloatx80(0, 0, 0);
    int      zSign      = (a < 0);
    DWORD absA       = zSign ? -a : a;
    int      shiftCount = countLeadingZeros32(absA) + 32;
    LARGE zSig       = absA;
    return packFloatx80(zSign, 0x403E - shiftCount, zSig << shiftCount);
}
floatx80 CpuInternal::int64_to_floatx80(int64_t a)
{
    if (a == 0)
        return packFloatx80(0, 0, 0);
    int      zSign      = (a < 0);
    LARGE absA       = zSign ? -a : a;
    int      shiftCount = countLeadingZeros64(absA);
    return packFloatx80(zSign, 0x403E - shiftCount, absA << shiftCount);
}
floatx80 CpuInternal::float32_to_floatx80(float32 a, float_status_t *status)
{
    DWORD aSig  = extractFloat32Frac(a);
    int16_t  aExp  = extractFloat32Exp(a);
    int      aSign = extractFloat32Sign(a);
    if (aExp == 0xFF) {
        if (aSig)
            return CpuInternal::commonNaNToFloatx80(float32ToCommonNaN(a, status));
        return packFloatx80(aSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0)
            return packFloatx80(aSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(aSig, &aExp, &aSig);
    }
    aSig |= 0x00800000;
    return packFloatx80(aSign, aExp + 0x3F80, ((LARGE)aSig) << 40);
}
floatx80 CpuInternal::float64_to_floatx80(float64 a, float_status_t *status)
{
    LARGE aSig  = extractFloat64Frac(a);
    int16_t  aExp  = extractFloat64Exp(a);
    int      aSign = extractFloat64Sign(a);
    if (aExp == 0x7FF) {
        if (aSig)
            return CpuInternal::commonNaNToFloatx80(float64ToCommonNaN(a, status));
        return packFloatx80(aSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0)
            return packFloatx80(aSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(aSig, &aExp, &aSig);
    }
    return packFloatx80(aSign, aExp + 0x3C00, (aSig | 0x0010000000000000ULL) << 11);
}
int32_t CpuInternal::floatx80_to_int32(floatx80 a, float_status_t *status)
{
    LARGE aSig  = extractFloatx80Frac(a);
    int32_t  aExp  = extractFloatx80Exp(a);
    int      aSign = extractFloatx80Sign(a);
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return ((int32_t)0x80000000);
    }
    if ((aExp == 0x7FFF) && (LARGE)(aSig << 1))
        aSign = 0;
    int shiftCount = 0x4037 - aExp;
    if (shiftCount <= 0)
        shiftCount = 1;
    aSig = shift64RightJamming(aSig, shiftCount);
    return roundAndPackInt32(aSign, aSig, status);
}
int32_t CpuInternal::floatx80_to_int32_round_to_zero(floatx80 a, float_status_t *status)
{
    int32_t  aExp;
    LARGE aSig, savedASig;
    int32_t  z;
    int      shiftCount;
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return ((int32_t)0x80000000);
    }
    aSig      = extractFloatx80Frac(a);
    aExp      = extractFloatx80Exp(a);
    int aSign = extractFloatx80Sign(a);
    if (aExp > 0x401E) {
        float_raise(status, float_flag_invalid);
        return (int32_t)(((int32_t)0x80000000));
    }
    if (aExp < 0x3FFF) {
        if (aExp || aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    }
    shiftCount = 0x403E - aExp;
    savedASig  = aSig;
    aSig >>= shiftCount;
    z = (int32_t)aSig;
    if (aSign)
        z = -z;
    if ((z < 0) ^ aSign) {
        float_raise(status, float_flag_invalid);
        return (int32_t)(((int32_t)0x80000000));
    }
    if ((aSig << shiftCount) != savedASig) {
        float_raise(status, float_flag_inexact);
    }
    return z;
}
int64_t CpuInternal::floatx80_to_int64(floatx80 a, float_status_t *status)
{
    int32_t  aExp;
    LARGE aSig, aSigExtra;
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return 0x8000000000000000ULL;
    }
    aSig           = extractFloatx80Frac(a);
    aExp           = extractFloatx80Exp(a);
    int aSign      = extractFloatx80Sign(a);
    int shiftCount = 0x403E - aExp;
    if (shiftCount <= 0) {
        if (shiftCount) {
            float_raise(status, float_flag_invalid);
            return (int64_t)(0x8000000000000000ULL);
        }
        aSigExtra = 0;
    } else {
        shift64ExtraRightJamming(aSig, 0, shiftCount, &aSig, &aSigExtra);
    }
    return roundAndPackInt64(aSign, aSig, aSigExtra, status);
}
int64_t CpuInternal::floatx80_to_int64_round_to_zero(floatx80 a, float_status_t *status)
{
    int      aSign;
    int32_t  aExp;
    LARGE aSig;
    int64_t  z;
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return 0x8000000000000000ULL;
    }
    aSig           = extractFloatx80Frac(a);
    aExp           = extractFloatx80Exp(a);
    aSign          = extractFloatx80Sign(a);
    int shiftCount = aExp - 0x403E;
    if (0 <= shiftCount) {
        aSig &= 0x7FFFFFFFFFFFFFFFULL;
        if ((a.exp != 0xC03E) || aSig) {
            float_raise(status, float_flag_invalid);
        }
        return (int64_t)(0x8000000000000000ULL);
    } else if (aExp < 0x3FFF) {
        if (aExp | aSig)
            float_raise(status, float_flag_inexact);
        return 0;
    }
    z = aSig >> (-shiftCount);
    if ((LARGE)(aSig << (shiftCount & 63))) {
        float_raise(status, float_flag_inexact);
    }
    if (aSign)
        z = -z;
    return z;
}
float32 CpuInternal::floatx80_to_float32(floatx80 a, float_status_t *status)
{
    LARGE aSig  = extractFloatx80Frac(a);
    int32_t  aExp  = extractFloatx80Exp(a);
    int      aSign = extractFloatx80Sign(a);
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return float32_default_nan;
    }
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1))
            return CpuInternal::commonNaNToFloat32(floatx80ToCommonNaN(a, status));
        return packFloat32(aSign, 0xFF, 0);
    }
    aSig = shift64RightJamming(aSig, 33);
    if (aExp || aSig)
        aExp -= 0x3F81;
    return roundAndPackFloat32(aSign, aExp, (DWORD)aSig, status);
}
float64 CpuInternal::floatx80_to_float64(floatx80 a, float_status_t *status)
{
    int32_t  aExp;
    LARGE aSig, zSig;
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return float64_default_nan;
    }
    aSig      = extractFloatx80Frac(a);
    aExp      = extractFloatx80Exp(a);
    int aSign = extractFloatx80Sign(a);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1)) {
            return CpuInternal::commonNaNToFloat64(floatx80ToCommonNaN(a, status));
        }
        return packFloat64(aSign, 0x7FF, 0);
    }
    zSig = shift64RightJamming(aSig, 1);
    if (aExp || aSig)
        aExp -= 0x3C01;
    return roundAndPackFloat64(aSign, aExp, zSig, status);
}
floatx80 CpuInternal::floatx80_round_to_int(floatx80 a, float_status_t *status)
{
    int      aSign;
    LARGE lastBitMask, roundBitsMask;
    int      roundingMode = get_float_rounding_mode(status);
    floatx80 z;
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    int32_t  aExp = extractFloatx80Exp(a);
    LARGE aSig = extractFloatx80Frac(a);
    if (0x403E <= aExp) {
        if ((aExp == 0x7FFF) && (LARGE)(aSig << 1)) {
            return propagateFloatx80NaN(a, status);
        }
        return a;
    }
    if (aExp < 0x3FFF) {
        if (aExp == 0) {
            if ((aSig << 1) == 0)
                return a;
            float_raise(status, float_flag_denormal);
        }
        float_raise(status, float_flag_inexact);
        aSign = extractFloatx80Sign(a);
        switch (roundingMode) {
            case float_round_nearest_even:
                if ((aExp == 0x3FFE) && (LARGE)(aSig << 1)) {
                    set_float_rounding_up(status);
                    return packFloatx80(aSign, 0x3FFF, 0x8000000000000000ULL);
                }
                break;
            case float_round_down:
                if (aSign) {
                    set_float_rounding_up(status);
                    return packFloatx80(1, 0x3FFF, 0x8000000000000000ULL);
                } else {
                    return packFloatx80(0, 0, 0);
                }
            case float_round_up:
                if (aSign) {
                    return packFloatx80(1, 0, 0);
                } else {
                    set_float_rounding_up(status);
                    return packFloatx80(0, 0x3FFF, 0x8000000000000000ULL);
                }
        }
        return packFloatx80(aSign, 0, 0);
    }
    lastBitMask = 1;
    lastBitMask <<= 0x403E - aExp;
    roundBitsMask = lastBitMask - 1;
    z             = a;
    if (roundingMode == float_round_nearest_even) {
        z.fraction += lastBitMask >> 1;
        if ((z.fraction & roundBitsMask) == 0)
            z.fraction &= ~lastBitMask;
    } else if (roundingMode != float_round_to_zero) {
        if (extractFloatx80Sign(z) ^ (roundingMode == float_round_up))
            z.fraction += roundBitsMask;
    }
    z.fraction &= ~roundBitsMask;
    if (z.fraction == 0) {
        z.exp++;
        z.fraction = 0x8000000000000000ULL;
    }
    if (z.fraction != a.fraction) {
        float_raise(status, float_flag_inexact);
        if (z.fraction > a.fraction || z.exp > a.exp)
            set_float_rounding_up(status);
    }
    return z;
}
floatx80 CpuInternal::addFloatx80Sigs(floatx80 a, floatx80 b, int zSign, float_status_t *status)
{
    int32_t  aExp, bExp, zExp;
    LARGE aSig, bSig, zSig0, zSig1;
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    aSig = extractFloatx80Frac(a);
    aExp = extractFloatx80Exp(a);
    bSig = extractFloatx80Frac(b);
    bExp = extractFloatx80Exp(b);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1) || ((bExp == 0x7FFF) && (LARGE)(bSig << 1)))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return a;
    }
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if ((bExp == 0) && bSig) {
                float_raise(status, float_flag_denormal);
                normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
            }
            return roundAndPackFloatx80(get_float_rounding_precision(status), zSign, bExp, bSig, 0, status);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        if (bSig == 0)
            return roundAndPackFloatx80(get_float_rounding_precision(status), zSign, aExp, aSig, 0, status);
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    int32_t expDiff = aExp - bExp;
    zExp            = aExp;
    if (0 < expDiff) {
        shift64ExtraRightJamming(bSig, 0, expDiff, &bSig, &zSig1);
    } else if (expDiff < 0) {
        shift64ExtraRightJamming(aSig, 0, -expDiff, &aSig, &zSig1);
        zExp = bExp;
    } else {
        zSig0 = aSig + bSig;
        zSig1 = 0;
        goto shiftRight1;
    }
    zSig0 = aSig + bSig;
    if ((int64_t)zSig0 < 0)
        goto roundAndPack;
shiftRight1:
    shift64ExtraRightJamming(zSig0, zSig1, 1, &zSig0, &zSig1);
    zSig0 |= 0x8000000000000000ULL;
    zExp++;
roundAndPack:
    return roundAndPackFloatx80(get_float_rounding_precision(status), zSign, zExp, zSig0, zSig1, status);
}
floatx80 CpuInternal::subFloatx80Sigs(floatx80 a, floatx80 b, int zSign, float_status_t *status)
{
    int32_t  aExp, bExp, zExp;
    LARGE aSig, bSig, zSig0, zSig1;
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    aSig = extractFloatx80Frac(a);
    aExp = extractFloatx80Exp(a);
    bSig = extractFloatx80Frac(b);
    bExp = extractFloatx80Exp(b);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (bExp == 0x7FFF) {
            if ((LARGE)(bSig << 1))
                return propagateFloatx80NaN_two_args(a, b, status);
            float_raise(status, float_flag_invalid);
            return floatx80_default_nan;
        }
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return a;
    }
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloatx80(zSign ^ 1, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if (bExp == 0) {
                if (bSig) {
                    float_raise(status, float_flag_denormal);
                    normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
                    return roundAndPackFloatx80(get_float_rounding_precision(status), zSign ^ 1, bExp, bSig, 0, status);
                }
                return packFloatx80(get_float_rounding_mode(status) == float_round_down, 0, 0);
            }
            return roundAndPackFloatx80(get_float_rounding_precision(status), zSign ^ 1, bExp, bSig, 0, status);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        if (bSig == 0)
            return roundAndPackFloatx80(get_float_rounding_precision(status), zSign, aExp, aSig, 0, status);
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    int32_t expDiff = aExp - bExp;
    if (0 < expDiff) {
        shift128RightJamming(bSig, 0, expDiff, &bSig, &zSig1);
        goto aBigger;
    }
    if (expDiff < 0) {
        shift128RightJamming(aSig, 0, -expDiff, &aSig, &zSig1);
        goto bBigger;
    }
    zSig1 = 0;
    if (bSig < aSig)
        goto aBigger;
    if (aSig < bSig)
        goto bBigger;
    return packFloatx80(get_float_rounding_mode(status) == float_round_down, 0, 0);
bBigger:
    sub128(bSig, 0, aSig, zSig1, &zSig0, &zSig1);
    zExp = bExp;
    zSign ^= 1;
    goto normalizeRoundAndPack;
aBigger:
    sub128(aSig, 0, bSig, zSig1, &zSig0, &zSig1);
    zExp = aExp;
normalizeRoundAndPack:
    return normalizeRoundAndPackFloatx80(get_float_rounding_precision(status), zSign, zExp, zSig0, zSig1, status);
}
floatx80 CpuInternal::floatx80_add(floatx80 a, floatx80 b, float_status_t *status)
{
    int aSign = extractFloatx80Sign(a);
    int bSign = extractFloatx80Sign(b);
    if (aSign == bSign)
        return addFloatx80Sigs(a, b, aSign, status);
    else
        return subFloatx80Sigs(a, b, aSign, status);
}
floatx80 CpuInternal::floatx80_sub(floatx80 a, floatx80 b, float_status_t *status)
{
    int aSign = extractFloatx80Sign(a);
    int bSign = extractFloatx80Sign(b);
    if (aSign == bSign)
        return subFloatx80Sigs(a, b, aSign, status);
    else
        return addFloatx80Sigs(a, b, aSign, status);
}
floatx80 CpuInternal::floatx80_mul(floatx80 a, floatx80 b, float_status_t *status)
{
    int      aSign, bSign, zSign;
    int32_t  aExp, bExp, zExp;
    LARGE aSig, bSig, zSig0, zSig1;
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
    invalid:
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    aSig  = extractFloatx80Frac(a);
    aExp  = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    bSig  = extractFloatx80Frac(b);
    bExp  = extractFloatx80Exp(b);
    bSign = extractFloatx80Sign(b);
    zSign = aSign ^ bSign;
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1) || ((bExp == 0x7FFF) && (LARGE)(bSig << 1))) {
            return propagateFloatx80NaN_two_args(a, b, status);
        }
        if (bExp == 0) {
            if (bSig == 0)
                goto invalid;
            float_raise(status, float_flag_denormal);
        }
        return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (aExp == 0) {
            if (aSig == 0)
                goto invalid;
            float_raise(status, float_flag_denormal);
        }
        return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if (bSig && (bExp == 0))
                float_raise(status, float_flag_denormal);
            return packFloatx80(zSign, 0, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        if (bSig == 0)
            return packFloatx80(zSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    zExp = aExp + bExp - 0x3FFE;
    mul64To128(aSig, bSig, &zSig0, &zSig1);
    if (0 < (int64_t)zSig0) {
        shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
        --zExp;
    }
    return roundAndPackFloatx80(get_float_rounding_precision(status), zSign, zExp, zSig0, zSig1, status);
}
floatx80 CpuInternal::floatx80_div(floatx80 a, floatx80 b, float_status_t *status)
{
    int      aSign, bSign, zSign;
    int32_t  aExp, bExp, zExp;
    LARGE aSig, bSig, zSig0, zSig1;
    LARGE rem0, rem1, rem2, term0, term1, term2;
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    aSig  = extractFloatx80Frac(a);
    aExp  = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    bSig  = extractFloatx80Frac(b);
    bExp  = extractFloatx80Exp(b);
    bSign = extractFloatx80Sign(b);
    zSign = aSign ^ bSign;
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (bExp == 0x7FFF) {
            if ((LARGE)(bSig << 1))
                return propagateFloatx80NaN_two_args(a, b, status);
            float_raise(status, float_flag_invalid);
            return floatx80_default_nan;
        }
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return packFloatx80(zSign, 0, 0);
    }
    if (bExp == 0) {
        if (bSig == 0) {
            if ((aExp | aSig) == 0) {
                float_raise(status, float_flag_invalid);
                return floatx80_default_nan;
            }
            float_raise(status, float_flag_divbyzero);
            return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    if (aExp == 0) {
        if (aSig == 0)
            return packFloatx80(zSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    zExp = aExp - bExp + 0x3FFE;
    rem1 = 0;
    if (bSig <= aSig) {
        shift128Right(aSig, 0, 1, &aSig, &rem1);
        ++zExp;
    }
    zSig0 = estimateDiv128To64(aSig, rem1, bSig);
    mul64To128(bSig, zSig0, &term0, &term1);
    sub128(aSig, rem1, term0, term1, &rem0, &rem1);
    while ((int64_t)rem0 < 0) {
        --zSig0;
        add128(rem0, rem1, 0, bSig, &rem0, &rem1);
    }
    zSig1 = estimateDiv128To64(rem1, 0, bSig);
    if ((LARGE)(zSig1 << 1) <= 8) {
        mul64To128(bSig, zSig1, &term1, &term2);
        sub128(rem1, 0, term1, term2, &rem1, &rem2);
        while ((int64_t)rem1 < 0) {
            --zSig1;
            add128(rem1, rem2, 0, bSig, &rem1, &rem2);
        }
        zSig1 |= ((rem1 | rem2) != 0);
    }
    return roundAndPackFloatx80(get_float_rounding_precision(status), zSign, zExp, zSig0, zSig1, status);
}
floatx80 CpuInternal::floatx80_sqrt(floatx80 a, float_status_t *status)
{
    int      aSign;
    int32_t  aExp, zExp;
    LARGE aSig0, aSig1, zSig0, zSig1, doubleZSig0;
    LARGE rem0, rem1, rem2, rem3, term0, term1, term2, term3;
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    aSig0 = extractFloatx80Frac(a);
    aExp  = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig0 << 1))
            return propagateFloatx80NaN(a, status);
        if (!aSign)
            return a;
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    if (aSign) {
        if ((aExp | aSig0) == 0)
            return a;
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    if (aExp == 0) {
        if (aSig0 == 0)
            return packFloatx80(0, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig0, &aExp, &aSig0);
    }
    zExp  = ((aExp - 0x3FFF) >> 1) + 0x3FFF;
    zSig0 = estimateSqrt32(aExp, aSig0 >> 32);
    shift128Right(aSig0, 0, 2 + (aExp & 1), &aSig0, &aSig1);
    zSig0       = estimateDiv128To64(aSig0, aSig1, zSig0 << 32) + (zSig0 << 30);
    doubleZSig0 = zSig0 << 1;
    mul64To128(zSig0, zSig0, &term0, &term1);
    sub128(aSig0, aSig1, term0, term1, &rem0, &rem1);
    while ((int64_t)rem0 < 0) {
        --zSig0;
        doubleZSig0 -= 2;
        add128(rem0, rem1, zSig0 >> 63, doubleZSig0 | 1, &rem0, &rem1);
    }
    zSig1 = estimateDiv128To64(rem1, 0, doubleZSig0);
    if ((zSig1 & 0x3FFFFFFFFFFFFFFFULL) <= 5) {
        if (zSig1 == 0)
            zSig1 = 1;
        mul64To128(doubleZSig0, zSig1, &term1, &term2);
        sub128(rem1, 0, term1, term2, &rem1, &rem2);
        mul64To128(zSig1, zSig1, &term2, &term3);
        sub192(rem1, rem2, 0, 0, term2, term3, &rem1, &rem2, &rem3);
        while ((int64_t)rem1 < 0) {
            --zSig1;
            shortShift128Left(0, zSig1, 1, &term2, &term3);
            term3 |= 1;
            term2 |= doubleZSig0;
            add192(rem1, rem2, rem3, 0, term2, term3, &rem1, &rem2, &rem3);
        }
        zSig1 |= ((rem1 | rem2 | rem3) != 0);
    }
    shortShift128Left(0, zSig1, 1, &zSig0, &zSig1);
    zSig0 |= doubleZSig0;
    return roundAndPackFloatx80(get_float_rounding_precision(status), 0, zExp, zSig0, zSig1, status);
}
float128 CpuInternal::floatx80_to_float128(floatx80 a, float_status_t *status)
{
    LARGE zSig0, zSig1;
    LARGE aSig  = extractFloatx80Frac(a);
    int32_t  aExp  = extractFloatx80Exp(a);
    int      aSign = extractFloatx80Sign(a);
    if ((aExp == 0x7FFF) && (LARGE)(aSig << 1))
        return CpuInternal::commonNaNToFloat128(floatx80ToCommonNaN(a, status));
    shift128Right(aSig << 1, 0, 16, &zSig0, &zSig1);
    return packFloat128(aSign, aExp, zSig0, zSig1);
}
floatx80 CpuInternal::float128_to_floatx80(float128 a, float_status_t *status)
{
    int32_t  aExp;
    LARGE aSig0, aSig1;
    aSig1     = extractFloat128Frac1(a);
    aSig0     = extractFloat128Frac0(a);
    aExp      = extractFloat128Exp(a);
    int aSign = extractFloat128Sign(a);
    if (aExp == 0x7FFF) {
        if (aSig0 | aSig1)
            return CpuInternal::commonNaNToFloatx80(float128ToCommonNaN(a, status));
        return packFloatx80(aSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if ((aSig0 | aSig1) == 0)
            return packFloatx80(aSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat128Subnormal(aSig0, aSig1, &aExp, &aSig0, &aSig1);
    } else
        aSig0 |= 0x0001000000000000ULL;
    shortShift128Left(aSig0, aSig1, 15, &aSig0, &aSig1);
    return roundAndPackFloatx80(80, aSign, aExp, aSig0, aSig1, status);
}
floatx80 CpuInternal::floatx80_mul_with_float128(floatx80 a, float128 b, float_status_t *status)
{
    int32_t  aExp, bExp, zExp;
    LARGE aSig, bSig0, bSig1, zSig0, zSig1, zSig2;
    int      aSign, bSign, zSign;
    if (floatx80_is_unsupported(a)) {
    invalid:
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    aSig  = extractFloatx80Frac(a);
    aExp  = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    bSig0 = extractFloat128Frac0(b);
    bSig1 = extractFloat128Frac1(b);
    bExp  = extractFloat128Exp(b);
    bSign = extractFloat128Sign(b);
    zSign = aSign ^ bSign;
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1) || ((bExp == 0x7FFF) && (bSig0 | bSig1))) {
            floatx80 r = CpuInternal::commonNaNToFloatx80(float128ToCommonNaN(b, status));
            return propagateFloatx80NaN_two_args(a, r, status);
        }
        if (bExp == 0) {
            if ((bSig0 | bSig1) == 0)
                goto invalid;
            float_raise(status, float_flag_denormal);
        }
        return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (bExp == 0x7FFF) {
        if (bSig0 | bSig1) {
            floatx80 r = CpuInternal::commonNaNToFloatx80(float128ToCommonNaN(b, status));
            return propagateFloatx80NaN_two_args(a, r, status);
        }
        if (aExp == 0) {
            if (aSig == 0)
                goto invalid;
            float_raise(status, float_flag_denormal);
        }
        return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if ((bExp == 0) && (bSig0 | bSig1))
                float_raise(status, float_flag_denormal);
            return packFloatx80(zSign, 0, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        if ((bSig0 | bSig1) == 0)
            return packFloatx80(zSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat128Subnormal(bSig0, bSig1, &bExp, &bSig0, &bSig1);
    } else
        bSig0 |= 0x0001000000000000ULL;
    zExp = aExp + bExp - 0x3FFE;
    shortShift128Left(bSig0, bSig1, 15, &bSig0, &bSig1);
    mul128By64To192(bSig0, bSig1, aSig, &zSig0, &zSig1, &zSig2);
    if (0 < (int64_t)zSig0) {
        shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
        --zExp;
    }
    return roundAndPackFloatx80(get_float_rounding_precision(status), zSign, zExp, zSig0, zSig1, status);
}
float128 CpuInternal::addFloat128Sigs(float128 a, float128 b, int zSign, float_status_t *status)
{
    int32_t  aExp, bExp, zExp;
    LARGE aSig0, aSig1, bSig0, bSig1, zSig0, zSig1, zSig2;
    int32_t  expDiff;
    aSig1   = extractFloat128Frac1(a);
    aSig0   = extractFloat128Frac0(a);
    aExp    = extractFloat128Exp(a);
    bSig1   = extractFloat128Frac1(b);
    bSig0   = extractFloat128Frac0(b);
    bExp    = extractFloat128Exp(b);
    expDiff = aExp - bExp;
    if (0 < expDiff) {
        if (aExp == 0x7FFF) {
            if (aSig0 | aSig1)
                return propagateFloat128NaN(a, b, status);
            return a;
        }
        if (bExp == 0)
            --expDiff;
        else
            bSig0 |= 0x0001000000000000ULL;
        shift128ExtraRightJamming(bSig0, bSig1, 0, expDiff, &bSig0, &bSig1, &zSig2);
        zExp = aExp;
    } else if (expDiff < 0) {
        if (bExp == 0x7FFF) {
            if (bSig0 | bSig1)
                return propagateFloat128NaN(a, b, status);
            return packFloat128(zSign, 0x7FFF, 0, 0);
        }
        if (aExp == 0)
            ++expDiff;
        else
            aSig0 |= 0x0001000000000000ULL;
        shift128ExtraRightJamming(aSig0, aSig1, 0, -expDiff, &aSig0, &aSig1, &zSig2);
        zExp = bExp;
    } else {
        if (aExp == 0x7FFF) {
            if (aSig0 | aSig1 | bSig0 | bSig1)
                return propagateFloat128NaN(a, b, status);
            return a;
        }
        add128(aSig0, aSig1, bSig0, bSig1, &zSig0, &zSig1);
        if (aExp == 0)
            return packFloat128(zSign, 0, zSig0, zSig1);
        zSig2 = 0;
        zSig0 |= 0x0002000000000000ULL;
        zExp = aExp;
        goto shiftRight1;
    }
    aSig0 |= 0x0001000000000000ULL;
    add128(aSig0, aSig1, bSig0, bSig1, &zSig0, &zSig1);
    --zExp;
    if (zSig0 < 0x0002000000000000ULL)
        goto roundAndPack;
    ++zExp;
shiftRight1:
    shift128ExtraRightJamming(zSig0, zSig1, zSig2, 1, &zSig0, &zSig1, &zSig2);
roundAndPack:
    return roundAndPackFloat128(zSign, zExp, zSig0, zSig1, zSig2, status);
}
float128 CpuInternal::subFloat128Sigs(float128 a, float128 b, int zSign, float_status_t *status)
{
    int32_t  aExp, bExp, zExp;
    LARGE aSig0, aSig1, bSig0, bSig1, zSig0, zSig1;
    int32_t  expDiff;
    aSig1   = extractFloat128Frac1(a);
    aSig0   = extractFloat128Frac0(a);
    aExp    = extractFloat128Exp(a);
    bSig1   = extractFloat128Frac1(b);
    bSig0   = extractFloat128Frac0(b);
    bExp    = extractFloat128Exp(b);
    expDiff = aExp - bExp;
    shortShift128Left(aSig0, aSig1, 14, &aSig0, &aSig1);
    shortShift128Left(bSig0, bSig1, 14, &bSig0, &bSig1);
    if (0 < expDiff)
        goto aExpBigger;
    if (expDiff < 0)
        goto bExpBigger;
    if (aExp == 0x7FFF) {
        if (aSig0 | aSig1 | bSig0 | bSig1)
            return propagateFloat128NaN(a, b, status);
        float_raise(status, float_flag_invalid);
        return float128_default_nan;
    }
    if (aExp == 0) {
        aExp = 1;
        bExp = 1;
    }
    if (bSig0 < aSig0)
        goto aBigger;
    if (aSig0 < bSig0)
        goto bBigger;
    if (bSig1 < aSig1)
        goto aBigger;
    if (aSig1 < bSig1)
        goto bBigger;
    return packFloat128_simple(0, 0);
bExpBigger:
    if (bExp == 0x7FFF) {
        if (bSig0 | bSig1)
            return propagateFloat128NaN(a, b, status);
        return packFloat128(zSign ^ 1, 0x7FFF, 0, 0);
    }
    if (aExp == 0)
        ++expDiff;
    else {
        aSig0 |= 0x4000000000000000ULL;
    }
    shift128RightJamming(aSig0, aSig1, -expDiff, &aSig0, &aSig1);
    bSig0 |= 0x4000000000000000ULL;
bBigger:
    sub128(bSig0, bSig1, aSig0, aSig1, &zSig0, &zSig1);
    zExp = bExp;
    zSign ^= 1;
    goto normalizeRoundAndPack;
aExpBigger:
    if (aExp == 0x7FFF) {
        if (aSig0 | aSig1)
            return propagateFloat128NaN(a, b, status);
        return a;
    }
    if (bExp == 0)
        --expDiff;
    else {
        bSig0 |= 0x4000000000000000ULL;
    }
    shift128RightJamming(bSig0, bSig1, expDiff, &bSig0, &bSig1);
    aSig0 |= 0x4000000000000000ULL;
aBigger:
    sub128(aSig0, aSig1, bSig0, bSig1, &zSig0, &zSig1);
    zExp = aExp;
normalizeRoundAndPack:
    --zExp;
    return normalizeRoundAndPackFloat128(zSign, zExp - 14, zSig0, zSig1, status);
}
float128 CpuInternal::float128_add(float128 a, float128 b, float_status_t *status)
{
    int aSign = extractFloat128Sign(a);
    int bSign = extractFloat128Sign(b);
    if (aSign == bSign) {
        return addFloat128Sigs(a, b, aSign, status);
    } else {
        return subFloat128Sigs(a, b, aSign, status);
    }
}
float128 CpuInternal::float128_sub(float128 a, float128 b, float_status_t *status)
{
    int aSign = extractFloat128Sign(a);
    int bSign = extractFloat128Sign(b);
    if (aSign == bSign) {
        return subFloat128Sigs(a, b, aSign, status);
    } else {
        return addFloat128Sigs(a, b, aSign, status);
    }
}
float128 CpuInternal::float128_mul(float128 a, float128 b, float_status_t *status)
{
    int      aSign, bSign, zSign;
    int32_t  aExp, bExp, zExp;
    LARGE aSig0, aSig1, bSig0, bSig1, zSig0, zSig1, zSig2, zSig3;
    aSig1 = extractFloat128Frac1(a);
    aSig0 = extractFloat128Frac0(a);
    aExp  = extractFloat128Exp(a);
    aSign = extractFloat128Sign(a);
    bSig1 = extractFloat128Frac1(b);
    bSig0 = extractFloat128Frac0(b);
    bExp  = extractFloat128Exp(b);
    bSign = extractFloat128Sign(b);
    zSign = aSign ^ bSign;
    if (aExp == 0x7FFF) {
        if ((aSig0 | aSig1) || ((bExp == 0x7FFF) && (bSig0 | bSig1))) {
            return propagateFloat128NaN(a, b, status);
        }
        if ((bExp | bSig0 | bSig1) == 0) {
            float_raise(status, float_flag_invalid);
            return float128_default_nan;
        }
        return packFloat128(zSign, 0x7FFF, 0, 0);
    }
    if (bExp == 0x7FFF) {
        if (bSig0 | bSig1)
            return propagateFloat128NaN(a, b, status);
        if ((aExp | aSig0 | aSig1) == 0) {
            float_raise(status, float_flag_invalid);
            return float128_default_nan;
        }
        return packFloat128(zSign, 0x7FFF, 0, 0);
    }
    if (aExp == 0) {
        if ((aSig0 | aSig1) == 0)
            return packFloat128(zSign, 0, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat128Subnormal(aSig0, aSig1, &aExp, &aSig0, &aSig1);
    }
    if (bExp == 0) {
        if ((bSig0 | bSig1) == 0)
            return packFloat128(zSign, 0, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat128Subnormal(bSig0, bSig1, &bExp, &bSig0, &bSig1);
    }
    zExp = aExp + bExp - 0x4000;
    aSig0 |= 0x0001000000000000ULL;
    shortShift128Left(bSig0, bSig1, 16, &bSig0, &bSig1);
    mul128To256(aSig0, aSig1, bSig0, bSig1, &zSig0, &zSig1, &zSig2, &zSig3);
    add128(zSig0, zSig1, aSig0, aSig1, &zSig0, &zSig1);
    zSig2 |= (zSig3 != 0);
    if (0x0002000000000000ULL <= zSig0) {
        shift128ExtraRightJamming(zSig0, zSig1, zSig2, 1, &zSig0, &zSig1, &zSig2);
        ++zExp;
    }
    return roundAndPackFloat128(zSign, zExp, zSig0, zSig1, zSig2, status);
}
float128 CpuInternal::float128_div(float128 a, float128 b, float_status_t *status)
{
    int      aSign, bSign, zSign;
    int32_t  aExp, bExp, zExp;
    LARGE aSig0, aSig1, bSig0, bSig1, zSig0, zSig1, zSig2;
    LARGE rem0, rem1, rem2, rem3, term0, term1, term2, term3;
    aSig1 = extractFloat128Frac1(a);
    aSig0 = extractFloat128Frac0(a);
    aExp  = extractFloat128Exp(a);
    aSign = extractFloat128Sign(a);
    bSig1 = extractFloat128Frac1(b);
    bSig0 = extractFloat128Frac0(b);
    bExp  = extractFloat128Exp(b);
    bSign = extractFloat128Sign(b);
    zSign = aSign ^ bSign;
    if (aExp == 0x7FFF) {
        if (aSig0 | aSig1)
            return propagateFloat128NaN(a, b, status);
        if (bExp == 0x7FFF) {
            if (bSig0 | bSig1)
                return propagateFloat128NaN(a, b, status);
            float_raise(status, float_flag_invalid);
            return float128_default_nan;
        }
        return packFloat128(zSign, 0x7FFF, 0, 0);
    }
    if (bExp == 0x7FFF) {
        if (bSig0 | bSig1)
            return propagateFloat128NaN(a, b, status);
        return packFloat128(zSign, 0, 0, 0);
    }
    if (bExp == 0) {
        if ((bSig0 | bSig1) == 0) {
            if ((aExp | aSig0 | aSig1) == 0) {
                float_raise(status, float_flag_invalid);
                return float128_default_nan;
            }
            float_raise(status, float_flag_divbyzero);
            return packFloat128(zSign, 0x7FFF, 0, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloat128Subnormal(bSig0, bSig1, &bExp, &bSig0, &bSig1);
    }
    if (aExp == 0) {
        if ((aSig0 | aSig1) == 0)
            return packFloat128(zSign, 0, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloat128Subnormal(aSig0, aSig1, &aExp, &aSig0, &aSig1);
    }
    zExp = aExp - bExp + 0x3FFD;
    shortShift128Left(aSig0 | 0x0001000000000000ULL, aSig1, 15, &aSig0, &aSig1);
    shortShift128Left(bSig0 | 0x0001000000000000ULL, bSig1, 15, &bSig0, &bSig1);
    if (le128(bSig0, bSig1, aSig0, aSig1)) {
        shift128Right(aSig0, aSig1, 1, &aSig0, &aSig1);
        ++zExp;
    }
    zSig0 = estimateDiv128To64(aSig0, aSig1, bSig0);
    mul128By64To192(bSig0, bSig1, zSig0, &term0, &term1, &term2);
    sub192(aSig0, aSig1, 0, term0, term1, term2, &rem0, &rem1, &rem2);
    while ((int64_t)rem0 < 0) {
        --zSig0;
        add192(rem0, rem1, rem2, 0, bSig0, bSig1, &rem0, &rem1, &rem2);
    }
    zSig1 = estimateDiv128To64(rem1, rem2, bSig0);
    if ((zSig1 & 0x3FFF) <= 4) {
        mul128By64To192(bSig0, bSig1, zSig1, &term1, &term2, &term3);
        sub192(rem1, rem2, 0, term1, term2, term3, &rem1, &rem2, &rem3);
        while ((int64_t)rem1 < 0) {
            --zSig1;
            add192(rem1, rem2, rem3, 0, bSig0, bSig1, &rem1, &rem2, &rem3);
        }
        zSig1 |= ((rem1 | rem2 | rem3) != 0);
    }
    shift128ExtraRightJamming(zSig0, zSig1, 0, 15, &zSig0, &zSig1, &zSig2);
    return roundAndPackFloat128(zSign, zExp, zSig0, zSig1, zSig2, status);
}
float128 CpuInternal::int64_to_float128(int64_t a)
{
    LARGE zSig0, zSig1;
    if (a == 0)
        return packFloat128(0, 0, 0, 0);
    int      zSign      = (a < 0);
    LARGE absA       = zSign ? -a : a;
    BYTE  shiftCount = countLeadingZeros64(absA) + 49;
    int32_t  zExp       = 0x406E - shiftCount;
    if (64 <= shiftCount) {
        zSig1 = 0;
        zSig0 = absA;
        shiftCount -= 64;
    } else {
        zSig1 = absA;
        zSig0 = 0;
    }
    shortShift128Left(zSig0, zSig1, shiftCount, &zSig0, &zSig1);
    return packFloat128(zSign, zExp, zSig0, zSig1);
}
int16_t CpuInternal::floatx80_to_int16(floatx80 a, float_status_t *status)
{
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return ((int16_t)0x8000);
    }
    int32_t v32 = floatx80_to_int32(a, status);
    if ((v32 > 32767) || (v32 < -32768)) {
        status->float_exception_flags = float_flag_invalid;
        return ((int16_t)0x8000);
    }
    return (int16_t)v32;
}
int16_t CpuInternal::floatx80_to_int16_round_to_zero(floatx80 a, float_status_t *status)
{
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return ((int16_t)0x8000);
    }
    int32_t v32 = floatx80_to_int32_round_to_zero(a, status);
    if ((v32 > 32767) || (v32 < -32768)) {
        status->float_exception_flags = float_flag_invalid;
        return ((int16_t)0x8000);
    }
    return (int16_t)v32;
}
floatx80 CpuInternal::floatx80_extract(floatx80 *input, float_status_t *status)
{
    floatx80 a     = *input;
    LARGE aSig  = extractFloatx80Frac(a);
    int32_t  aExp  = extractFloatx80Exp(a);
    int      aSign = extractFloatx80Sign(a);
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        a      = floatx80_default_nan;
        *input = a;
        return a;
    }
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1)) {
            a      = propagateFloatx80NaN(a, status);
            *input = a;
            return a;
        }
        *input = a;
        return packFloatx80(0, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            float_raise(status, float_flag_divbyzero);
            a      = packFloatx80(aSign, 0, 0);
            *input = a;
            return packFloatx80(1, 0x7FFF, 0x8000000000000000ULL);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    a.exp      = (aSign << 15) + 0x3FFF;
    a.fraction = aSig;
    *input     = a;
    return int32_to_floatx80(aExp - 0x3FFF);
}
floatx80 CpuInternal::floatx80_scale(floatx80 a, floatx80 b, float_status_t *status)
{
    int32_t  aExp, bExp;
    LARGE aSig, bSig;
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    aSig      = extractFloatx80Frac(a);
    aExp      = extractFloatx80Exp(a);
    int aSign = extractFloatx80Sign(a);
    bSig      = extractFloatx80Frac(b);
    bExp      = extractFloatx80Exp(b);
    int bSign = extractFloatx80Sign(b);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1) || ((bExp == 0x7FFF) && (LARGE)(bSig << 1))) {
            return propagateFloatx80NaN_two_args(a, b, status);
        }
        if ((bExp == 0x7FFF) && bSign) {
            float_raise(status, float_flag_invalid);
            return floatx80_default_nan;
        }
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        return a;
    }
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if ((aExp | aSig) == 0) {
            if (!bSign) {
                float_raise(status, float_flag_invalid);
                return floatx80_default_nan;
            }
            return a;
        }
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        if (bSign)
            return packFloatx80(aSign, 0, 0);
        return packFloatx80(aSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
        if (aSig == 0)
            return a;
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
        if (bExp < 0x3FFF)
            return normalizeRoundAndPackFloatx80(80, aSign, aExp, aSig, 0, status);
    }
    if (bExp == 0) {
        if (bSig == 0)
            return a;
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    if (bExp > 0x400E) {
        return roundAndPackFloatx80(80, aSign, bSign ? -0x3FFF : 0x7FFF, aSig, 0, status);
    }
    if (bExp < 0x3FFF)
        return a;
    int shiftCount = 0x403E - bExp;
    bSig >>= shiftCount;
    int32_t scale = (int32_t)bSig;
    if (bSign)
        scale = -scale;
    return roundAndPackFloatx80(80, aSign, aExp + scale, aSig, 0, status);
}
float_class_t CpuInternal::floatx80_class(floatx80 a)
{
    int32_t  aExp = extractFloatx80Exp(a);
    LARGE aSig = extractFloatx80Frac(a);
    if (aExp == 0) {
        if (aSig == 0)
            return float_zero;
        return float_denormal;
    }
    if (!(aSig & 0x8000000000000000ULL))
        return float_SNaN;
    if (aExp == 0x7fff) {
        int aSign = extractFloatx80Sign(a);
        if (((LARGE)(aSig << 1)) == 0)
            return (aSign) ? float_negative_inf : float_positive_inf;
        return (aSig & 0x4000000000000000ULL) ? float_QNaN : float_SNaN;
    }
    return float_normalized;
}
int CpuInternal::floatx80_compare_internal(floatx80 a, floatx80 b, int quiet, float_status_t *status)
{
    float_class_t aClass = floatx80_class(a);
    float_class_t bClass = floatx80_class(b);
    if (aClass == float_SNaN || bClass == float_SNaN) {
        float_raise(status, float_flag_invalid);
        return float_relation_unordered;
    }
    if (aClass == float_QNaN || bClass == float_QNaN) {
        if (!quiet)
            float_raise(status, float_flag_invalid);
        return float_relation_unordered;
    }
    if (aClass == float_denormal || bClass == float_denormal) {
        float_raise(status, float_flag_denormal);
    }
    int aSign = extractFloatx80Sign(a);
    int bSign = extractFloatx80Sign(b);
    if (aClass == float_zero) {
        if (bClass == float_zero)
            return float_relation_equal;
        return bSign ? float_relation_greater : float_relation_less;
    }
    if (bClass == float_zero || aSign != bSign) {
        return aSign ? float_relation_less : float_relation_greater;
    }
    LARGE aSig = extractFloatx80Frac(a);
    int32_t  aExp = extractFloatx80Exp(a);
    LARGE bSig = extractFloatx80Frac(b);
    int32_t  bExp = extractFloatx80Exp(b);
    if (aClass == float_denormal)
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    if (bClass == float_denormal)
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    if (aExp == bExp && aSig == bSig)
        return float_relation_equal;
    int less_than = aSign ? ((bExp < aExp) || ((bExp == aExp) && (bSig < aSig)))
                          : ((aExp < bExp) || ((aExp == bExp) && (aSig < bSig)));
    if (less_than)
        return float_relation_less;
    return float_relation_greater;
}
float32 CpuInternal::propagateFloat32NaN_two_args(float32 a, float32 b, float_status_t *status)
{
    int aIsNaN, aIsSignalingNaN, bIsNaN, bIsSignalingNaN;
    aIsNaN          = float32_is_nan(a);
    aIsSignalingNaN = float32_is_signaling_nan(a);
    bIsNaN          = float32_is_nan(b);
    bIsSignalingNaN = float32_is_signaling_nan(b);
    a |= 0x00400000;
    b |= 0x00400000;
    if (aIsSignalingNaN | bIsSignalingNaN)
        float_raise(status, float_flag_invalid);
    if (get_float_nan_handling_mode(status) == float_larger_significand_nan) {
        if (aIsSignalingNaN) {
            if (bIsSignalingNaN)
                goto returnLargerSignificand;
            return bIsNaN ? b : a;
        } else if (aIsNaN) {
            if (bIsSignalingNaN | !bIsNaN)
                return a;
        returnLargerSignificand:
            if ((DWORD)(a << 1) < (DWORD)(b << 1))
                return b;
            if ((DWORD)(b << 1) < (DWORD)(a << 1))
                return a;
            return (a < b) ? a : b;
        } else {
            return b;
        }
    } else {
        return (aIsSignalingNaN | aIsNaN) ? a : b;
    }
}
float64 CpuInternal::propagateFloat64NaN_two_args(float64 a, float64 b, float_status_t *status)
{
    int aIsNaN          = float64_is_nan(a);
    int aIsSignalingNaN = float64_is_signaling_nan(a);
    int bIsNaN          = float64_is_nan(b);
    int bIsSignalingNaN = float64_is_signaling_nan(b);
    a |= 0x0008000000000000ULL;
    b |= 0x0008000000000000ULL;
    if (aIsSignalingNaN | bIsSignalingNaN)
        float_raise(status, float_flag_invalid);
    if (get_float_nan_handling_mode(status) == float_larger_significand_nan) {
        if (aIsSignalingNaN) {
            if (bIsSignalingNaN)
                goto returnLargerSignificand;
            return bIsNaN ? b : a;
        } else if (aIsNaN) {
            if (bIsSignalingNaN | !bIsNaN)
                return a;
        returnLargerSignificand:
            if ((LARGE)(a << 1) < (LARGE)(b << 1))
                return b;
            if ((LARGE)(b << 1) < (LARGE)(a << 1))
                return a;
            return (a < b) ? a : b;
        } else {
            return b;
        }
    } else {
        return (aIsSignalingNaN | aIsNaN) ? a : b;
    }
}
floatx80 CpuInternal::propagateFloatx80NaN_two_args(floatx80 a, floatx80 b, float_status_t *status)
{
    int aIsNaN          = floatx80_is_nan(a);
    int aIsSignalingNaN = floatx80_is_signaling_nan(a);
    int bIsNaN          = floatx80_is_nan(b);
    int bIsSignalingNaN = floatx80_is_signaling_nan(b);
    a.fraction |= 0xC000000000000000ULL;
    b.fraction |= 0xC000000000000000ULL;
    if (aIsSignalingNaN | bIsSignalingNaN)
        float_raise(status, float_flag_invalid);
    if (aIsSignalingNaN) {
        if (bIsSignalingNaN)
            goto returnLargerSignificand;
        return bIsNaN ? b : a;
    } else if (aIsNaN) {
        if (bIsSignalingNaN | !bIsNaN)
            return a;
    returnLargerSignificand:
        if (a.fraction < b.fraction)
            return b;
        if (b.fraction < a.fraction)
            return a;
        return (a.exp < b.exp) ? a : b;
    } else {
        return b;
    }
}
float128 CpuInternal::propagateFloat128NaN(float128 a, float128 b, float_status_t *status)
{
    int aIsNaN, aIsSignalingNaN, bIsNaN, bIsSignalingNaN;
    aIsNaN          = float128_is_nan(a);
    aIsSignalingNaN = float128_is_signaling_nan(a);
    bIsNaN          = float128_is_nan(b);
    bIsSignalingNaN = float128_is_signaling_nan(b);
    a.hi |= 0x0000800000000000ULL;
    b.hi |= 0x0000800000000000ULL;
    if (aIsSignalingNaN | bIsSignalingNaN)
        float_raise(status, float_flag_invalid);
    if (aIsSignalingNaN) {
        if (bIsSignalingNaN)
            goto returnLargerSignificand;
        return bIsNaN ? b : a;
    } else if (aIsNaN) {
        if (bIsSignalingNaN | !bIsNaN)
            return a;
    returnLargerSignificand:
        if (lt128(a.hi << 1, a.lo, b.hi << 1, b.lo))
            return b;
        if (lt128(b.hi << 1, b.lo, a.hi << 1, a.lo))
            return a;
        return (a.hi < b.hi) ? a : b;
    } else {
        return b;
    }
}
int32_t CpuInternal::roundAndPackInt32(int zSign, LARGE exactAbsZ, float_status_t *status)
{
    int roundingMode     = get_float_rounding_mode(status);
    int roundNearestEven = (roundingMode == float_round_nearest_even);
    int roundIncrement   = 0x40;
    if (!roundNearestEven) {
        if (roundingMode == float_round_to_zero)
            roundIncrement = 0;
        else {
            roundIncrement = 0x7F;
            if (zSign) {
                if (roundingMode == float_round_up)
                    roundIncrement = 0;
            } else {
                if (roundingMode == float_round_down)
                    roundIncrement = 0;
            }
        }
    }
    int      roundBits = (int)(exactAbsZ & 0x7F);
    LARGE absZ      = (exactAbsZ + roundIncrement) >> 7;
    absZ &= ~(((roundBits ^ 0x40) == 0) & roundNearestEven);
    int32_t z = (int32_t)absZ;
    if (zSign)
        z = -z;
    if ((absZ >> 32) || (z && ((z < 0) ^ zSign))) {
        float_raise(status, float_flag_invalid);
        return (int32_t)(((int32_t)0x80000000));
    }
    if (roundBits) {
        float_raise(status, float_flag_inexact);
        if ((absZ << 7) > exactAbsZ)
            set_float_rounding_up(status);
    }
    return z;
}
int64_t CpuInternal::roundAndPackInt64(int zSign, LARGE absZ0, LARGE absZ1, float_status_t *status)
{
    int64_t z;
    int     roundingMode     = get_float_rounding_mode(status);
    int     roundNearestEven = (roundingMode == float_round_nearest_even);
    int     increment        = ((int64_t)absZ1 < 0);
    if (!roundNearestEven) {
        if (roundingMode == float_round_to_zero)
            increment = 0;
        else {
            if (zSign) {
                increment = (roundingMode == float_round_down) && absZ1;
            } else {
                increment = (roundingMode == float_round_up) && absZ1;
            }
        }
    }
    LARGE exactAbsZ0 = absZ0;
    if (increment) {
        ++absZ0;
        if (absZ0 == 0)
            goto overflow;
        absZ0 &= ~(((LARGE)(absZ1 << 1) == 0) & roundNearestEven);
    }
    z = absZ0;
    if (zSign)
        z = -z;
    if (z && ((z < 0) ^ zSign)) {
    overflow:
        float_raise(status, float_flag_invalid);
        return (int64_t)(0x8000000000000000ULL);
    }
    if (absZ1) {
        float_raise(status, float_flag_inexact);
        if (absZ0 > exactAbsZ0)
            set_float_rounding_up(status);
    }
    return z;
}
LARGE CpuInternal::roundAndPackUint64(int zSign, LARGE absZ0, LARGE absZ1, float_status_t *status)
{
    int roundingMode     = get_float_rounding_mode(status);
    int roundNearestEven = (roundingMode == float_round_nearest_even);
    int increment        = ((int64_t)absZ1 < 0);
    if (!roundNearestEven) {
        if (roundingMode == float_round_to_zero) {
            increment = 0;
        } else if (absZ1) {
            if (zSign) {
                increment = (roundingMode == float_round_down) && absZ1;
            } else {
                increment = (roundingMode == float_round_up) && absZ1;
            }
        }
    }
    if (increment) {
        ++absZ0;
        if (absZ0 == 0) {
            float_raise(status, float_flag_invalid);
            return 0xffffffffffffffffULL;
        }
        absZ0 &= ~(((LARGE)(absZ1 << 1) == 0) & roundNearestEven);
    }
    if (zSign && absZ0) {
        float_raise(status, float_flag_invalid);
        return 0xffffffffffffffffULL;
    }
    if (absZ1) {
        float_raise(status, float_flag_inexact);
    }
    return absZ0;
}
void CpuInternal::normalizeFloat16Subnormal(WORD aSig, int16_t *zExpPtr, WORD *zSigPtr)
{
    int shiftCount = countLeadingZeros16(aSig) - 5;
    *zSigPtr       = aSig << shiftCount;
    *zExpPtr       = 1 - shiftCount;
}
float16 CpuInternal::roundAndPackFloat16(int zSign, int16_t zExp, WORD zSig, float_status_t *status)
{
    int16_t roundIncrement, roundBits, roundMask;
    int     roundingMode     = get_float_rounding_mode(status);
    int     roundNearestEven = (roundingMode == float_round_nearest_even);
    roundIncrement           = 8;
    roundMask                = 0xF;
    if (!roundNearestEven) {
        if (roundingMode == float_round_to_zero)
            roundIncrement = 0;
        else {
            roundIncrement = roundMask;
            if (zSign) {
                if (roundingMode == float_round_up)
                    roundIncrement = 0;
            } else {
                if (roundingMode == float_round_down)
                    roundIncrement = 0;
            }
        }
    }
    roundBits = zSig & roundMask;
    if (0x1D <= (WORD)zExp) {
        if ((0x1D < zExp) || ((zExp == 0x1D) && ((int16_t)(zSig + roundIncrement) < 0))) {
            float_raise(status, float_flag_overflow);
            if (roundBits || float_exception_masked(status, float_flag_overflow)) {
                float_raise(status, float_flag_inexact);
            }
            return packFloat16(zSign, 0x1F, 0) - (roundIncrement == 0);
        }
        if (zExp < 0) {
            int isTiny = (zExp < -1) || (zSig + roundIncrement < 0x8000);
            zSig       = shift16RightJamming(zSig, -zExp);
            zExp       = 0;
            roundBits  = zSig & roundMask;
            if (isTiny) {
                if (get_flush_underflow_to_zero(status)) {
                    float_raise(status, float_flag_underflow | float_flag_inexact);
                    return packFloat16(zSign, 0, 0);
                }
                if (roundBits || !float_exception_masked(status, float_flag_underflow)) {
                    float_raise(status, float_flag_underflow);
                }
            }
        }
    }
    if (roundBits)
        float_raise(status, float_flag_inexact);
    WORD zSigRound = ((zSig + roundIncrement) & ~roundMask) >> 4;
    zSigRound &= ~(((roundBits ^ 0x10) == 0) & roundNearestEven);
    if (zSigRound == 0)
        zExp = 0;
    return packFloat16(zSign, zExp, zSigRound);
}
void CpuInternal::normalizeFloat32Subnormal(DWORD aSig, int16_t *zExpPtr, DWORD *zSigPtr)
{
    int shiftCount = countLeadingZeros32(aSig) - 8;
    *zSigPtr       = aSig << shiftCount;
    *zExpPtr       = 1 - shiftCount;
}
float32 CpuInternal::roundAndPackFloat32(int zSign, int16_t zExp, DWORD zSig, float_status_t *status)
{
    int32_t       roundIncrement, roundBits;
    const int32_t roundMask        = 0x7F;
    int           roundingMode     = get_float_rounding_mode(status);
    int           roundNearestEven = (roundingMode == float_round_nearest_even);
    roundIncrement                 = 0x40;
    if (!roundNearestEven) {
        if (roundingMode == float_round_to_zero)
            roundIncrement = 0;
        else {
            roundIncrement = roundMask;
            if (zSign) {
                if (roundingMode == float_round_up)
                    roundIncrement = 0;
            } else {
                if (roundingMode == float_round_down)
                    roundIncrement = 0;
            }
        }
    }
    roundBits = zSig & roundMask;
    if (0xFD <= (WORD)zExp) {
        if ((0xFD < zExp) || ((zExp == 0xFD) && ((int32_t)(zSig + roundIncrement) < 0))) {
            float_raise(status, float_flag_overflow);
            if (roundBits || float_exception_masked(status, float_flag_overflow)) {
                float_raise(status, float_flag_inexact);
                if (roundIncrement != 0)
                    set_float_rounding_up(status);
            }
            return packFloat32(zSign, 0xFF, 0) - (roundIncrement == 0);
        }
        if (zExp < 0) {
            int isTiny = (zExp < -1) || (zSig + roundIncrement < 0x80000000);
            if (isTiny) {
                if (!float_exception_masked(status, float_flag_underflow)) {
                    float_raise(status, float_flag_underflow);
                    zExp += 192;
                }
            }
            if (zExp < 0) {
                zSig      = shift32RightJamming(zSig, -zExp);
                zExp      = 0;
                roundBits = zSig & roundMask;
                if (isTiny) {
                    if (get_flush_underflow_to_zero(status)) {
                        float_raise(status, float_flag_underflow | float_flag_inexact);
                        return packFloat32(zSign, 0, 0);
                    }
                    if (roundBits)
                        float_raise(status, float_flag_underflow);
                }
            }
        }
    }
    DWORD zSigRound = ((zSig + roundIncrement) & ~roundMask) >> 7;
    zSigRound &= ~(((roundBits ^ 0x40) == 0) & roundNearestEven);
    if (zSigRound == 0)
        zExp = 0;
    if (roundBits) {
        float_raise(status, float_flag_inexact);
        if ((zSigRound << 7) > zSig)
            set_float_rounding_up(status);
    }
    return packFloat32(zSign, zExp, zSigRound);
}
float32 CpuInternal::normalizeRoundAndPackFloat32(int zSign, int16_t zExp, DWORD zSig, float_status_t *status)
{
    int shiftCount = countLeadingZeros32(zSig) - 1;
    return roundAndPackFloat32(zSign, zExp - shiftCount, zSig << shiftCount, status);
}
void CpuInternal::normalizeFloat64Subnormal(LARGE aSig, int16_t *zExpPtr, LARGE *zSigPtr)
{
    int shiftCount = countLeadingZeros64(aSig) - 11;
    *zSigPtr       = aSig << shiftCount;
    *zExpPtr       = 1 - shiftCount;
}
float64 CpuInternal::roundAndPackFloat64(int zSign, int16_t zExp, LARGE zSig, float_status_t *status)
{
    int16_t       roundIncrement, roundBits;
    const int16_t roundMask        = 0x3FF;
    int           roundingMode     = get_float_rounding_mode(status);
    int           roundNearestEven = (roundingMode == float_round_nearest_even);
    roundIncrement                 = 0x200;
    if (!roundNearestEven) {
        if (roundingMode == float_round_to_zero)
            roundIncrement = 0;
        else {
            roundIncrement = roundMask;
            if (zSign) {
                if (roundingMode == float_round_up)
                    roundIncrement = 0;
            } else {
                if (roundingMode == float_round_down)
                    roundIncrement = 0;
            }
        }
    }
    roundBits = (int16_t)(zSig & roundMask);
    if (0x7FD <= (WORD)zExp) {
        if ((0x7FD < zExp) || ((zExp == 0x7FD) && ((int64_t)(zSig + roundIncrement) < 0))) {
            float_raise(status, float_flag_overflow);
            if (roundBits || float_exception_masked(status, float_flag_overflow)) {
                float_raise(status, float_flag_inexact);
                if (roundIncrement != 0)
                    set_float_rounding_up(status);
            }
            return packFloat64(zSign, 0x7FF, 0) - (roundIncrement == 0);
        }
        if (zExp < 0) {
            int isTiny = (zExp < -1) || (zSig + roundIncrement < 0x8000000000000000ULL);
            if (isTiny) {
                if (!float_exception_masked(status, float_flag_underflow)) {
                    float_raise(status, float_flag_underflow);
                    zExp += 1536;
                }
            }
            if (zExp < 0) {
                zSig      = shift64RightJamming(zSig, -zExp);
                zExp      = 0;
                roundBits = (int16_t)(zSig & roundMask);
                if (isTiny) {
                    if (get_flush_underflow_to_zero(status)) {
                        float_raise(status, float_flag_underflow | float_flag_inexact);
                        return packFloat64(zSign, 0, 0);
                    }
                    if (roundBits)
                        float_raise(status, float_flag_underflow);
                }
            }
        }
    }
    LARGE zSigRound = (zSig + roundIncrement) >> 10;
    zSigRound &= ~(((roundBits ^ 0x200) == 0) & roundNearestEven);
    if (zSigRound == 0)
        zExp = 0;
    if (roundBits) {
        float_raise(status, float_flag_inexact);
        if ((zSigRound << 10) > zSig)
            set_float_rounding_up(status);
    }
    return packFloat64(zSign, zExp, zSigRound);
}
float64 CpuInternal::normalizeRoundAndPackFloat64(int zSign, int16_t zExp, LARGE zSig, float_status_t *status)
{
    int shiftCount = countLeadingZeros64(zSig) - 1;
    return roundAndPackFloat64(zSign, zExp - shiftCount, zSig << shiftCount, status);
}
void CpuInternal::normalizeFloatx80Subnormal(LARGE aSig, int32_t *zExpPtr, LARGE *zSigPtr)
{
    int shiftCount = countLeadingZeros64(aSig);
    *zSigPtr       = aSig << shiftCount;
    *zExpPtr       = 1 - shiftCount;
}
floatx80 CpuInternal::SoftFloatRoundAndPackFloatx80(int roundingPrecision, int zSign, int32_t zExp, LARGE zSig0,
                                                    LARGE zSig1, float_status_t *status)
{
    LARGE roundIncrement, roundMask, roundBits;
    int      increment;
    LARGE zSigExact;
    BYTE  roundingMode     = get_float_rounding_mode(status);
    int      roundNearestEven = (roundingMode == float_round_nearest_even);
    if (roundingPrecision == 64) {
        roundIncrement = 0x0000000000000400ULL;
        roundMask      = 0x00000000000007FFULL;
    } else if (roundingPrecision == 32) {
        roundIncrement = 0x0000008000000000ULL;
        roundMask      = 0x000000FFFFFFFFFFULL;
    } else
        goto precision80;
    zSig0 |= (zSig1 != 0);
    if (!roundNearestEven) {
        if (roundingMode == float_round_to_zero)
            roundIncrement = 0;
        else {
            roundIncrement = roundMask;
            if (zSign) {
                if (roundingMode == float_round_up)
                    roundIncrement = 0;
            } else {
                if (roundingMode == float_round_down)
                    roundIncrement = 0;
            }
        }
    }
    roundBits = zSig0 & roundMask;
    if (0x7FFD <= (DWORD)(zExp - 1)) {
        if ((0x7FFE < zExp) || ((zExp == 0x7FFE) && (zSig0 + roundIncrement < zSig0))) {
            goto overflow;
        }
        if (zExp <= 0) {
            int isTiny = (zExp < 0) || (zSig0 <= zSig0 + roundIncrement);
            zSig0      = shift64RightJamming(zSig0, 1 - zExp);
            zSigExact  = zSig0;
            zExp       = 0;
            roundBits  = zSig0 & roundMask;
            if (isTiny) {
                if (roundBits || (zSig0 && !float_exception_masked(status, float_flag_underflow)))
                    float_raise(status, float_flag_underflow);
            }
            zSig0 += roundIncrement;
            if ((int64_t)zSig0 < 0)
                zExp = 1;
            roundIncrement = roundMask + 1;
            if (roundNearestEven && (roundBits << 1 == roundIncrement))
                roundMask |= roundIncrement;
            zSig0 &= ~roundMask;
            if (roundBits) {
                float_raise(status, float_flag_inexact);
                if (zSig0 > zSigExact)
                    set_float_rounding_up(status);
            }
            return packFloatx80(zSign, zExp, zSig0);
        }
    }
    if (roundBits)
        float_raise(status, float_flag_inexact);
    zSigExact = zSig0;
    zSig0 += roundIncrement;
    if (zSig0 < roundIncrement) {
        ++zExp;
        zSig0 = 0x8000000000000000ULL;
        zSigExact >>= 1;
    }
    roundIncrement = roundMask + 1;
    if (roundNearestEven && (roundBits << 1 == roundIncrement))
        roundMask |= roundIncrement;
    zSig0 &= ~roundMask;
    if (zSig0 > zSigExact)
        set_float_rounding_up(status);
    if (zSig0 == 0)
        zExp = 0;
    return packFloatx80(zSign, zExp, zSig0);
precision80:
    increment = ((int64_t)zSig1 < 0);
    if (!roundNearestEven) {
        if (roundingMode == float_round_to_zero)
            increment = 0;
        else {
            if (zSign) {
                increment = (roundingMode == float_round_down) && zSig1;
            } else {
                increment = (roundingMode == float_round_up) && zSig1;
            }
        }
    }
    if (0x7FFD <= (DWORD)(zExp - 1)) {
        if ((0x7FFE < zExp) || ((zExp == 0x7FFE) && (zSig0 == 0xFFFFFFFFFFFFFFFFULL) && increment)) {
            roundMask = 0;
        overflow:
            float_raise(status, float_flag_overflow | float_flag_inexact);
            if ((roundingMode == float_round_to_zero) || (zSign && (roundingMode == float_round_up)) ||
                (!zSign && (roundingMode == float_round_down))) {
                return packFloatx80(zSign, 0x7FFE, ~roundMask);
            }
            set_float_rounding_up(status);
            return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
        }
        if (zExp <= 0) {
            int isTiny = (zExp < 0) || (!increment) || (zSig0 < 0xFFFFFFFFFFFFFFFFULL);
            shift64ExtraRightJamming(zSig0, zSig1, 1 - zExp, &zSig0, &zSig1);
            zExp = 0;
            if (isTiny) {
                if (zSig1 || (zSig0 && !float_exception_masked(status, float_flag_underflow)))
                    float_raise(status, float_flag_underflow);
            }
            if (zSig1)
                float_raise(status, float_flag_inexact);
            if (roundNearestEven)
                increment = ((int64_t)zSig1 < 0);
            else {
                if (zSign) {
                    increment = (roundingMode == float_round_down) && zSig1;
                } else {
                    increment = (roundingMode == float_round_up) && zSig1;
                }
            }
            if (increment) {
                zSigExact = zSig0++;
                zSig0 &= ~(((LARGE)(zSig1 << 1) == 0) & roundNearestEven);
                if (zSig0 > zSigExact)
                    set_float_rounding_up(status);
                if ((int64_t)zSig0 < 0)
                    zExp = 1;
            }
            return packFloatx80(zSign, zExp, zSig0);
        }
    }
    if (zSig1)
        float_raise(status, float_flag_inexact);
    if (increment) {
        zSigExact = zSig0++;
        if (zSig0 == 0) {
            zExp++;
            zSig0 = 0x8000000000000000ULL;
            zSigExact >>= 1;
        } else {
            zSig0 &= ~(((LARGE)(zSig1 << 1) == 0) & roundNearestEven);
        }
        if (zSig0 > zSigExact)
            set_float_rounding_up(status);
    } else {
        if (zSig0 == 0)
            zExp = 0;
    }
    return packFloatx80(zSign, zExp, zSig0);
}
floatx80 CpuInternal::roundAndPackFloatx80(int roundingPrecision, int zSign, int32_t zExp, LARGE zSig0, LARGE zSig1,
                                           float_status_t *status)
{
    float_status_t round_status = *status;
    floatx80       result       = SoftFloatRoundAndPackFloatx80(roundingPrecision, zSign, zExp, zSig0, zSig1, status);
    if (status->float_exception_flags & ~status->float_exception_masks & float_flag_underflow) {
        *status = round_status;
        float_raise(status, float_flag_underflow);
        return SoftFloatRoundAndPackFloatx80(roundingPrecision, zSign, zExp + 0x6000, zSig0, zSig1, status);
    }
    if (status->float_exception_flags & ~status->float_exception_masks & float_flag_overflow) {
        *status = round_status;
        float_raise(status, float_flag_overflow);
        return SoftFloatRoundAndPackFloatx80(roundingPrecision, zSign, zExp - 0x6000, zSig0, zSig1, status);
    }
    return result;
}
floatx80 CpuInternal::normalizeRoundAndPackFloatx80(int roundingPrecision, int zSign, int32_t zExp, LARGE zSig0,
                                                    LARGE zSig1, float_status_t *status)
{
    if (zSig0 == 0) {
        zSig0 = zSig1;
        zSig1 = 0;
        zExp -= 64;
    }
    int shiftCount = countLeadingZeros64(zSig0);
    shortShift128Left(zSig0, zSig1, shiftCount, &zSig0, &zSig1);
    zExp -= shiftCount;
    return roundAndPackFloatx80(roundingPrecision, zSign, zExp, zSig0, zSig1, status);
}
void CpuInternal::normalizeFloat128Subnormal(LARGE aSig0, LARGE aSig1, int32_t *zExpPtr, LARGE *zSig0Ptr,
                                             LARGE *zSig1Ptr)
{
    int shiftCount;
    if (aSig0 == 0) {
        shiftCount = countLeadingZeros64(aSig1) - 15;
        if (shiftCount < 0) {
            *zSig0Ptr = aSig1 >> (-shiftCount);
            *zSig1Ptr = aSig1 << (shiftCount & 63);
        } else {
            *zSig0Ptr = aSig1 << shiftCount;
            *zSig1Ptr = 0;
        }
        *zExpPtr = -shiftCount - 63;
    } else {
        shiftCount = countLeadingZeros64(aSig0) - 15;
        shortShift128Left(aSig0, aSig1, shiftCount, zSig0Ptr, zSig1Ptr);
        *zExpPtr = 1 - shiftCount;
    }
}
float128 CpuInternal::roundAndPackFloat128(int zSign, int32_t zExp, LARGE zSig0, LARGE zSig1, LARGE zSig2,
                                           float_status_t *status)
{
    int increment = ((int64_t)zSig2 < 0);
    if (0x7FFD <= (DWORD)zExp) {
        if ((0x7FFD < zExp) ||
            ((zExp == 0x7FFD) && eq128(0x0001FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, zSig0, zSig1) && increment)) {
            float_raise(status, float_flag_overflow | float_flag_inexact);
            return packFloat128(zSign, 0x7FFF, 0, 0);
        }
        if (zExp < 0) {
            int isTiny = (zExp < -1) || !increment || lt128(zSig0, zSig1, 0x0001FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL);
            shift128ExtraRightJamming(zSig0, zSig1, zSig2, -zExp, &zSig0, &zSig1, &zSig2);
            zExp = 0;
            if (isTiny && zSig2)
                float_raise(status, float_flag_underflow);
            increment = ((int64_t)zSig2 < 0);
        }
    }
    if (zSig2)
        float_raise(status, float_flag_inexact);
    if (increment) {
        add128(zSig0, zSig1, 0, 1, &zSig0, &zSig1);
        zSig1 &= ~((zSig2 + zSig2 == 0) & 1);
    } else {
        if ((zSig0 | zSig1) == 0)
            zExp = 0;
    }
    return packFloat128(zSign, zExp, zSig0, zSig1);
}
float128 CpuInternal::normalizeRoundAndPackFloat128(int zSign, int32_t zExp, LARGE zSig0, LARGE zSig1,
                                                    float_status_t *status)
{
    LARGE zSig2;
    if (zSig0 == 0) {
        zSig0 = zSig1;
        zSig1 = 0;
        zExp -= 64;
    }
    int shiftCount = countLeadingZeros64(zSig0) - 15;
    if (0 <= shiftCount) {
        zSig2 = 0;
        shortShift128Left(zSig0, zSig1, shiftCount, &zSig0, &zSig1);
    } else {
        shift128ExtraRightJamming(zSig0, zSig1, 0, -shiftCount, &zSig0, &zSig1, &zSig2);
    }
    zExp -= shiftCount;
    return roundAndPackFloat128(zSign, zExp, zSig0, zSig1, zSig2, status);
}
float32 CpuInternal::propagateFloat32MulAddNaN(float32 a, float32 b, float32 c, float_status_t *status)
{
    int aIsNaN          = float32_is_nan(a);
    int bIsNaN          = float32_is_nan(b);
    int aIsSignalingNaN = float32_is_signaling_nan(a);
    int bIsSignalingNaN = float32_is_signaling_nan(b);
    int cIsSignalingNaN = float32_is_signaling_nan(c);
    a |= 0x00400000;
    b |= 0x00400000;
    c |= 0x00400000;
    if (aIsSignalingNaN | bIsSignalingNaN | cIsSignalingNaN)
        float_raise(status, float_flag_invalid);
    if (aIsSignalingNaN | aIsNaN) {
        return a;
    } else {
        return (bIsSignalingNaN | bIsNaN) ? b : c;
    }
}
float64 CpuInternal::propagateFloat64MulAddNaN(float64 a, float64 b, float64 c, float_status_t *status)
{
    int aIsNaN          = float64_is_nan(a);
    int bIsNaN          = float64_is_nan(b);
    int aIsSignalingNaN = float64_is_signaling_nan(a);
    int bIsSignalingNaN = float64_is_signaling_nan(b);
    int cIsSignalingNaN = float64_is_signaling_nan(c);
    a |= 0x0008000000000000ULL;
    b |= 0x0008000000000000ULL;
    c |= 0x0008000000000000ULL;
    if (aIsSignalingNaN | bIsSignalingNaN | cIsSignalingNaN)
        float_raise(status, float_flag_invalid);
    if (aIsSignalingNaN | aIsNaN) {
        return a;
    } else {
        return (bIsSignalingNaN | bIsNaN) ? b : c;
    }
}
float32 CpuInternal::float32_muladd(float32 a, float32 b, float32 c, int flags, float_status_t *status)
{
    int      aSign, bSign, cSign, zSign;
    int16_t  aExp, bExp, cExp, pExp, zExp;
    DWORD aSig, bSig, cSig;
    int      pInf, pZero, pSign;
    LARGE pSig64, cSig64, zSig64;
    DWORD pSig;
    int      shiftcount;
    aSig  = extractFloat32Frac(a);
    aExp  = extractFloat32Exp(a);
    aSign = extractFloat32Sign(a);
    bSig  = extractFloat32Frac(b);
    bExp  = extractFloat32Exp(b);
    bSign = extractFloat32Sign(b);
    cSig  = extractFloat32Frac(c);
    cExp  = extractFloat32Exp(c);
    cSign = extractFloat32Sign(c);
    if (((aExp == 0xff) && aSig) || ((bExp == 0xff) && bSig) || ((cExp == 0xff) && cSig)) {
        return propagateFloat32MulAddNaN(a, b, c, status);
    }
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
        if (cExp == 0)
            cSig = 0;
    }
    int infzero = ((aExp == 0 && aSig == 0 && bExp == 0xff && bSig == 0) ||
                   (aExp == 0xff && aSig == 0 && bExp == 0 && bSig == 0));
    if (infzero) {
        float_raise(status, float_flag_invalid);
        return float32_default_nan;
    }
    if (flags & float_muladd_negate_c) {
        cSign ^= 1;
    }
    pSign = aSign ^ bSign;
    if (flags & float_muladd_negate_product) {
        pSign ^= 1;
    }
    pInf  = (aExp == 0xff) || (bExp == 0xff);
    pZero = ((aExp | aSig) == 0) || ((bExp | bSig) == 0);
    if (cExp == 0xff) {
        if (pInf && (pSign ^ cSign)) {
            float_raise(status, float_flag_invalid);
            return float32_default_nan;
        }
        if ((aSig && aExp == 0) || (bSig && bExp == 0)) {
            float_raise(status, float_flag_denormal);
        }
        return packFloat32(cSign, 0xff, 0);
    }
    if (pInf) {
        if ((aSig && aExp == 0) || (bSig && bExp == 0) || (cSig && cExp == 0)) {
            float_raise(status, float_flag_denormal);
        }
        return packFloat32(pSign, 0xff, 0);
    }
    if (pZero) {
        if (cExp == 0) {
            if (cSig == 0) {
                if (pSign == cSign) {
                    zSign = pSign;
                } else if (get_float_rounding_mode(status) == float_round_down) {
                    zSign = 1;
                } else {
                    zSign = 0;
                }
                return packFloat32(zSign, 0, 0);
            }
            float_raise(status, float_flag_denormal);
            if (get_flush_underflow_to_zero(status)) {
                float_raise(status, float_flag_underflow | float_flag_inexact);
                return packFloat32(cSign, 0, 0);
            }
        }
        return packFloat32(cSign, cExp, cSig);
    }
    if (aExp == 0) {
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(bSig, &bExp, &bSig);
    }
    pExp   = aExp + bExp - 0x7e;
    aSig   = (aSig | 0x00800000) << 7;
    bSig   = (bSig | 0x00800000) << 8;
    pSig64 = (LARGE)aSig * bSig;
    if ((int64_t)(pSig64 << 1) >= 0) {
        pSig64 <<= 1;
        pExp--;
    }
    zSign = pSign;
    if (cExp == 0) {
        if (!cSig) {
            pSig = (DWORD)shift64RightJamming(pSig64, 32);
            return roundAndPackFloat32(zSign, pExp - 1, pSig, status);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloat32Subnormal(cSig, &cExp, &cSig);
    }
    cSig64 = (LARGE)cSig << 39;
    cSig64 |= 0x4000000000000000ULL;
    int expDiff = pExp - cExp;
    if (pSign == cSign) {
        if (expDiff > 0) {
            cSig64 = shift64RightJamming(cSig64, expDiff);
            zExp   = pExp;
        } else if (expDiff < 0) {
            pSig64 = shift64RightJamming(pSig64, -expDiff);
            zExp   = cExp;
        } else {
            zExp = cExp;
        }
        zSig64 = pSig64 + cSig64;
        if ((int64_t)zSig64 < 0) {
            zSig64 = shift64RightJamming(zSig64, 1);
        } else {
            zExp--;
        }
        zSig64 = shift64RightJamming(zSig64, 32);
        return roundAndPackFloat32(zSign, zExp, zSig64, status);
    } else {
        if (expDiff > 0) {
            cSig64 = shift64RightJamming(cSig64, expDiff);
            zSig64 = pSig64 - cSig64;
            zExp   = pExp;
        } else if (expDiff < 0) {
            pSig64 = shift64RightJamming(pSig64, -expDiff);
            zSig64 = cSig64 - pSig64;
            zExp   = cExp;
            zSign ^= 1;
        } else {
            zExp = pExp;
            if (cSig64 < pSig64) {
                zSig64 = pSig64 - cSig64;
            } else if (pSig64 < cSig64) {
                zSig64 = cSig64 - pSig64;
                zSign ^= 1;
            } else {
                return packFloat32(get_float_rounding_mode(status) == float_round_down, 0, 0);
            }
        }
        --zExp;
        shiftcount = countLeadingZeros64(zSig64) - 1;
        zSig64 <<= shiftcount;
        zExp -= shiftcount;
        zSig64 = shift64RightJamming(zSig64, 32);
        return roundAndPackFloat32(zSign, zExp, zSig64, status);
    }
}
float64 CpuInternal::float64_muladd(float64 a, float64 b, float64 c, int flags, float_status_t *status)
{
    int      aSign, bSign, cSign, zSign;
    int16_t  aExp, bExp, cExp, pExp, zExp;
    LARGE aSig, bSig, cSig;
    int      pInf, pZero, pSign;
    LARGE pSig0, pSig1, cSig0, cSig1, zSig0, zSig1;
    int      shiftcount;
    aSig  = extractFloat64Frac(a);
    aExp  = extractFloat64Exp(a);
    aSign = extractFloat64Sign(a);
    bSig  = extractFloat64Frac(b);
    bExp  = extractFloat64Exp(b);
    bSign = extractFloat64Sign(b);
    cSig  = extractFloat64Frac(c);
    cExp  = extractFloat64Exp(c);
    cSign = extractFloat64Sign(c);
    if (((aExp == 0x7ff) && aSig) || ((bExp == 0x7ff) && bSig) || ((cExp == 0x7ff) && cSig)) {
        return propagateFloat64MulAddNaN(a, b, c, status);
    }
    if (get_denormals_are_zeros(status)) {
        if (aExp == 0)
            aSig = 0;
        if (bExp == 0)
            bSig = 0;
        if (cExp == 0)
            cSig = 0;
    }
    int infzero = ((aExp == 0 && aSig == 0 && bExp == 0x7ff && bSig == 0) ||
                   (aExp == 0x7ff && aSig == 0 && bExp == 0 && bSig == 0));
    if (infzero) {
        float_raise(status, float_flag_invalid);
        return float64_default_nan;
    }
    if (flags & float_muladd_negate_c) {
        cSign ^= 1;
    }
    pSign = aSign ^ bSign;
    if (flags & float_muladd_negate_product) {
        pSign ^= 1;
    }
    pInf  = (aExp == 0x7ff) || (bExp == 0x7ff);
    pZero = ((aExp | aSig) == 0) || ((bExp | bSig) == 0);
    if (cExp == 0x7ff) {
        if (pInf && (pSign ^ cSign)) {
            float_raise(status, float_flag_invalid);
            return float64_default_nan;
        }
        if ((aSig && aExp == 0) || (bSig && bExp == 0)) {
            float_raise(status, float_flag_denormal);
        }
        return packFloat64(cSign, 0x7ff, 0);
    }
    if (pInf) {
        if ((aSig && aExp == 0) || (bSig && bExp == 0) || (cSig && cExp == 0)) {
            float_raise(status, float_flag_denormal);
        }
        return packFloat64(pSign, 0x7ff, 0);
    }
    if (pZero) {
        if (cExp == 0) {
            if (cSig == 0) {
                if (pSign == cSign) {
                    zSign = pSign;
                } else if (get_float_rounding_mode(status) == float_round_down) {
                    zSign = 1;
                } else {
                    zSign = 0;
                }
                return packFloat64(zSign, 0, 0);
            }
            float_raise(status, float_flag_denormal);
            if (get_flush_underflow_to_zero(status)) {
                float_raise(status, float_flag_underflow | float_flag_inexact);
                return packFloat64(cSign, 0, 0);
            }
        }
        return packFloat64(cSign, cExp, cSig);
    }
    if (aExp == 0) {
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(bSig, &bExp, &bSig);
    }
    pExp = aExp + bExp - 0x3fe;
    aSig = (aSig | 0x0010000000000000ULL) << 10;
    bSig = (bSig | 0x0010000000000000ULL) << 11;
    mul64To128(aSig, bSig, &pSig0, &pSig1);
    if ((int64_t)(pSig0 << 1) >= 0) {
        shortShift128Left(pSig0, pSig1, 1, &pSig0, &pSig1);
        pExp--;
    }
    zSign = pSign;
    if (cExp == 0) {
        if (!cSig) {
            shift128RightJamming(pSig0, pSig1, 64, &pSig0, &pSig1);
            return roundAndPackFloat64(zSign, pExp - 1, pSig1, status);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloat64Subnormal(cSig, &cExp, &cSig);
    }
    cSig0 = cSig << 10;
    cSig1 = 0;
    cSig0 |= 0x4000000000000000ULL;
    int expDiff = pExp - cExp;
    if (pSign == cSign) {
        if (expDiff > 0) {
            shift128RightJamming(cSig0, cSig1, expDiff, &cSig0, &cSig1);
            zExp = pExp;
        } else if (expDiff < 0) {
            shift128RightJamming(pSig0, pSig1, -expDiff, &pSig0, &pSig1);
            zExp = cExp;
        } else {
            zExp = cExp;
        }
        add128(pSig0, pSig1, cSig0, cSig1, &zSig0, &zSig1);
        if ((int64_t)zSig0 < 0) {
            shift128RightJamming(zSig0, zSig1, 1, &zSig0, &zSig1);
        } else {
            zExp--;
        }
        shift128RightJamming(zSig0, zSig1, 64, &zSig0, &zSig1);
        return roundAndPackFloat64(zSign, zExp, zSig1, status);
    } else {
        if (expDiff > 0) {
            shift128RightJamming(cSig0, cSig1, expDiff, &cSig0, &cSig1);
            sub128(pSig0, pSig1, cSig0, cSig1, &zSig0, &zSig1);
            zExp = pExp;
        } else if (expDiff < 0) {
            shift128RightJamming(pSig0, pSig1, -expDiff, &pSig0, &pSig1);
            sub128(cSig0, cSig1, pSig0, pSig1, &zSig0, &zSig1);
            zExp = cExp;
            zSign ^= 1;
        } else {
            zExp = pExp;
            if (lt128(cSig0, cSig1, pSig0, pSig1)) {
                sub128(pSig0, pSig1, cSig0, cSig1, &zSig0, &zSig1);
            } else if (lt128(pSig0, pSig1, cSig0, cSig1)) {
                sub128(cSig0, cSig1, pSig0, pSig1, &zSig0, &zSig1);
                zSign ^= 1;
            } else {
                return packFloat64(get_float_rounding_mode(status) == float_round_down, 0, 0);
            }
        }
        --zExp;
        if (zSig0) {
            shiftcount = countLeadingZeros64(zSig0) - 1;
            shortShift128Left(zSig0, zSig1, shiftcount, &zSig0, &zSig1);
            if (zSig1) {
                zSig0 |= 1;
            }
            zExp -= shiftcount;
        } else {
            shiftcount = countLeadingZeros64(zSig1) - 1;
            zSig0      = zSig1 << shiftcount;
            zExp -= (shiftcount + 64);
        }
        return roundAndPackFloat64(zSign, zExp, zSig0, status);
    }
}
float128 CpuInternal::EvalPoly(float128 x, float128 *arr, int n, float_status_t *status)
{
    float128 r = arr[--n];
    do {
        r = float128_mul(r, x, status);
        r = float128_add(r, arr[--n], status);
    } while (n > 0);
    return r;
}
float128 CpuInternal::EvenPoly(float128 x, float128 *arr, int n, float_status_t *status)
{
    return EvalPoly(float128_mul(x, x, status), arr, n, status);
}
float128 CpuInternal::OddPoly(float128 x, float128 *arr, int n, float_status_t *status)
{
    return float128_mul(x, EvenPoly(x, arr, n, status), status);
}
float128 CpuInternal::poly_ln(float128 x1, float_status_t *status)
{
    return OddPoly(x1, ln_arr, 9, status);
}
float128 CpuInternal::poly_l2(float128 x, float_status_t *status)
{
    float128 x_p1 = float128_add(x, float128_one, status);
    float128 x_m1 = float128_sub(x, float128_one, status);
    x             = float128_div(x_m1, x_p1, status);
    x             = poly_ln(x, status);
    x             = float128_mul(x, float128_ln2inv2, status);
    return x;
}
float128 CpuInternal::poly_l2p1(float128 x, float_status_t *status)
{
    float128 x_p2 = float128_add(x, float128_two, status);
    x             = float128_div(x, x_p2, status);
    x             = poly_ln(x, status);
    x             = float128_mul(x, float128_ln2inv2, status);
    return x;
}
floatx80 CpuInternal::fyl2x(floatx80 a, floatx80 b, float_status_t *status)
{
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
    invalid:
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    LARGE aSig  = extractFloatx80Frac(a);
    int32_t  aExp  = extractFloatx80Exp(a);
    int      aSign = extractFloatx80Sign(a);
    LARGE bSig  = extractFloatx80Frac(b);
    int32_t  bExp  = extractFloatx80Exp(b);
    int      bSign = extractFloatx80Sign(b);
    int      zSign = bSign ^ 1;
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1) || ((bExp == 0x7FFF) && (LARGE)(bSig << 1))) {
            return propagateFloatx80NaN_two_args(a, b, status);
        }
        if (aSign)
            goto invalid;
        else {
            if (bExp == 0) {
                if (bSig == 0)
                    goto invalid;
                float_raise(status, float_flag_denormal);
            }
            return packFloatx80(bSign, 0x7FFF, 0x8000000000000000ULL);
        }
    }
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (aSign && (LARGE)(aExp | aSig))
            goto invalid;
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        if (aExp < 0x3FFF) {
            return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
        }
        if (aExp == 0x3FFF && ((LARGE)(aSig << 1) == 0))
            goto invalid;
        return packFloatx80(bSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if ((bExp | bSig) == 0)
                goto invalid;
            float_raise(status, float_flag_divbyzero);
            return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
        }
        if (aSign)
            goto invalid;
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    if (aSign)
        goto invalid;
    if (bExp == 0) {
        if (bSig == 0) {
            if (aExp < 0x3FFF)
                return packFloatx80(zSign, 0, 0);
            return packFloatx80(bSign, 0, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    if (aExp == 0x3FFF && ((LARGE)(aSig << 1) == 0))
        return packFloatx80(bSign, 0, 0);
    float_raise(status, float_flag_inexact);
    int ExpDiff = aExp - 0x3FFF;
    aExp        = 0;
    if (aSig >= 0xb504f333f9de6484ULL) {
        ExpDiff++;
        aExp--;
    }
    LARGE zSig0, zSig1;
    shift128Right(aSig << 1, 0, 16, &zSig0, &zSig1);
    float128 x = packFloat128(0, aExp + 0x3FFF, zSig0, zSig1);
    x          = poly_l2(x, status);
    x          = float128_add(x, int64_to_float128((int64_t)ExpDiff), status);
    return floatx80_mul_with_float128(b, x, status);
}
floatx80 CpuInternal::fyl2xp1(floatx80 a, floatx80 b, float_status_t *status)
{
    int32_t  aExp, bExp;
    LARGE aSig, bSig, zSig0, zSig1, zSig2;
    int      aSign, bSign;
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
    invalid:
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    aSig      = extractFloatx80Frac(a);
    aExp      = extractFloatx80Exp(a);
    aSign     = extractFloatx80Sign(a);
    bSig      = extractFloatx80Frac(b);
    bExp      = extractFloatx80Exp(b);
    bSign     = extractFloatx80Sign(b);
    int zSign = aSign ^ bSign;
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1) || ((bExp == 0x7FFF) && (LARGE)(bSig << 1))) {
            return propagateFloatx80NaN_two_args(a, b, status);
        }
        if (aSign)
            goto invalid;
        else {
            if (bExp == 0) {
                if (bSig == 0)
                    goto invalid;
                float_raise(status, float_flag_denormal);
            }
            return packFloatx80(bSign, 0x7FFF, 0x8000000000000000ULL);
        }
    }
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (aExp == 0) {
            if (aSig == 0)
                goto invalid;
            float_raise(status, float_flag_denormal);
        }
        return packFloatx80(zSign, 0x7FFF, 0x8000000000000000ULL);
    }
    if (aExp == 0) {
        if (aSig == 0) {
            if (bSig && (bExp == 0))
                float_raise(status, float_flag_denormal);
            return packFloatx80(zSign, 0, 0);
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    if (bExp == 0) {
        if (bSig == 0)
            return packFloatx80(zSign, 0, 0);
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    float_raise(status, float_flag_inexact);
    if (aSign && aExp >= 0x3FFF)
        return a;
    if (aExp >= 0x3FFC) {
        return fyl2x(floatx80_add(a, floatx80_one, status), b, status);
    }
    if (aExp < 0x3FFF - 70) {
        int32_t zExp = aExp + (0x3FFF) - 0x3FFE;
        mul128By64To192((0xb8aa3b295c17f0bbULL), (0xC000000000000000ULL), aSig, &zSig0, &zSig1, &zSig2);
        if (0 < (int64_t)zSig0) {
            shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
            --zExp;
        }
        zExp = zExp + bExp - 0x3FFE;
        mul128By64To192(zSig0, zSig1, bSig, &zSig0, &zSig1, &zSig2);
        if (0 < (int64_t)zSig0) {
            shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
            --zExp;
        }
        return roundAndPackFloatx80(80, aSign ^ bSign, zExp, zSig0, zSig1, status);
    }
    shift128Right(aSig << 1, 0, 16, &zSig0, &zSig1);
    float128 x = packFloat128(aSign, aExp, zSig0, zSig1);
    x          = poly_l2p1(x, status);
    return floatx80_mul_with_float128(b, x, status);
}
LARGE CpuInternal::argument_reduction_kernel(LARGE aSig0, int Exp, LARGE *zSig0, LARGE *zSig1)
{
    LARGE term0, term1, term2;
    LARGE aSig1 = 0;
    shortShift128Left(aSig1, aSig0, Exp, &aSig1, &aSig0);
    LARGE q = estimateDiv128To64(aSig1, aSig0, (0xc90fdaa22168c234ULL));
    mul128By64To192((0xc90fdaa22168c234ULL), (0xC000000000000000ULL), q, &term0, &term1, &term2);
    sub128(aSig1, aSig0, term0, term1, zSig1, zSig0);
    while ((int64_t)(*zSig1) < 0) {
        --q;
        add192(*zSig1, *zSig0, term2, 0, (0xc90fdaa22168c234ULL), (0xC000000000000000ULL), zSig1, zSig0, &term2);
    }
    *zSig1 = term2;
    return q;
}
int CpuInternal::reduce_trig_arg(int expDiff, int *zSign_input, LARGE *aSig0_input, LARGE *aSig1_input)
{
    int      zSign = *zSign_input;
    LARGE aSig0 = *aSig0_input, aSig1 = *aSig1_input;
    LARGE term0, term1, q = 0;
    if (expDiff < 0) {
        shift128Right(aSig0, 0, 1, &aSig0, &aSig1);
        expDiff = 0;
    }
    if (expDiff > 0) {
        q = argument_reduction_kernel(aSig0, expDiff, &aSig0, &aSig1);
    } else {
        if ((0xc90fdaa22168c234ULL) <= aSig0) {
            aSig0 -= (0xc90fdaa22168c234ULL);
            q = 1;
        }
    }
    shift128Right((0xc90fdaa22168c234ULL), (0xC000000000000000ULL), 1, &term0, &term1);
    if (!lt128(aSig0, aSig1, term0, term1)) {
        int lt = lt128(term0, term1, aSig0, aSig1);
        int eq = eq128(aSig0, aSig1, term0, term1);
        if ((eq && (q & 1)) || lt) {
            zSign = !zSign;
            ++q;
        }
        if (lt)
            sub128((0xc90fdaa22168c234ULL), (0xC000000000000000ULL), aSig0, aSig1, &aSig0, &aSig1);
    }
    *zSign_input = zSign;
    *aSig0_input = aSig0;
    *aSig1_input = aSig1;
    return (int)(q & 3);
}
float128 CpuInternal::poly_sin(float128 x, float_status_t *status)
{
    return OddPoly(x, sin_arr, 11, status);
}
float128 CpuInternal::poly_cos(float128 x, float_status_t *status)
{
    return EvenPoly(x, cos_arr, 11, status);
}
void CpuInternal::sincos_invalid(floatx80 *sin_a, floatx80 *cos_a, floatx80 a)
{
    if (sin_a)
        *sin_a = a;
    if (cos_a)
        *cos_a = a;
}
void CpuInternal::sincos_tiny_argument(floatx80 *sin_a, floatx80 *cos_a, floatx80 a)
{
    if (sin_a)
        *sin_a = a;
    if (cos_a)
        *cos_a = floatx80_one;
}
floatx80 CpuInternal::sincos_approximation(int neg, float128 r, LARGE quotient, float_status_t *status)
{
    if (quotient & 0x1) {
        r   = poly_cos(r, status);
        neg = 0;
    } else {
        r = poly_sin(r, status);
    }
    floatx80 result = float128_to_floatx80(r, status);
    if (quotient & 0x2)
        neg = !neg;
    if (neg)
        floatx80_chs(&result);
    return result;
}
int CpuInternal::fsincos(floatx80 a, floatx80 *sin_a, floatx80 *cos_a, float_status_t *status)
{
    LARGE aSig0, aSig1 = 0;
    int32_t  aExp, zExp, expDiff;
    int      aSign, zSign;
    int      q = 0;
    if (floatx80_is_unsupported(a)) {
        goto invalid;
    }
    aSig0 = extractFloatx80Frac(a);
    aExp  = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig0 << 1)) {
            sincos_invalid(sin_a, cos_a, propagateFloatx80NaN(a, status));
            return 0;
        }
    invalid:
        float_raise(status, float_flag_invalid);
        sincos_invalid(sin_a, cos_a, floatx80_default_nan);
        return 0;
    }
    if (aExp == 0) {
        if (aSig0 == 0) {
            sincos_tiny_argument(sin_a, cos_a, a);
            return 0;
        }
        float_raise(status, float_flag_denormal);
        if (!(aSig0 & 0x8000000000000000ULL)) {
            float_raise(status, float_flag_inexact);
            if (sin_a)
                float_raise(status, float_flag_underflow);
            sincos_tiny_argument(sin_a, cos_a, a);
            return 0;
        }
        normalizeFloatx80Subnormal(aSig0, &aExp, &aSig0);
    }
    zSign   = aSign;
    zExp    = 0x3FFF;
    expDiff = aExp - zExp;
    if (expDiff >= 63)
        return -1;
    float_raise(status, float_flag_inexact);
    if (expDiff < -1) {
        if (expDiff <= -68) {
            a = packFloatx80(aSign, aExp, aSig0);
            sincos_tiny_argument(sin_a, cos_a, a);
            return 0;
        }
        zExp = aExp;
    } else {
        q = reduce_trig_arg(expDiff, &zSign, &aSig0, &aSig1);
    }
    float128 r = normalizeRoundAndPackFloat128(0, zExp - 0x10, aSig0, aSig1, status);
    if (aSign)
        q = -q;
    if (sin_a)
        *sin_a = sincos_approximation(zSign, r, q, status);
    if (cos_a)
        *cos_a = sincos_approximation(zSign, r, q + 1, status);
    return 0;
}
int CpuInternal::fsin(floatx80 *a, float_status_t *status)
{
    return fsincos(*a, a, 0, status);
}
int CpuInternal::fcos(floatx80 *a, float_status_t *status)
{
    return fsincos(*a, 0, a, status);
}
int CpuInternal::ftan(floatx80 *a_input, float_status_t *status)
{
    floatx80 a = *a_input;
    LARGE aSig0, aSig1 = 0;
    int32_t  aExp, zExp, expDiff;
    int      aSign, zSign;
    int      q = 0;
    if (floatx80_is_unsupported(a)) {
        goto invalid;
    }
    aSig0 = extractFloatx80Frac(a);
    aExp  = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig0 << 1)) {
            a        = propagateFloatx80NaN(a, status);
            *a_input = a;
            return 0;
        }
    invalid:
        float_raise(status, float_flag_invalid);
        a        = floatx80_default_nan;
        *a_input = a;
        return 0;
    }
    if (aExp == 0) {
        if (aSig0 == 0)
            return 0;
        float_raise(status, float_flag_denormal);
        if (!(aSig0 & 0x8000000000000000ULL)) {
            float_raise(status, float_flag_inexact | float_flag_underflow);
            *a_input = a;
            return 0;
        }
        normalizeFloatx80Subnormal(aSig0, &aExp, &aSig0);
    }
    zSign   = aSign;
    zExp    = 0x3FFF;
    expDiff = aExp - zExp;
    if (expDiff >= 63) {
        *a_input = a;
        return -1;
    }
    float_raise(status, float_flag_inexact);
    if (expDiff < -1) {
        if (expDiff <= -68) {
            a        = packFloatx80(aSign, aExp, aSig0);
            *a_input = a;
            return 0;
        }
        zExp = aExp;
    } else {
        q = reduce_trig_arg(expDiff, &zSign, &aSig0, &aSig1);
    }
    float128 r     = normalizeRoundAndPackFloat128(0, zExp - 0x10, aSig0, aSig1, status);
    float128 sin_r = poly_sin(r, status);
    float128 cos_r = poly_cos(r, status);
    if (q & 0x1) {
        r     = float128_div(cos_r, sin_r, status);
        zSign = !zSign;
    } else {
        r = float128_div(sin_r, cos_r, status);
    }
    a = float128_to_floatx80(r, status);
    if (zSign)
        floatx80_chs(&a);
    *a_input = a;
    return 0;
}
LARGE CpuInternal::remainder_kernel(LARGE aSig0, LARGE bSig, int expDiff, LARGE *zSig0, LARGE *zSig1)
{
    LARGE term0, term1;
    LARGE aSig1 = 0;
    shortShift128Left(aSig1, aSig0, expDiff, &aSig1, &aSig0);
    LARGE q = estimateDiv128To64(aSig1, aSig0, bSig);
    mul64To128(bSig, q, &term0, &term1);
    sub128(aSig1, aSig0, term0, term1, zSig1, zSig0);
    while ((int64_t)(*zSig1) < 0) {
        --q;
        add128(*zSig1, *zSig0, 0, bSig, zSig1, zSig0);
    }
    return q;
}
int CpuInternal::do_fprem(floatx80 a, floatx80 b, floatx80 *r_input, LARGE *q_input, int rounding_mode,
                          float_status_t *status)
{
    floatx80 r = *r_input;
    LARGE q = *q_input;
    int32_t  aExp, bExp, zExp, expDiff;
    LARGE aSig0, aSig1, bSig;
    int      aSign;
    q = 0;
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
        float_raise(status, float_flag_invalid);
        r        = floatx80_default_nan;
        *r_input = r;
        *q_input = q;
        return -1;
    }
    aSig0 = extractFloatx80Frac(a);
    aExp  = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    bSig  = extractFloatx80Frac(b);
    bExp  = extractFloatx80Exp(b);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig0 << 1) || ((bExp == 0x7FFF) && (LARGE)(bSig << 1))) {
            r        = propagateFloatx80NaN_two_args(a, b, status);
            *r_input = r;
            *q_input = q;
            return -1;
        }
        float_raise(status, float_flag_invalid);
        r        = floatx80_default_nan;
        *r_input = r;
        *q_input = q;
        return -1;
    }
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1)) {
            r        = propagateFloatx80NaN_two_args(a, b, status);
            *r_input = r;
            *q_input = q;
            return -1;
        }
        if (aExp == 0 && aSig0) {
            float_raise(status, float_flag_denormal);
            normalizeFloatx80Subnormal(aSig0, &aExp, &aSig0);
            r        = (a.fraction & 0x8000000000000000ULL) ? packFloatx80(aSign, aExp, aSig0) : a;
            *r_input = r;
            *q_input = q;
            return 0;
        }
        r        = a;
        *r_input = r;
        *q_input = q;
        return 0;
    }
    if (bExp == 0) {
        if (bSig == 0) {
            float_raise(status, float_flag_invalid);
            r        = floatx80_default_nan;
            *r_input = r;
            *q_input = q;
            return -1;
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    if (aExp == 0) {
        if (aSig0 == 0) {
            r        = a;
            *r_input = r;
            *q_input = q;
            return 0;
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig0, &aExp, &aSig0);
    }
    expDiff           = aExp - bExp;
    aSig1             = 0;
    DWORD overflow = 0;
    if (expDiff >= 64) {
        int n = (expDiff & 0x1f) | 0x20;
        remainder_kernel(aSig0, bSig, n, &aSig0, &aSig1);
        zExp     = aExp - n;
        overflow = 1;
    } else {
        zExp = bExp;
        if (expDiff < 0) {
            if (expDiff < -1) {
                r        = (a.fraction & 0x8000000000000000ULL) ? packFloatx80(aSign, aExp, aSig0) : a;
                *r_input = r;
                *q_input = q;
                return 0;
            }
            shift128Right(aSig0, 0, 1, &aSig0, &aSig1);
            expDiff = 0;
        }
        if (expDiff > 0) {
            q = remainder_kernel(aSig0, bSig, expDiff, &aSig0, &aSig1);
        } else {
            if (bSig <= aSig0) {
                aSig0 -= bSig;
                q = 1;
            }
        }
        if (rounding_mode == float_round_nearest_even) {
            LARGE term0, term1;
            shift128Right(bSig, 0, 1, &term0, &term1);
            if (!lt128(aSig0, aSig1, term0, term1)) {
                int lt = lt128(term0, term1, aSig0, aSig1);
                int eq = eq128(aSig0, aSig1, term0, term1);
                if ((eq && (q & 1)) || lt) {
                    aSign = !aSign;
                    ++q;
                }
                if (lt)
                    sub128(bSig, 0, aSig0, aSig1, &aSig0, &aSig1);
            }
        }
    }
    r        = normalizeRoundAndPackFloatx80(80, aSign, zExp, aSig0, aSig1, status);
    *r_input = r;
    *q_input = q;
    return overflow;
}
int CpuInternal::floatx80_ieee754_remainder(floatx80 a, floatx80 b, floatx80 *r, LARGE *q, float_status_t *status)
{
    return do_fprem(a, b, r, q, float_round_nearest_even, status);
}
int CpuInternal::floatx80_remainder(floatx80 a, floatx80 b, floatx80 *r, LARGE *q, float_status_t *status)
{
    return do_fprem(a, b, r, q, float_round_to_zero, status);
}
float128 CpuInternal::poly_atan(float128 x1, float_status_t *status)
{
    return OddPoly(x1, atan_arr, 11, status);
}
floatx80 CpuInternal::fpatan(floatx80 a, floatx80 b, float_status_t *status)
{
    if (floatx80_is_unsupported(a) || floatx80_is_unsupported(b)) {
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    LARGE aSig  = extractFloatx80Frac(a);
    int32_t  aExp  = extractFloatx80Exp(a);
    int      aSign = extractFloatx80Sign(a);
    LARGE bSig  = extractFloatx80Frac(b);
    int32_t  bExp  = extractFloatx80Exp(b);
    int      bSign = extractFloatx80Sign(b);
    int      zSign = aSign ^ bSign;
    if (bExp == 0x7FFF) {
        if ((LARGE)(bSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (aExp == 0x7FFF) {
            if ((LARGE)(aSig << 1))
                return propagateFloatx80NaN_two_args(a, b, status);
            if (aSign) {
                return roundAndPackFloatx80(80, bSign, (0x4000), (0x96cbe3f9990e91a7ULL), (0x9000000000000000ULL),
                                            status);
            } else {
                return roundAndPackFloatx80(80, bSign, (0x3FFE), (0xc90fdaa22168c234ULL), (0xC000000000000000ULL),
                                            status);
            }
        }
        if (aSig && (aExp == 0))
            float_raise(status, float_flag_denormal);
        return roundAndPackFloatx80(80, bSign, (0x3FFF), (0xc90fdaa22168c234ULL), (0xC000000000000000ULL), status);
    }
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1))
            return propagateFloatx80NaN_two_args(a, b, status);
        if (bSig && (bExp == 0))
            float_raise(status, float_flag_denormal);
    return_PI_or_ZERO:
        if (aSign) {
            return roundAndPackFloatx80(80, bSign, (0x4000), (0xc90fdaa22168c234ULL), (0xC000000000000000ULL), status);
        } else {
            return packFloatx80(bSign, 0, 0);
        }
    }
    if (bExp == 0) {
        if (bSig == 0) {
            if (aSig && (aExp == 0))
                float_raise(status, float_flag_denormal);
            goto return_PI_or_ZERO;
        }
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(bSig, &bExp, &bSig);
    }
    if (aExp == 0) {
        if (aSig == 0)
            return roundAndPackFloatx80(80, bSign, (0x3FFF), (0xc90fdaa22168c234ULL), (0xC000000000000000ULL), status);
        float_raise(status, float_flag_denormal);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    }
    float_raise(status, float_flag_inexact);
    if (aSig == bSig && aExp == bExp)
        return roundAndPackFloatx80(80, bSign, (0x3FFE), (0xc90fdaa22168c234ULL), (0xC000000000000000ULL), status);
    float128 a128 = normalizeRoundAndPackFloat128(0, aExp - 0x10, aSig, 0, status);
    float128 b128 = normalizeRoundAndPackFloat128(0, bExp - 0x10, bSig, 0, status);
    float128 x;
    int      swap = 0, add_pi6 = 0, add_pi4 = 0;
    if (aExp > bExp || (aExp == bExp && aSig > bSig)) {
        x = float128_div(b128, a128, status);
    } else {
        x    = float128_div(a128, b128, status);
        swap = 1;
    }
    int32_t xExp = extractFloat128Exp(x);
    if (xExp <= 0x3FFF - 40)
        goto approximation_completed;
    if (x.hi >= 0x3ffe800000000000ULL) {
        float128 t1 = float128_sub(x, float128_one, status);
        float128 t2 = float128_add(x, float128_one, status);
        x           = float128_div(t1, t2, status);
        add_pi4     = 1;
    } else {
        if (xExp >= 0x3FFD) {
            float128 t1 = float128_mul(x, float128_sqrt3, status);
            float128 t2 = float128_add(x, float128_sqrt3, status);
            x           = float128_sub(t1, float128_one, status);
            x           = float128_div(x, t2, status);
            add_pi6     = 1;
        }
    }
    x = poly_atan(x, status);
    if (add_pi6)
        x = float128_add(x, float128_pi6, status);
    if (add_pi4)
        x = float128_add(x, float128_pi4, status);
approximation_completed:
    if (swap)
        x = float128_sub(float128_pi2, x, status);
    floatx80 result = float128_to_floatx80(x, status);
    if (zSign)
        floatx80_chs(&result);
    int rSign = extractFloatx80Sign(result);
    if (!bSign && rSign)
        return floatx80_add(result, floatx80_pi, status);
    if (bSign && !rSign)
        return floatx80_sub(result, floatx80_pi, status);
    return result;
}
float128 CpuInternal::poly_exp(float128 x, float_status_t *status)
{
    float128 t = EvalPoly(x, exp_arr, 15, status);
    return float128_mul(t, x, status);
}
floatx80 CpuInternal::f2xm1(floatx80 a, float_status_t *status)
{
    LARGE zSig0, zSig1, zSig2;
    if (floatx80_is_unsupported(a)) {
        float_raise(status, float_flag_invalid);
        return floatx80_default_nan;
    }
    LARGE aSig  = extractFloatx80Frac(a);
    int32_t  aExp  = extractFloatx80Exp(a);
    int      aSign = extractFloatx80Sign(a);
    if (aExp == 0x7FFF) {
        if ((LARGE)(aSig << 1))
            return propagateFloatx80NaN(a, status);
        return (aSign) ? floatx80_negone : a;
    }
    if (aExp == 0) {
        if (aSig == 0)
            return a;
        float_raise(status, float_flag_denormal | float_flag_inexact);
        normalizeFloatx80Subnormal(aSig, &aExp, &aSig);
    tiny_argument:
        mul128By64To192(0xb17217f7d1cf79abULL, 0xc000000000000000ULL, aSig, &zSig0, &zSig1, &zSig2);
        if (0 < (int64_t)zSig0) {
            shortShift128Left(zSig0, zSig1, 1, &zSig0, &zSig1);
            --aExp;
        }
        return roundAndPackFloatx80(80, aSign, aExp, zSig0, zSig1, status);
    }
    float_raise(status, float_flag_inexact);
    if (aExp < 0x3FFF) {
        if (aExp < 0x3FFF - 68)
            goto tiny_argument;
        float128 x = floatx80_to_float128(a, status);
        x          = float128_mul(x, float128_ln2, status);
        x          = poly_exp(x, status);
        return float128_to_floatx80(x, status);
    } else {
        if (a.exp == 0xBFFF && !(aSig << 1))
            return floatx80_neghalf;
        return a;
    }
}
