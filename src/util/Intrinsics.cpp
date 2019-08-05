
#include "Intrinsics.h"
#include <iostream>
#include <cstring>
#include <cassert>

#if defined (__SSE2__)
// use -msse2 for g++
#include <emmintrin.h>
#include <immintrin.h>
#endif

namespace tf
{
#if not defined (__GNUC__)
int ffs (int x)
{
    // De Bruijn Multiply table
    static const int DeBruijnTable[32] = 
    {
       1, 2, 29, 3, 30, 15, 25, 4, 31, 23, 21, 16, 26, 18, 5, 9, 
       32, 28, 14, 24, 22, 20, 17, 8, 27, 13, 19, 7, 12, 6, 11, 10
    };
    return DeBruijnTable[((unsigned)((x & -x) * 0x077CB531U)) >> 27];
}

int clz (int x)
{
    int n = 0;
    if (x == 0) return sizeof(x) * 8;
    for (; x>0; ++n, x<<1);
    return n;
}
#endif

const char* fastStrChrAligned(const char* s, char ch)
{
#if defined (__SSE2__)
    __m128i zero = _mm_setzero_si128();
    __m128i cx16 = _mm_set1_epi8(ch); // (ch) replicated 16 times.
    while (1) 
    {
        __m128i x = _mm_loadu_si128((__m128i const *)s);
        __m128i u = _mm_cmpeq_epi8(cx16, x);
        __m128i v = _mm_cmpeq_epi8(zero, x);
        int m1 = _mm_movemask_epi8(u);
        int m0 = _mm_movemask_epi8(v);
        if (m1 && (m1 < m0 || m0==0))
        {
            return s + ctz(m1);
            break;
        }
        if (m0)
        {
            return nullptr;
        }
        s += 16;
    }
#else
    return strchr(s,ch);
#endif
}

const char* fastStrChr(const char* s, char ch)
{
#if defined (__SSE2__)
    const char* aligned_addr = ConstAlign(s, 16);
    while (s < aligned_addr)
    {
        if (*s==0)
        {
            return nullptr;
        }
        if (*s==ch)
        {
            return s;
        }
        ++s;   
    }
    return fastStrChrAligned(s, ch);
#else
    return strchr(s,ch);
#endif
}

size_t fastStrLenAligned(const char* s)
{
#if defined (__SSE2__)
    size_t len(0);
    __m128i zero = _mm_setzero_si128();
    __m128i cx16 = _mm_set1_epi8(0);
    while (1) 
    {
        __m128i x = _mm_load_si128((__m128i const *)s);
        x = _mm_cmpeq_epi8(x, zero);
        int m = _mm_movemask_epi8(x);
        if (m)
        {
            len += ctz(m);
            break;
        }
        s += 16;
        len += 16;
    }
    return len;
#else
    return strlen(s);
#endif
}

size_t fastStrLen(const char* s)
{
#if defined (__SSE2__)
    size_t len(0);
    const char* aligned_addr = ConstAlign(s, 16);
    while (s < aligned_addr)
    {
        if (*s++==0)
        {
            return len;
        }
        ++len;   
    }
    return len + fastStrLenAligned(s);
#else
    return strlen(s);
#endif
}

uint64_t fastSumAliged(const uint8_t* bytes, int sz)
{
#if defined (__SSE2__)
    auto sum256 = [](const uint8_t a[], size_t n)
    {
        // constant vector of all zeros
        const __m128i vk0 = _mm_set1_epi8(0);
        // constant vector of all 1's
        const __m128i vk1 = _mm_set1_epi16(1);
        __m128i sum128 = _mm_set1_epi16(0);
        for (size_t i = 0; i < n; i += 16)
        {
            // load 16 bytes of 8 bit values
            __m128i v = _mm_load_si128((const __m128i*)&a[i]);
            // unpack to two vectors in 16 bit values
            __m128i vl = _mm_unpacklo_epi8(v, vk0);
            __m128i vh = _mm_unpackhi_epi8(v, vk0);
            // add vl and vh to sum128
            sum128 = _mm_add_epi16(sum128, _mm_add_epi16(vl, vh));
            //std::cout << "low32=" << std::hex << _mm_cvtsi128_si32(sum128) 
            //<< std::dec << std::endl;
        }
        // now sum128 has 8 sums in 16-bit. Combine them into 4 sums in 32-bit
        sum128 = _mm_madd_epi16(sum128, vk1);
        // horizontal add of 4 sums in 32-bit
        sum128 = _mm_add_epi32(sum128, _mm_srli_si128(sum128, 8));
        sum128 = _mm_add_epi32(sum128, _mm_srli_si128(sum128, 4));
        uint32_t lsum = _mm_cvtsi128_si32(sum128);
        return lsum;
    };
    uint64_t sum {0};
    // add up to 256*16 bytes one time to avoid overflow in sum256
    constexpr int max_add_bytes = 256*16;
    while (sz >= max_add_bytes)
    {
        sum += sum256(bytes, max_add_bytes);
        bytes += max_add_bytes;
        sz -= max_add_bytes;
    }
    if (sz)
    {
        size_t even_sz = (sz/16)*16;
        size_t oddsz = sz - even_sz;
        if (even_sz)
        {
            sum += sum256(bytes, even_sz);
            bytes += even_sz;
            sz -= even_sz;
        }
        while (oddsz)
        {
            sum += *bytes++;
           --oddsz;
        }
    }
    return sum;
#else
    uint64_t sum {0};
    while (sz)
    {
        sum += *bytes++;
        --sz;
    }
    return sum;
#endif
}

uint64_t fastSum(const uint8_t* bytes, int sz)
{
#if defined (__SSE2__)
    uint64_t sum {0};
    // check alignment of bytes
    const uint8_t* aligned_addr = ConstAlign(bytes, 16);
    while (bytes < aligned_addr)
    {
        sum += *bytes++;
        --sz;   
    }
    return sum + fastSumAliged(bytes, sz);
#else
    uint64_t sum {0};
    while (sz)
    {
        sum += *bytes++;
        --sz;
    }
#endif
}

void fastMemcpyAligned(void *dest, void *src, size_t sz)
{
#if defined (__AVX2__)
// requies -mavx2
    // assert alighments
    assert(sz % sizeof(__m256i) == 0);
    assert((intptr_t(dest) & (sizeof(__m256i)-1)) == 0);
    assert((intptr_t(src) & (sizeof(__m256i)-1)) == 0);
    const __m256i *s = (const __m256i*)src;
    __m256i *d = (__m256i*)dest;
    for (size_t n = sz >> 5; n > 0; n--, s++, d++)
        _mm256_stream_si256(d, _mm256_stream_load_si256(s));
    _mm_sfence();
#elif defined (__SSE2__)
    assert(sz % sizeof(__m128i) == 0);
    assert((intptr_t(dest) & (sizeof(__m128i)-1)) == 0);
    assert((intptr_t(src) & (sizeof(__m128i)-1)) == 0);
    const __m128i *s = (const __m128i*)src;
    __m128i *d = (__m128i*)dest;
    for (size_t n = sz >> 4; n > 0; n--, s++, d++)
        _mm_stream_si128(d, _mm_stream_load_si128(s));
    _mm_sfence();
#else
    assert(sz % sizeof(uint64_t) == 0);
    assert((intptr_t(dest) & (sizeof(uint64_t)-1)) == 0);
    assert((intptr_t(src) & (sizeof(uint64_t)-1)) == 0);
    for(size_t n_sz = sz >> 3; n_sz > 0 ; --n_sz)
        *(uint64_t*)dest++ = *(uint64_t*)src++;
#endif
}

void fastMemcpyAlignedBackword(void *dest, void *src, size_t sz)
{
#if defined (__AVX2__)
    // assert alighments
    assert(sz % sizeof(__m256i) == 0);
    assert((intptr_t(dest) & (sizeof(__m256i)-1)) == 0);
    assert((intptr_t(src) & (sizeof(__m256i)-1)) == 0);
    size_t n_sz = sz >> 5;
    const __m256i *s = (const __m256i*)src + n_sz;
    __m256i *d = (__m256i*)dest + n_sz;
    for (; n_sz > 0; n_sz--, --s, --d)
        _mm256_stream_si256(d, _mm256_stream_load_si256(s));
    _mm_sfence();
#elif defined (__SSE2__)
    assert(sz % sizeof(__m128i) == 0);
    assert((intptr_t(dest) & (sizeof(__m128i)-1)) == 0);
    assert((intptr_t(src) & (sizeof(__m128i)-1)) == 0);
    size_t n_sz = sz >> 4;
     const __m128i *s = (const __m128i*)src + n_sz;
    __m128i *d = (__m128i*)dest + n_sz;
    for (; n_sz > 0; n_sz--, s++, d++)
        _mm_stream_si128(d, _mm_stream_load_si128(s));
    _mm_sfence();
#else
    assert(sz % sizeof(uint64_t) == 0);
    assert((intptr_t(dest) & (sizeof(uint64_t)-1)) == 0);
    assert((intptr_t(src) & (sizeof(uint64_t)-1)) == 0);
    size_t n_sz = sz >> 3;
    const uint64_t* s = (const uint64_t*)src + n_sz;
    uint64_t* d = (uint64_t*)dest + n_sz;
    for(; n_sz > 0 ; --n)
        *(uint64_t*)dest++ = *(uint64_t*)src++;
#endif
}

} // namespace tf

int main ()
{
    uint8_t ints[777];
    for (size_t i=0; i<sizeof(ints); ++i) ints[i]=1;
    auto sum = tf::fastSum(ints, sizeof(ints));
    std::cout << "SUM=" << sum << std::endl;

    const char * str = "snch whissw n iwdksllsfhhw sdd ssfff fxxxxxxxxxdq";
    const char * cp = tf::fastStrChr(str, 'q');
    const char * cp1 = strchr(str, 'q');
    size_t len = tf::fastStrLen(str);
    if (cp && cp1)
        std::cout << "FAST FIND pos=" << (int)(cp-str)
        << " pos=" << (int)(cp1-str)
        << std::endl;
    else
    {
        std::cout << "Not FIND" << std::endl;
    }
    std::cout << "FAST LEN=" << len << " len=" << strlen(str) << std::endl;
    
    return 0;
}