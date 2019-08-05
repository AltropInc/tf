
#pragma once

// Platform Operating System
// OS 
#define TF_OS_UNKNOWN     0
#define TF_OS_LINUX       1
#define TF_OS_WINDOWS     2
#define TF_OS_MAC_OS_X    3 // For iOS and OSX
#define TF_OS_ANDRIOD     4

// OS FAMILY
#define  TF_OS_FAMILY_UNKNOWN  0
#define  TF_OS_FAMILY_LINUX    1
#define  TF_OS_FAMILY_BSD      2
#define  TF_OS_FMAIL_WINDOWS   3

#if defined (__ANDRIOD__)
    #define TF_OS          TF_OS_ANDRIOD
    #define TF_OS_FAMILY   TF_OS_FAMILY_LINUX
#elif defined (__linux__)
    #define TF_OS          TF_OS_LINUX
    #define TF_OS_FAMILY   TF_OS_FAMILY_LINUX
#elif defined (__CYGWIN__)
    #if defined (_GNU_SOURCE)
        #define TF_OS          TF_OS_LINUX
        #define TF_OS_FAMILY   TF_OS_FAMILY_LINUX
    #else
        #define TF_OS          TF_OS_WINDOWS
        #define TF_OS_FAMILY   TF_OS_FAMILY_WINDOWS
    #endif
#elif defined (__APPLE__)
    #define TF_OS          TF_OS_MAC_OS_X
    #define TF_OS_FAMILY   TF_OS_FAMILY_MAC
#elif defined (_WIN32) || defined (_WIN64)
    #define TF_OS          TF_OS_WINDOWS
    #define TF_OS_FAMILY   TF_OS_FAMILY_WINDOWS
#else
    #error "OS unknown, not supported"
#endif

// OS depednet marcos
#if (TF_OS_FAMILY==TF_OS_WINDOWS)
    #define TF_NEWLINE    "\r\n"
#else
    #define TF_NEWLINE    "\n"
#endif

// Platform CPU Architectures
#define TF_ARCH_UNKNOWN   0
#define TF_ARCH_I386      1  // 32 Bit X86
#define TF_ARCH_AMD64     2  // X86-64 by Intel
#define TF_ARCH_ARM       3  // 32 Bit RISC
#define TF_ARCH_AARCH64   4  // 64 Bit RISC, ARM64 by appple
#define TF_ARCH_RISCV64   5  // 64 Bit RISC-V by OSS

// gcc -march=native -dM -E - </dev/null 
#if defined (__x86_64__) || defined (_X86_) || defined (__amd64__)
    #define TF_CPU        TF_ARCH_AMD64
#elif defined (__arm__)
    #define TF_CPU        TF_ARCH_ARM
#elif defined (__aarch64__)
    #define TF_CPU        TF_ARCH_ARM64
#elif defined (__powerpc64__)
    #define TF_CPU        TF_ARCH_POWERPC
#elif defined (__aarch64__)
    #define TF_CPU        TF_ARCH_AARCH64
#elif defined (__riscv) && (__riscv_xlen==64)
    #define TF_CPU        TF_ARCH_RISCV64
#elif defined (__i386__) || defined (i386) || defined (__i386)
    #define TF_CPU        TF_ARCH_I386
#else
    #define TF_CPU        TF_ARCH_UNKNOWN
    #error "Unknown architecture"
#endif

// Compiler, currently only GCC 
#define TF_COMPILER_UNKNOWN    0
#define TF_COMPILER_GCC        1

#if defined (__GNUC__)
    #define TF_COMPLIER     TF_COMPILER_GCC
#else
    #error "Compiler not supported"
#endif

// C++ attributes
#if (__cplusplus >= 2017003L)
    // C++17
    #define TF_FALLTROUGH        [[fallthrough]]
    #define TF_NODISCARD         [[nodiscard]]
    #define TF_MAYBE_UNUSED      [[maybe_unused]]
#endif
#if (__cplusplus >= 201402L)
    // C++ 14
    #define TF_DEPRECATED        [[deprecated]]
    #define TF_DEPRECATED_BECUASE(reason) [[deprecated(reason)]]
#endif
#if (__cplusplus >= 201103L)
    // C++ 11
    #define TF_NORETURN            [[noreturn]]
    #define TF_CARRIES_DEPENDENCY  [[carries_dependency]]
#else
    #error "Requires C++11 or later"
#endif
// Compiler specific attributes
//#if (TF_COMPILER==TF_COMPILER_GCC)
#if defined (__GNUC__)
    // inline
    #if defined(NDEBUG)
        #define TF_INLINE        [[gnu::always_inline]]
    #else
        #define TF_INLINE inline  // Do not force inline on debug
    #endif
    #define TF_HOT               [[gnu::hot]]
    #define TF_COLD              [[gnu::cold]]
    #define TF_LIKELY(x)         __builtin_expect(!!(x), 1)
    #define TF_UNLIKELY(x)       __builtin_expect(!!(x), 0)
    #ifndef TF_DEPRECATED
        #define TF_DEPRECATED    [[gnu::deprecated]]
        #define TF_DEPRECATED_BECUASE(reason)  [[gnu::deprecated(reason)]]
    #endif
    #ifndef TF_NODISCARD
        #define TF_NODISCARD     [[gnu::warn_unused_result]]
    #endif
