#pragma once

#include "platform.h"
#include <stdint.h>
#include <type_traits>
//----------------------------------------------------------------------------
// Define byte specific integers
//----------------------------------------------------------------------------
namespace tf
{
//----------------------------------------------------------------------------
// constLog2, constPower2 log2Floor, log2Ceil, and power2Next
//----------------------------------------------------------------------------
template <typename T>
constexpr T constLog2(T n) { return (n>1) ? 1 + constLog2(n>>1) : 0; }
template <typename T>
constexpr T constPower2(T n) { return (n>0) ? constPower2(n-1)<<1 : 1; }

#if defined (__GNUC__)
template <typename T>
TF_INLINE int log2Floor (uint64_t n) {return 63 - __builtin_clzl(uint64_t(n));}
TF_INLINE int log2Floor (uint32_t n) {return 32 - __builtin_clz(uint64_t(n));}

template <typename T>
TF_INLINE T power2Next (int n)
{
    return (n-2 <= 0 ? T(n) : (T(1) << 63) >> (__builtin_clz(n-1)-1));
}

#else

template <typename T>
TF_INLINE int log2Floor (T x)
{
  T i;
  int position =0;
  for ( i = (x >> 1); i != 0; ++position)
     i >>= 1;
  return position;
}

template <typename T>
TF_INLINE T power2Next (int n)
{
	T y = 1;
	if (n==0) return 0;
	// Keep doubling y until we are done
	while (y <= n) y += y;
	return y;
}
#endif

template <typename T>
TF_INLINE int log2Ceil (T n)
{
    return n > 1 ? log2Floor(n-1)+1 : 0;
}

//----------------------------------------------------------------------------
// constAlign align must be the value of 2, 4, 8, 16, 32, 64, ...
//----------------------------------------------------------------------------
template <typename T>
constexpr T constAlign(T n, size_t align) { return  n + (-n & (align-1));}

template <typename T>
constexpr const T* constAlign(const T* p, size_t align)
{ return (const T*) constAlign(intptr_t(p), align); }

template <typename T>
constexpr T* constAlign(T* p, size_t align)
 { return  (T*) constAlign(intptr_t(p), align); }

//----------------------------------------------------------------------------
// Bitsets
//----------------------------------------------------------------------------
#if defined (__GNUC__)
TF_INLINE int clz (unsigned int n) { return __builtin_clz(n); }
TF_INLINE int ctz (unsigned int n) { return __builtin_ctz(n); }
TF_INLINE int ffs (unsigned int n) { return __builtin_ffs(n); }
#else
int clz (unsigned int x);
int ffs (unsigned int x);
int ctz (unsigned int x) return ffs(x) -1; }
#endif

template<typename T>
TF_INLINE constexpr T clearBits (T val, T upd_bits) { return val & ~upd_bits; }
template<typename T>
TF_INLINE constexpr T setBits (T val, T upd_bits) { return val | upd_bits; }
template<typename T>
TF_INLINE constexpr T toggleBits (T val, T upd_bits) { return val ^ upd_bits; }
template<typename T>
TF_INLINE constexpr T updateBits (T val, T upd_bits, bool set)
{ return (val & ~upd_bits) | (val & -T(set)); }

template<typename T>
TF_INLINE constexpr T isel(bool cond, const T& v1, const T& v2)
{ return v1 ^ ((v1 ^ v2) & -T(cond)); }

//----------------------------------------------------------------------------
// ConstSwapBytes
//----------------------------------------------------------------------------
TF_INLINE constexpr uint16_t ConstSwapBytes (uint16_t x)
{
   return ((x & 0XFF00) >> 8) | ((x & 0X00FF) << 8);
}

TF_INLINE constexpr uint32_t ConstSwapBytes (uint32_t x)
{
   return uint32_t(ConstSwapBytes(uint16_t((x & 0XFFFF0000) >> 16))) |
          uint32_t((ConstSwapBytes(uint16_t(x & 0X0000FFFF))) << 16);
}

TF_INLINE constexpr uint64_t ConstSwapBytes (uint64_t x)
{
   return uint64_t(ConstSwapBytes(uint32_t((x & 0XFFFFFFFF00000000) >> 32))) |
          (uint64_t(ConstSwapBytes(uint32_t(x & 0X00000000FFFFFFFF))) << 32);
}

//----------------------------------------------------------------------------
// Floats and Doubles
//----------------------------------------------------------------------------
template <typename T>
class FloatChore
{
    using INT = typename std::conditional<
                std::is_same<T, float>::value, uint32_t, uint64_t>::type;
    constexpr static INT SIGNBIT = 1<<(sizeof(INT)-1);
    constexpr static INT INTMASK = ~SIGNBIT;
    constexpr TF_INLINE static INT castToInt(T val)
    { return reinterpret_cast<INT&>(val); }
    constexpr TF_INLINE static T castFromInt(INT val)
    { return reinterpret_cast<T&>(val); }
  public:
    constexpr TF_INLINE static T abs (T val)
    { return castFromInt(castToInt(val)&INTMASK); }
    constexpr TF_INLINE static bool is0 (T val, T e)
    { return castToInt(val)&INTMASK < castToInt(e); }
    constexpr TF_INLINE static T fsel(bool c, T v1, T v2)
    { return castFromInt(isel(c, castToInt(v1),castToInt(v2))); }
};

