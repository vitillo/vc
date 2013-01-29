/*  This file is part of the Vc library.

    Copyright (C) 2009-2012 Matthias Kretz <kretz@kde.org>

    Vc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Vc is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Vc.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef VC_GLOBAL_H
#define VC_GLOBAL_H

#ifndef DOXYGEN

// Compiler defines
#ifdef __INTEL_COMPILER
#define VC_ICC __INTEL_COMPILER_BUILD_DATE
#elif defined(__OPENCC__)
#define VC_OPEN64 1
#elif defined(__clang__)
#define VC_CLANG (__clang_major__ * 0x10000 + __clang_minor__ * 0x100 + __clang_patchlevel__)
#elif defined(__GNUC__)
#define VC_GCC (__GNUC__ * 0x10000 + __GNUC_MINOR__ * 0x100 + __GNUC_PATCHLEVEL__)
#elif defined(_MSC_VER)
#define VC_MSVC _MSC_FULL_VER
#else
#define VC_UNSUPPORTED_COMPILER 1
#endif

// Features/Quirks defines
#if defined VC_MSVC && defined _WIN32
// the Win32 ABI can't handle function parameters with alignment >= 16
#define VC_PASSING_VECTOR_BY_VALUE_IS_BROKEN 1
#endif
#if defined(__GNUC__) && !defined(VC_NO_INLINE_ASM)
#define VC_GNU_ASM 1
#endif
#if defined(VC_GCC) && (VC_GCC <= 0x40405 || (VC_GCC >= 0x40500 && VC_GCC <= 0x40502)) && !(VC_GCC == 0x40502 && defined(__GNUC_UBUNTU_VERSION__) && __GNUC_UBUNTU_VERSION__ == 0xb0408)
// GCC 4.6.0 / 4.5.3 / 4.4.6 switched to the interface as defined by ICC
// (Ubuntu 11.04 ships a GCC 4.5.2 with the new interface)
#define VC_MM256_MASKSTORE_WRONG_MASK_TYPE 1
#endif
#if defined(VC_GCC) && VC_GCC >= 0x40300
#define VC_HAVE_ATTRIBUTE_ERROR 1
#define VC_HAVE_ATTRIBUTE_WARNING 1
#endif

#if (defined(__GXX_EXPERIMENTAL_CXX0X__) && VC_GCC >= 0x40600) || __cplusplus >= 201103
#  define VC_CXX11 1
#  ifdef VC_GCC
#    if VC_GCC >= 0x40700 // && VC_GCC < 0x408000)
//     ::max_align_t was introduced with GCC 4.7. std::max_align_t took a bit longer.
#      define VC_HAVE_MAX_ALIGN_T 1
#    endif
#  elif !defined(VC_CLANG)
//   Clang doesn't provide max_align_t at all
#    define VC_HAVE_STD_MAX_ALIGN_T 1
#  endif
#endif

// ICC ships the AVX2 intrinsics inside the AVX1 header.
// FIXME: the number 20120731 is too large, but I don't know which one is the right one
#if VC_ICC >= 20120731 || VC_MSVC >= 170000000
#define VC_UNCONDITIONAL_AVX2_INTRINSICS 1
#endif

/* Define the following strings to a unique integer, which is the only type the preprocessor can
 * compare. This allows to use -DVC_IMPL=SSE3. The preprocessor will then consider VC_IMPL and SSE3
 * to be equal. Of course, it is important to undefine the strings later on!
 */
#define SSE    9875294
#define SSE2   9875295
#define SSE3   9875296
#define SSSE3  9875297
#define SSE4_1 9875298
#define Scalar 9875299
#define SSE4_2 9875301
#define SSE4a  9875302
#define AVX    9875303