#endif
// Define default for all undefined attributes
#ifndef TF_FALLTROUGH
    #define TF_FALLTROUGH
#endif
#ifndef TF_NODISCARD
    #define TF_NODISCARD
#endif
#ifndef TF_MAYBE_UNUSED
    #define TF_MAYBE_UNUSED
#endif
#ifndef TF_DEPRECATED
    #define TF_DEPRECATED
    #define TF_DEPRECATED_BECUASE(x)
#endif
#ifndef TF_INLINE
    #define TF_INLINE    inline
#endif
#ifndef TF_HOT
    #define TF_HOT
#endif
#ifndef TF_COLD
    #define TF_COLD
#endif
#ifndef TF_LIKELY
    #define TF_LIKELY(x)     x
    #define TF_UNLIKELY(x)   x
#endif

#if defined _WIN32 || defined __CYGWIN__
  #define TF_IMPORT __declspec(dllimport)
  #define TF_EXPORT __declspec(dllexport)
  #define TF_LOCAL
#else
  #if __GNUC__ >= 4
    #define TF_IMPORT __attribute__ ((visibility ("default")))
    #define TF_EXPORT __attribute__ ((visibility ("default")))
    #define TF_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define TF_IMPORT
    #define TF_EXPORT
    #define TF_LOCAL
  #endif
#endif

#ifdef __GNUC__
  #define ALIGNMENT(x)  __attribute__((align(x)))
#endif

#ifdef TF_CORE_DLL
  // defined when TF_CORE is compiled as a DLL
  #ifdef TF_CORE_EXPORTS
    // defined if we are building the TF CORE DLL (instead of using it)
    #define TF_CORE_PUBLIC TF_EXPORT
  #else
    #define TF_CORE_PUBLIC TF_IMPORT
  #endif
  #define TF_CORE_LOCAL TF_LOCAL
#else 
  // when TF_CORE is compiled as a stati clib
  #define TF_CORE_PUBLIC
  #define TF_CORE_LOCAL
#endif

// endian
#define TF_ENDIAN_UNKNOWN 0
#define TF_ENDIAN_LITTLE  1
#define TF_ENDIAN_BIG     2

// First, use library provided by the complier if any
#if defined (__GLIBC__)
    #include <endian.h>
    #if (__BYTE_ORDER==__LITTLE_ENDIAN)
        #define TF_ENDIAN    TF_ENDIAN_LITTLE
    #elif (__BYTE_ORDER==_BIG_ENDIAN)
        #define TF_ENDIAN    TF_ENDIAN_BIG
    #endif
#endif
// Then, check if there is built-in macro or flag
#if not defined (TF_ENDIAN)
    #if defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
        #define TF_ENDIAN   TF_ENDIAN_BIG
    #elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
        #define TF_ENDIAN   TF_ENDIAN_LITTLE
    #elif defined(__BYTE_ORDER__)
        #if (__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__)
            #define TF_ENDIAN  TF_ENDIAN_LITTLE
        #elif (__BYTE_ORDER__==__ORDER_BIG_ENDIAN__)
            #define TF_ENDIAN  TF_ENDIAN_BIG
        #endif
    #endif
#endif
// Finally, check the architecture
#if not defined (TF_ENDIAN)
    #if (TF_CPU==TF_ARCH_I386 || \
         TF_CPU==TF_ARCH_AMD64 || \
         TF_CPU==TF_RISCV64 \
        )
        #define TF_ENDIAN   TF_ENDIAN_LITTLE
    #elif (TF_CPU==TF_ARCH_POWERPC || \
           TF_CPU==TF_ARCH_AARCH64 \
          )
        #define TF_ENDIAN   TF_ENDIAN_BIG
    #elif (TF_CPU==TF_ARCH_ARM || \
           TF_CPU==TF_ARCH_ARM64 \
          )
        #if defined(__ARMEB__)
            #define TF_ENDIAN   TF_ENDIAN_BIG
        #else
            #define TF_ENDIAN   TF_ENDIAN_LITTLE
        #endif
    #elif (TF_CPU==TF_ARCH_AARCH64)
        #if defined(__AARCH64EB__)
            #define TF_ENDIAN   TF_ENDIAN_BIG
        #elif deinfed(__AARCH64EL_)
            #define TF_ENDIAN   TF_ENDIAN_LITTLE
        #else
            #error "Unknown Edian in AARCH64"
        #endif
    #else
        #error "Unknown Endian by architecture"
    #endif
#endif

#define CHAR_BIT       8

namespace tf {

struct CPUInfo
{
    enum { cache_alignment_ =64,
    };
};

}