using fchore = FloatChore<float>;
using dchore = FloatChore<double>;

//----------------------------------------------------------------------------
// swap_bytes
//----------------------------------------------------------------------------

#if defined(__GLIBC__)
#include <byteswap.h>
TF_INLINE T swapBytes (uint_16 v=x) return { __bswap_16(x); }
TF_INLINE T swapBytes (uint_32 v=x) return { __bswap_32(x); }
TF_INLINE T swapBytes (uint_64 v=x) return { __bswap_64(x); }
#else
template <typename T>
TF_INLINE T swapBytes (T x) { return ConstSwapBytes(x); }
#endif

//----------------------------------------------------------------------------
// Edianness conversion utils
//----------------------------------------------------------------------------
#if (TF_ENDIAN==TF_ENDIAN_LITTLE)
template <typename T> TF_INLINE T htobe (T n) { return swapBytes(n); }
template <typename T> TF_INLINE T htole (T n) { return n; }
template <typename T> TF_INLINE T betoh (T n) { return swapBytes(n); }
template <typename T> TF_INLINE T letoh (T n) { return n; }
#else
template <typename T> TF_INLINE T htobe (T n) { return n; }
template <typename T> TF_INLINE T htole (T n) { return swapBytes(n); }
template <typename T> TF_INLINE T betoh (T n) { return n; }
template <typename T> TF_INLINE T letoh (T n) { return swapBytes(n); }
#endif

//----------------------------------------------------------------------------
// string utils
//----------------------------------------------------------------------------

TF_INLINE void str2Cpy (char* d, const char* s) 
{ *((int16_t*)d)=*((int16_t*)s); }
TF_INLINE void str3Cpy (char* d, const char* s)
{ str2Cpy(d+4,s+4); *(d+4)=*(s+4); }
TF_INLINE void str4Cpy (char* d, const char* s)
{ *((int32_t*)d)=*((int32_t*)s); }
TF_INLINE void str5Cpy (char* d, const char* s)
{ str4Cpy(d,s); *(d+4)=*(s+4); }
TF_INLINE void str6Cpy (char* d, const char* s)
{ str4Cpy(d,s); str2Cpy(d+4,s+4); }
TF_INLINE void str7Cpy (char* d, const char* s)
{ str4Cpy(d,s); str3Cpy(d+4,s+4); }
TF_INLINE void str8Cpy (char* d, const char* s)
{ *((int64_t*)d)=*((int64_t*)s); }
TF_INLINE void str9Cpy (char* d, const char* s)
{ str8Cpy(d,s); *(d+8)=*(s+8); }
TF_INLINE void str10Cpy (char* d, const char* s)
{ str8Cpy(d,s); str2Cpy(d+8,s+8); }

TF_INLINE bool str2Equal (const char* x, const char* y)
{ return *((int16_t*)x)==*((int16_t*)y); }
TF_INLINE bool str3Equal (const char* x, const char* y)
{ return str2Equal(x,y) && *(x+2)==*(y+2); }
TF_INLINE bool str4Equal (const char* x, const char* y)
{ return *((int32_t*)x)==*((int32_t*)y); }
TF_INLINE bool str5Equal (const char* x, const char* y)
{ return str4Equal(x,y) && *(x+2)==*(y+2); }
TF_INLINE bool str6Equal (const char* x, const char* y)
{ return str4Equal(x,y) && str2Equal(x+4,y+4); }
TF_INLINE bool str7Equal (const char* x, const char* y)
{ return str4Equal(x,y) && str3Equal(x+4,y+4); }
TF_INLINE bool str8Equal (const char* x, const char* y)
{ return *((int64_t*)x)==*((int64_t*)y); }
TF_INLINE bool str9Equal (const char* x, const char* y)
{ return str8Equal(x,y) && *(x+8)==*(y+8); }
TF_INLINE bool str10Equal (const char* x, const char* y)
{ return str8Equal(x,y) && str2Equal(x+8,y+8); }

// Call these functions if you know the start buffer is already
// aligned by 16. Oterwise, called the unaligned version
const char* fastStrChrAligned(const char* s, char ch);
size_t fastStrLenAligned(const char* s);
uint64_t fastSumAligned(const uint8_t* bytes, int sz);

const char* fastStrChr(const char* s, char ch);
size_t fastStrLen(const char* s);
uint64_t fastSum(const uint8_t* bytes, int sz);

} // namespace tf