#ifdef VC_MSVC
# ifdef _M_IX86_FP
#  if _M_IX86_FP >= 1
#   ifndef __SSE__
#    define __SSE__ 1
#   endif
#  endif
#  if _M_IX86_FP >= 2
#   ifndef __SSE2__
#    define __SSE2__ 1
#   endif
#  endif
# elif defined(_M_AMD64)
// If the target is x86_64 then SSE2 is guaranteed
#  ifndef __SSE__
#   define __SSE__ 1
#  endif
#  ifndef __SSE2__
#   define __SSE2__ 1
#  endif
# endif
#endif

#ifndef VC_IMPL

#  if defined(__AVX__)
#    define VC_IMPL_AVX 1
#  else
#    if defined(__SSE4A__)
#      define VC_IMPL_SSE 1
#      define VC_IMPL_SSE4a 1
#    endif
#    if defined(__SSE4_2__)
#      define VC_IMPL_SSE 1
#      define VC_IMPL_SSE4_2 1
#    endif
#    if defined(__SSE4_1__)
#      define VC_IMPL_SSE 1
#      define VC_IMPL_SSE4_1 1
#    endif
#    if defined(__SSE3__)
#      define VC_IMPL_SSE 1
#      define VC_IMPL_SSE3 1
#    endif
#    if defined(__SSSE3__)
#      define VC_IMPL_SSE 1
#      define VC_IMPL_SSSE3 1
#    endif
#    if defined(__SSE2__)
#      define VC_IMPL_SSE 1
#      define VC_IMPL_SSE2 1
#    endif

#    if defined(VC_IMPL_SSE)
       // nothing
#    else
#      define VC_IMPL_Scalar 1
#    endif
#  endif

#else // VC_IMPL

#  if VC_IMPL == AVX // AVX supersedes SSE
#    define VC_IMPL_AVX 1
#  elif VC_IMPL == Scalar
#    define VC_IMPL_Scalar 1
#  elif VC_IMPL == SSE4a
#    define VC_IMPL_SSE4a 1
#    define VC_IMPL_SSE3 1
#    define VC_IMPL_SSE2 1
#    define VC_IMPL_SSE 1
#  elif VC_IMPL == SSE4_2
#    define VC_IMPL_SSE4_2 1
#    define VC_IMPL_SSE4_1 1
#    define VC_IMPL_SSSE3 1
#    define VC_IMPL_SSE3 1
#    define VC_IMPL_SSE2 1
#    define VC_IMPL_SSE 1
#  elif VC_IMPL == SSE4_1
#    define VC_IMPL_SSE4_1 1
#    define VC_IMPL_SSSE3 1
#    define VC_IMPL_SSE3 1
#    define VC_IMPL_SSE2 1
#    define VC_IMPL_SSE 1
#  elif VC_IMPL == SSSE3
#    define VC_IMPL_SSSE3 1
#    define VC_IMPL_SSE3 1
#    define VC_IMPL_SSE2 1
#    define VC_IMPL_SSE 1
#  elif VC_IMPL == SSE3
#    define VC_IMPL_SSE3 1
#    define VC_IMPL_SSE2 1
#    define VC_IMPL_SSE 1
#  elif VC_IMPL == SSE2
#    define VC_IMPL_SSE2 1
#    define VC_IMPL_SSE 1
#  elif VC_IMPL == SSE
#    define VC_IMPL_SSE 1
#    if defined(__SSE4A__)
#      define VC_IMPL_SSE4a 1
#    endif
#    if defined(__SSE4_2__)
#      define VC_IMPL_SSE4_2 1
#    endif
#    if defined(__SSE4_1__)
#      define VC_IMPL_SSE4_1 1
#    endif
#    if defined(__SSE3__)
#      define VC_IMPL_SSE3 1
#    endif
#    if defined(__SSSE3__)
#      define VC_IMPL_SSSE3 1
#    endif
#    if defined(__SSE2__)
#      define VC_IMPL_SSE2 1
#    endif
#  endif
#  undef VC_IMPL

#endif // VC_IMPL

// If AVX is enabled in the compiler it will use VEX coding for the SIMD instructions.
#ifdef __AVX__
#  define VC_USE_VEX_CODING 1
#endif

#ifdef VC_IMPL_AVX
// There are no explicit switches for FMA4/XOP in Vc yet, so enable it when the compiler
// says it's active and AVX is enabled
#ifdef __FMA4__
#  define VC_IMPL_FMA4 1
#endif
#ifdef __XOP__
#  define VC_IMPL_XOP 1
#endif
#endif

#if defined(VC_GCC) && VC_GCC < 0x40300 && !defined(VC_IMPL_Scalar)
#    ifndef VC_DONT_WARN_OLD_GCC
#      warning "GCC < 4.3 does not have full support for SSE2 intrinsics. Using scalar types/operations only. Define VC_DONT_WARN_OLD_GCC to silence this warning."
#    endif
#    undef VC_IMPL_SSE
#    undef VC_IMPL_SSE2
#    undef VC_IMPL_SSE3
#    undef VC_IMPL_SSE4a
#    undef VC_IMPL_SSE4_1
#    undef VC_IMPL_SSE4_2
#    undef VC_IMPL_SSSE3
#    undef VC_IMPL_AVX
#    undef VC_IMPL_FMA4
#    undef VC_IMPL_XOP
#    undef VC_USE_VEX_CODING
#    define VC_IMPL_Scalar 1
#endif

# if !defined(VC_IMPL_Scalar) && !defined(VC_IMPL_SSE) && !defined(VC_IMPL_AVX)
#  error "No suitable Vc implementation was selected! Probably VC_IMPL was set to an invalid value."
# elif defined(VC_IMPL_SSE) && !defined(VC_IMPL_SSE2)
#  error "SSE requested but no SSE2 support. Vc needs at least SSE2!"
# endif

#undef SSE
#undef SSE2
#undef SSE3
#undef SSSE3
#undef SSE4_1
#undef SSE4_2
#undef SSE4a
#undef AVX
#undef Scalar

namespace Vc {
enum AlignedFlag {
    Aligned = 0
};
enum UnalignedFlag {
    Unaligned = 1
};
enum StreamingAndAlignedFlag { // implies Aligned
    Streaming = 2
};
enum StreamingAndUnalignedFlag {
    StreamingAndUnaligned = 3
};
#endif // DOXYGEN

/**
 * \ingroup Utilities
 *
 * Enum that specifies the alignment and padding restrictions to use for memory allocation with
 * Vc::malloc.
 */
enum MallocAlignment {
    /**
     * Align on boundary of vector sizes (e.g. 16 Bytes on SSE platforms) and pad to allow
     * vector access to the end. Thus the allocated memory contains a multiple of
     * VectorAlignment bytes.
     */
    AlignOnVector,
    /**
     * Align on boundary of cache line sizes (e.g. 64 Bytes on x86) and pad to allow
     * full cache line access to the end. Thus the allocated memory contains a multiple of
     * 64 bytes.
     */
    AlignOnCacheline,
    /**
     * Align on boundary of page sizes (e.g. 4096 Bytes on x86) and pad to allow
     * full page access to the end. Thus the allocated memory contains a multiple of
     * 4096 bytes.
     */
    AlignOnPage
};

static inline StreamingAndUnalignedFlag operator|(UnalignedFlag, StreamingAndAlignedFlag) { return StreamingAndUnaligned; }
static inline StreamingAndUnalignedFlag operator|(StreamingAndAlignedFlag, UnalignedFlag) { return StreamingAndUnaligned; }
static inline StreamingAndUnalignedFlag operator&(UnalignedFlag, StreamingAndAlignedFlag) { return StreamingAndUnaligned; }
static inline StreamingAndUnalignedFlag operator&(StreamingAndAlignedFlag, UnalignedFlag) { return StreamingAndUnaligned; }

static inline StreamingAndAlignedFlag operator|(AlignedFlag, StreamingAndAlignedFlag) { return Streaming; }
static inline StreamingAndAlignedFlag operator|(StreamingAndAlignedFlag, AlignedFlag) { return Streaming; }
static inline StreamingAndAlignedFlag operator&(AlignedFlag, StreamingAndAlignedFlag) { return Streaming; }
static inline StreamingAndAlignedFlag operator&(StreamingAndAlignedFlag, AlignedFlag) { return Streaming; }

/**
 * \ingroup Utilities
 *
 * Enum to identify a certain SIMD instruction set.
 *
 * You can use \ref VC_IMPL for the currently active implementation.
 */
enum Implementation {
    /// uses only built-in types
    ScalarImpl,
    /// x86 SSE + SSE2
    SSE2Impl,
    /// x86 SSE + SSE2 + SSE3
    SSE3Impl,
    /// x86 SSE + SSE2 + SSE3 + SSSE3
    SSSE3Impl,
    /// x86 SSE + SSE2 + SSE3 + SSSE3 + SSE4.1
    SSE41Impl,
    /// x86 SSE + SSE2 + SSE3 + SSSE3 + SSE4.1 + SSE4.2
    SSE42Impl,
    /// x86 (AMD only) SSE + SSE2 + SSE3 + SSE4a
    SSE4aImpl,
    /// x86 AVX
    AVXImpl,
    /// x86 (AMD only) XOP
    Fma4Impl,
    /// x86 (AMD only) FMA4
    XopImpl
};

#ifdef DOXYGEN
/**
 * \ingroup Utilities
 *
 * This macro is set to the value of \ref Vc::Implementation that the current translation unit is
 * compiled with.
 */
#define VC_IMPL
#elif VC_IMPL_Scalar
#define VC_IMPL ::Vc::ScalarImpl
#elif VC_IMPL_AVX
#define VC_IMPL ::Vc::AVXImpl
#elif VC_IMPL_SSE4a
#define VC_IMPL ::Vc::SSE4aImpl
#elif VC_IMPL_SSE4_2
#define VC_IMPL ::Vc::SSE42Impl
#elif VC_IMPL_SSE4_1
#define VC_IMPL ::Vc::SSE41Impl
#elif VC_IMPL_SSSE3
#define VC_IMPL ::Vc::SSSE3Impl
#elif VC_IMPL_SSE3
#define VC_IMPL ::Vc::SSE3Impl
#elif VC_IMPL_SSE2
#define VC_IMPL ::Vc::SSE2Impl
#endif

namespace Internal {
    template<Implementation Impl> struct HelperImpl;
    typedef HelperImpl<VC_IMPL> Helper;

    template<typename A> struct FlagObject;
    template<> struct FlagObject<AlignedFlag> { static inline AlignedFlag the() { return Aligned; } };
    template<> struct FlagObject<UnalignedFlag> { static inline UnalignedFlag the() { return Unaligned; } };
    template<> struct FlagObject<StreamingAndAlignedFlag> { static inline StreamingAndAlignedFlag the() { return Streaming; } };
    template<> struct FlagObject<StreamingAndUnalignedFlag> { static inline StreamingAndUnalignedFlag the() { return StreamingAndUnaligned; } };
} // namespace Internal

namespace Warnings
{
    void _operator_bracket_warning()
#if VC_HAVE_ATTRIBUTE_WARNING
        __attribute__((warning("\n\tUse of Vc::Vector::operator[] to modify scalar entries is known to miscompile with GCC 4.3.x.\n\tPlease upgrade to a more recent GCC or avoid operator[] altogether.\n\t(This warning adds an unnecessary function call to operator[] which should work around the problem at a little extra cost.)")))
#endif
        ;
} // namespace Warnings

namespace Error
{
    template<typename L, typename R> struct invalid_operands_of_types {};
} // namespace Error

} // namespace Vc

#include "version.h"

#endif // VC_GLOBAL_H
