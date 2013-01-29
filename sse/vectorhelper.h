/*  This file is part of the Vc library.

    Copyright (C) 2009-2011 Matthias Kretz <kretz@kde.org>

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

#ifndef SSE_VECTORHELPER_H
#define SSE_VECTORHELPER_H

#include "types.h"
#include <limits>

namespace Vc
{
namespace SSE
{
    template<typename VectorType, unsigned int Size> struct SortHelper
    {
        static VectorType sort(VectorType) Vc_PURE;
    };
    template<unsigned int Size> struct SortHelper<M256, Size>
    {
        static M256 sort(const M256 &) Vc_PURE;
    };

#define OP0(name, code) static inline VectorType name() Vc_PURE { return code; }
#define OP2(name, code) static inline VectorType name(VectorTypeArg a, VectorTypeArg b) Vc_PURE { return code; }
#define OP3(name, code) static inline VectorType name(VectorTypeArg a, VectorTypeArg b, VectorTypeArg c) Vc_PURE { return code; }
    template<> struct VectorHelper<M256>
    {
        typedef M256 VectorType;
#ifdef VC_PASSING_VECTOR_BY_VALUE_IS_BROKEN
        typedef const VectorType &VectorTypeArg;
#else
        typedef const VectorType VectorTypeArg;
#endif
        template<typename A> static VectorType load(const float *x, A) Vc_PURE;
        static void store(float *mem, VectorTypeArg x, AlignedFlag);
        static void store(float *mem, VectorTypeArg x, UnalignedFlag);
        static void store(float *mem, VectorTypeArg x, StreamingAndAlignedFlag);
        static void store(float *mem, VectorTypeArg x, StreamingAndUnalignedFlag);
        static void store(float *mem, VectorTypeArg x, VectorTypeArg m, AlignedFlag);
        static void store(float *mem, VectorTypeArg x, VectorTypeArg m, UnalignedFlag);
        static void store(float *mem, VectorTypeArg x, VectorTypeArg m, StreamingAndAlignedFlag);
        static void store(float *mem, VectorTypeArg x, VectorTypeArg m, StreamingAndUnalignedFlag);

        OP0(allone, VectorType::create(_mm_setallone_ps(), _mm_setallone_ps()))
        OP0(zero, VectorType::create(_mm_setzero_ps(), _mm_setzero_ps()))
        OP2(or_, VectorType::create(_mm_or_ps(a[0], b[0]), _mm_or_ps(a[1], b[1])))
        OP2(xor_, VectorType::create(_mm_xor_ps(a[0], b[0]), _mm_xor_ps(a[1], b[1])))
        OP2(and_, VectorType::create(_mm_and_ps(a[0], b[0]), _mm_and_ps(a[1], b[1])))
        OP2(andnot_, VectorType::create(_mm_andnot_ps(a[0], b[0]), _mm_andnot_ps(a[1], b[1])))
        OP3(blend, VectorType::create(_mm_blendv_ps(a[0], b[0], c[0]), _mm_blendv_ps(a[1], b[1], c[1])))
    };
#undef OP0
#undef OP2
#undef OP3

#define OP0(name, code) static inline VectorType name() Vc_PURE { return code; }
#define OP1(name, code) static inline VectorType name(const VectorType a) Vc_PURE { return code; }
#define OP2(name, code) static inline VectorType name(const VectorType a, const VectorType b) Vc_PURE { return code; }
#define OP3(name, code) static inline VectorType name(const VectorType a, const VectorType b, const VectorType c) Vc_PURE { return code; }

        template<> struct VectorHelper<_M128>
        {
            typedef _M128 VectorType;
            template<typename A> static VectorType load(const float *x, A) Vc_PURE;
            static void store(float *mem, const VectorType x, AlignedFlag);
            static void store(float *mem, const VectorType x, UnalignedFlag);
            static void store(float *mem, const VectorType x, StreamingAndAlignedFlag);
            static void store(float *mem, const VectorType x, StreamingAndUnalignedFlag);
            static void store(float *mem, const VectorType x, const VectorType m, AlignedFlag);
            static void store(float *mem, const VectorType x, const VectorType m, UnalignedFlag);
            static void store(float *mem, const VectorType x, const VectorType m, StreamingAndAlignedFlag);
            static void store(float *mem, const VectorType x, const VectorType m, StreamingAndUnalignedFlag);

            OP0(allone, _mm_setallone_ps())
            OP0(zero, _mm_setzero_ps())
            OP2(or_, _mm_or_ps(a, b))
            OP2(xor_, _mm_xor_ps(a, b))
            OP2(and_, _mm_and_ps(a, b))
            OP2(andnot_, _mm_andnot_ps(a, b))
            OP3(blend, _mm_blendv_ps(a, b, c))
        };


        template<> struct VectorHelper<_M128D>
        {
            typedef _M128D VectorType;
            template<typename A> static VectorType load(const double *x, A) Vc_PURE;
            static void store(double *mem, const VectorType x, AlignedFlag);
            static void store(double *mem, const VectorType x, UnalignedFlag);
            static void store(double *mem, const VectorType x, StreamingAndAlignedFlag);
            static void store(double *mem, const VectorType x, StreamingAndUnalignedFlag);
            static void store(double *mem, const VectorType x, const VectorType m, AlignedFlag);
            static void store(double *mem, const VectorType x, const VectorType m, UnalignedFlag);
            static void store(double *mem, const VectorType x, const VectorType m, StreamingAndAlignedFlag);
            static void store(double *mem, const VectorType x, const VectorType m, StreamingAndUnalignedFlag);

            OP0(allone, _mm_setallone_pd())
            OP0(zero, _mm_setzero_pd())
            OP2(or_, _mm_or_pd(a, b))
            OP2(xor_, _mm_xor_pd(a, b))
            OP2(and_, _mm_and_pd(a, b))
            OP2(andnot_, _mm_andnot_pd(a, b))
            OP3(blend, _mm_blendv_pd(a, b, c))
        };

        template<> struct VectorHelper<_M128I>
        {
            typedef _M128I VectorType;
            template<typename T> static VectorType load(const T *x, AlignedFlag) Vc_PURE;
            template<typename T> static VectorType load(const T *x, UnalignedFlag) Vc_PURE;
            template<typename T> static VectorType load(const T *x, StreamingAndAlignedFlag) Vc_PURE;
            template<typename T> static VectorType load(const T *x, StreamingAndUnalignedFlag) Vc_PURE;
            template<typename T> static void store(T *mem, const VectorType x, AlignedFlag);
            template<typename T> static void store(T *mem, const VectorType x, UnalignedFlag);
            template<typename T> static void store(T *mem, const VectorType x, StreamingAndAlignedFlag);
            template<typename T> static void store(T *mem, const VectorType x, StreamingAndUnalignedFlag);
            template<typename T> static void store(T *mem, const VectorType x, const VectorType m, AlignedFlag);
            template<typename T> static void store(T *mem, const VectorType x, const VectorType m, UnalignedFlag);
            template<typename T> static void store(T *mem, const VectorType x, const VectorType m, StreamingAndAlignedFlag);
            template<typename T> static void store(T *mem, const VectorType x, const VectorType m, StreamingAndUnalignedFlag);

            OP0(allone, _mm_setallone_si128())
            OP0(zero, _mm_setzero_si128())
            OP2(or_, _mm_or_si128(a, b))
            OP2(xor_, _mm_xor_si128(a, b))
            OP2(and_, _mm_and_si128(a, b))
            OP2(andnot_, _mm_andnot_si128(a, b))
            OP3(blend, _mm_blendv_epi8(a, b, c))
        };

#undef OP1
#undef OP2
#undef OP3

#define OP1(op) \
        static inline VectorType op(const VectorType a) Vc_PURE { return CAT(_mm_##op##_, SUFFIX)(a); }
#define OP(op) \
        static inline VectorType op(const VectorType a, const VectorType b) Vc_PURE { return CAT(_mm_##op##_ , SUFFIX)(a, b); }
#define OP_(op) \
        static inline VectorType op(const VectorType a, const VectorType b) Vc_PURE { return CAT(_mm_##op    , SUFFIX)(a, b); }
#define OPx(op, op2) \
        static inline VectorType op(const VectorType a, const VectorType b) Vc_PURE { return CAT(_mm_##op2##_, SUFFIX)(a, b); }
#define OPcmp(op) \
        static inline VectorType cmp##op(const VectorType a, const VectorType b) Vc_PURE { return CAT(_mm_cmp##op##_, SUFFIX)(a, b); }
#define OP_CAST_(op) \
        static inline VectorType op(const VectorType a, const VectorType b) Vc_PURE { return CAT(_mm_castps_, SUFFIX)( \
            _mm_##op##ps(CAT(CAT(_mm_cast, SUFFIX), _ps)(a), \
              CAT(CAT(_mm_cast, SUFFIX), _ps)(b))); \
        }
#define MINMAX \
        static inline VectorType min(VectorType a, VectorType b) Vc_PURE { return CAT(_mm_min_, SUFFIX)(a, b); } \
        static inline VectorType max(VectorType a, VectorType b) Vc_PURE { return CAT(_mm_max_, SUFFIX)(a, b); }

        template<> struct VectorHelper<double> {
            typedef _M128D VectorType;
            typedef double EntryType;
#define SUFFIX pd

            OP_(or_) OP_(and_) OP_(xor_)
            static inline VectorType notMaskedToZero(VectorType a, _M128 mask) Vc_PURE { return CAT(_mm_and_, SUFFIX)(_mm_castps_pd(mask), a); }
            static inline VectorType set(const double a) Vc_PURE { return CAT(_mm_set1_, SUFFIX)(a); }
            static inline VectorType set(const double a, const double b) Vc_PURE { return CAT(_mm_set_, SUFFIX)(a, b); }
            static inline VectorType zero() Vc_PURE { return CAT(_mm_setzero_, SUFFIX)(); }
            static inline VectorType one()  Vc_PURE { return CAT(_mm_setone_, SUFFIX)(); }// set(1.); }

#ifdef VC_IMPL_FMA4
#warning "Argument order of _mm_macc_pd unknown!"
            static inline void fma(VectorType &v1, VectorType v2, VectorType v3) {
                v1 = _mm_macc_pd(v1, v2, v3);
            }
#else
            static inline void fma(VectorType &v1, VectorType v2, VectorType v3) {
                VectorType h1 = _mm_and_pd(v1, _mm_load_pd(reinterpret_cast<const double *>(&c_general::highMaskDouble)));
                VectorType h2 = _mm_and_pd(v2, _mm_load_pd(reinterpret_cast<const double *>(&c_general::highMaskDouble)));
#if defined(VC_GCC) && VC_GCC < 0x40703
                // GCC before 4.7.3 uses an incorrect optimization where it replaces the subtraction with an andnot
                // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=54703
                asm("":"+x"(h1), "+x"(h2));
#endif
                const VectorType l1 = _mm_sub_pd(v1, h1);
                const VectorType l2 = _mm_sub_pd(v2, h2);
                const VectorType ll = mul(l1, l2);
                const VectorType lh = add(mul(l1, h2), mul(h1, l2));
                const VectorType hh = mul(h1, h2);
                // ll < lh < hh for all entries is certain
                const VectorType lh_lt_v3 = cmplt(abs(lh), abs(v3)); // |lh| < |v3|
                const VectorType b = _mm_blendv_pd(v3, lh, lh_lt_v3);
                const VectorType c = _mm_blendv_pd(lh, v3, lh_lt_v3);
                v1 = add(add(ll, b), add(c, hh));
            }
#endif
            static inline VectorType mul(VectorType a, VectorType b, _M128 _mask) Vc_PURE {
                _M128D mask = _mm_castps_pd(_mask);
                return _mm_or_pd(
                    _mm_and_pd(mask, _mm_mul_pd(a, b)),
                    _mm_andnot_pd(mask, a)
                    );
            }

            OP(add) OP(sub) OP(mul)
            OPcmp(eq) OPcmp(neq)
            OPcmp(lt) OPcmp(nlt)
            OPcmp(le) OPcmp(nle)

            OP1(sqrt)
            static inline VectorType rsqrt(VectorType x) Vc_PURE {
                return _mm_div_pd(one(), sqrt(x));
            }
            static inline VectorType reciprocal(VectorType x) Vc_PURE {
                return _mm_div_pd(one(), x);
            }
            static inline VectorType isNaN(VectorType x) Vc_PURE {
                return _mm_cmpunord_pd(x, x);
            }
            static inline VectorType isFinite(VectorType x) Vc_PURE {
                return _mm_cmpord_pd(x, _mm_mul_pd(zero(), x));
            }
            static inline VectorType abs(const VectorType a) Vc_PURE {
                return CAT(_mm_and_, SUFFIX)(a, _mm_setabsmask_pd());
            }

            MINMAX
            static inline EntryType min(VectorType a) Vc_PURE {
                a = _mm_min_sd(a, _mm_unpackhi_pd(a, a));
                return _mm_cvtsd_f64(a);
            }
            static inline EntryType max(VectorType a) Vc_PURE {
                a = _mm_max_sd(a, _mm_unpackhi_pd(a, a));
                return _mm_cvtsd_f64(a);
            }
            static inline EntryType mul(VectorType a) Vc_PURE {
                a = _mm_mul_sd(a, _mm_shuffle_pd(a, a, _MM_SHUFFLE2(0, 1)));
                return _mm_cvtsd_f64(a);
            }
            static inline EntryType add(VectorType a) Vc_PURE {
                a = _mm_add_sd(a, _mm_shuffle_pd(a, a, _MM_SHUFFLE2(0, 1)));
                return _mm_cvtsd_f64(a);
            }
#undef SUFFIX
            static inline VectorType round(VectorType a) Vc_PURE {
#if VC_IMPL_SSE4_1
                return _mm_round_pd(a, _MM_FROUND_NINT);
#else
                //XXX: slow: _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
                return _mm_cvtepi32_pd(_mm_cvtpd_epi32(a));
#endif
            }
        };

        template<> struct VectorHelper<float> {
            typedef float EntryType;
            typedef _M128 VectorType;
#define SUFFIX ps

            OP_(or_) OP_(and_) OP_(xor_)
            static inline VectorType notMaskedToZero(VectorType a, _M128 mask) Vc_PURE { return CAT(_mm_and_, SUFFIX)(mask, a); }
            static inline VectorType set(const float a) Vc_PURE { return CAT(_mm_set1_, SUFFIX)(a); }
            static inline VectorType set(const float a, const float b, const float c, const float d) Vc_PURE { return CAT(_mm_set_, SUFFIX)(a, b, c, d); }
            static inline VectorType zero() Vc_PURE { return CAT(_mm_setzero_, SUFFIX)(); }
            static inline VectorType one()  Vc_PURE { return CAT(_mm_setone_, SUFFIX)(); }// set(1.f); }
            static inline _M128 concat(_M128D a, _M128D b) Vc_PURE { return _mm_movelh_ps(_mm_cvtpd_ps(a), _mm_cvtpd_ps(b)); }

#ifdef VC_IMPL_FMA4
#warning "Argument order of _mm_macc_ps unknown!"
            static inline void fma(VectorType &v1, VectorType v2, VectorType v3) {
                v1 = _mm_macc_ps(v1, v2, v3);
            }
#else
            static inline void fma(VectorType &v1, VectorType v2, VectorType v3) {
                __m128d v1_0 = _mm_cvtps_pd(v1);
                __m128d v1_1 = _mm_cvtps_pd(_mm_movehl_ps(v1, v1));
                __m128d v2_0 = _mm_cvtps_pd(v2);
                __m128d v2_1 = _mm_cvtps_pd(_mm_movehl_ps(v2, v2));
                __m128d v3_0 = _mm_cvtps_pd(v3);
                __m128d v3_1 = _mm_cvtps_pd(_mm_movehl_ps(v3, v3));
                v1 = _mm_movelh_ps(
                        _mm_cvtpd_ps(_mm_add_pd(_mm_mul_pd(v1_0, v2_0), v3_0)),
                        _mm_cvtpd_ps(_mm_add_pd(_mm_mul_pd(v1_1, v2_1), v3_1)));
            }
#endif
            static inline VectorType mul(VectorType a, VectorType b, _M128 mask) Vc_PURE {
                return _mm_or_ps(
                    _mm_and_ps(mask, _mm_mul_ps(a, b)),
                    _mm_andnot_ps(mask, a)
                    );
            }

            OP(add) OP(sub) OP(mul)
            OPcmp(eq) OPcmp(neq)
            OPcmp(lt) OPcmp(nlt)
            OPcmp(le) OPcmp(nle)

            OP1(sqrt) OP1(rsqrt)
            static inline VectorType isNaN(VectorType x) Vc_PURE {
                return _mm_cmpunord_ps(x, x);
            }
            static inline VectorType isFinite(VectorType x) Vc_PURE {
                return _mm_cmpord_ps(x, _mm_mul_ps(zero(), x));
            }
            static inline VectorType reciprocal(VectorType x) Vc_PURE {
                return _mm_rcp_ps(x);
            }
            static inline VectorType abs(const VectorType a) Vc_PURE {
                return CAT(_mm_and_, SUFFIX)(a, _mm_setabsmask_ps());
            }

            MINMAX
            static inline EntryType min(VectorType a) Vc_PURE {
                a = _mm_min_ps(a, _mm_movehl_ps(a, a));   // a = min(a0, a2), min(a1, a3), min(a2, a2), min(a3, a3)
                a = _mm_min_ss(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1))); // a = min(a0, a1), a1, a2, a3
                return _mm_cvtss_f32(a);
            }
            static inline EntryType max(VectorType a) Vc_PURE {
                a = _mm_max_ps(a, _mm_movehl_ps(a, a));   // a = max(a0, a2), max(a1, a3), max(a2, a2), max(a3, a3)
                a = _mm_max_ss(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(1, 1, 1, 1))); // a = max(a0, a1), a1, a2, a3
                return _mm_cvtss_f32(a);
            }
            static inline EntryType mul(VectorType a) Vc_PURE {
                a = _mm_mul_ps(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 1, 2, 3)));
                a = _mm_mul_ss(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 2, 0, 1)));
                return _mm_cvtss_f32(a);
            }
            static inline EntryType add(VectorType a) Vc_PURE {
                a = _mm_add_ps(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 1, 2, 3)));
                a = _mm_add_ss(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 2, 0, 1)));
                return _mm_cvtss_f32(a);
            }
#undef SUFFIX
            static inline VectorType round(VectorType a) Vc_PURE {
#if VC_IMPL_SSE4_1
                return _mm_round_ps(a, _MM_FROUND_NINT);
#else
                //XXX slow: _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
                return _mm_cvtepi32_ps(_mm_cvtps_epi32(a));
#endif
            }
        };

        template<> struct VectorHelper<float8> {
            typedef float EntryType;
            typedef M256 VectorType;
#ifdef VC_PASSING_VECTOR_BY_VALUE_IS_BROKEN
            typedef const VectorType &VectorTypeArg;
#else
            typedef const VectorType VectorTypeArg;
#endif

            static inline VectorType set(const float a) Vc_PURE {
                const _M128 x = _mm_set1_ps(a);
                return VectorType::create(x, x);
            }
            static inline VectorType set(const float a, const float b, const float c, const float d) Vc_PURE {
                const _M128 x = _mm_set_ps(a, b, c, d);
                return VectorType::create(x, x);
            }
            static inline VectorType set(const float a, const float b, const float c, const float d,
                    const float e, const float f, const float g, const float h) Vc_PURE {
                return VectorType::create(_mm_set_ps(a, b, c, d), _mm_set_ps(e, f, g, h));
            }
            static inline VectorType zero() Vc_PURE { return VectorType::create(_mm_setzero_ps(), _mm_setzero_ps()); }
            static inline VectorType one()  Vc_PURE { return set(1.f); }

#define REUSE_FLOAT_IMPL1(fun) \
            static inline VectorType fun(VectorTypeArg x) Vc_PURE { \
                return VectorType::create(VectorHelper<float>::fun(x[0]), VectorHelper<float>::fun(x[1])); \
            }
#define REUSE_FLOAT_IMPL2(fun) \
            static inline VectorType fun(VectorTypeArg x, VectorTypeArg y) Vc_PURE { \
                return VectorType::create(VectorHelper<float>::fun(x[0], y[0]), VectorHelper<float>::fun(x[1], y[1])); \
            }
#define REUSE_FLOAT_IMPL3(fun) \
            static inline VectorType fun(VectorTypeArg x, VectorTypeArg y, VectorTypeArg z) Vc_PURE { \
                return VectorType::create(VectorHelper<float>::fun(x[0], y[0], z[0]), VectorHelper<float>::fun(x[1], y[1], z[1])); \
            }
            REUSE_FLOAT_IMPL1(reciprocal)
            REUSE_FLOAT_IMPL1(sqrt)
            REUSE_FLOAT_IMPL1(rsqrt)
            REUSE_FLOAT_IMPL1(isNaN)
            REUSE_FLOAT_IMPL1(isFinite)
            REUSE_FLOAT_IMPL1(abs)
            REUSE_FLOAT_IMPL1(round)

            REUSE_FLOAT_IMPL2(and_)
            REUSE_FLOAT_IMPL2(or_)
            REUSE_FLOAT_IMPL2(xor_)
            REUSE_FLOAT_IMPL2(notMaskedToZero)
            REUSE_FLOAT_IMPL2(add)
            REUSE_FLOAT_IMPL2(sub)
            REUSE_FLOAT_IMPL2(mul)
            REUSE_FLOAT_IMPL2(cmple)
            REUSE_FLOAT_IMPL2(cmpnle)
            REUSE_FLOAT_IMPL2(cmplt)
            REUSE_FLOAT_IMPL2(cmpnlt)
            REUSE_FLOAT_IMPL2(cmpeq)
            REUSE_FLOAT_IMPL2(cmpneq)
            REUSE_FLOAT_IMPL2(min)
            REUSE_FLOAT_IMPL2(max)

            static inline EntryType min(VectorTypeArg a) Vc_PURE {
                return VectorHelper<float>::min(VectorHelper<float>::min(a[0], a[1]));
            }
            static inline EntryType max(VectorTypeArg a) Vc_PURE {
                return VectorHelper<float>::max(VectorHelper<float>::max(a[0], a[1]));
            }
            static inline EntryType mul(VectorTypeArg a) Vc_PURE {
                return VectorHelper<float>::mul(VectorHelper<float>::mul(a[0], a[1]));
            }
            static inline EntryType add(VectorTypeArg a) Vc_PURE {
                return VectorHelper<float>::add(VectorHelper<float>::add(a[0], a[1]));
            }

            static inline void fma(VectorType &a, VectorTypeArg b, VectorTypeArg c) {
                VectorHelper<float>::fma(a[0], b[0], c[0]);
                VectorHelper<float>::fma(a[1], b[1], c[1]);
            }
            REUSE_FLOAT_IMPL3(mul)
#undef REUSE_FLOAT_IMPL3
#undef REUSE_FLOAT_IMPL2
#undef REUSE_FLOAT_IMPL1
        };

        template<> struct VectorHelper<int> {
            typedef int EntryType;
            typedef _M128I VectorType;
#define SUFFIX si128

            OP_(or_) OP_(and_) OP_(xor_)
            static inline VectorType zero() Vc_PURE { return CAT(_mm_setzero_, SUFFIX)(); }
            static inline VectorType notMaskedToZero(VectorType a, _M128 mask) Vc_PURE { return CAT(_mm_and_, SUFFIX)(_mm_castps_si128(mask), a); }
#undef SUFFIX
#define SUFFIX epi32
            static inline VectorType one() Vc_PURE { return CAT(_mm_setone_, SUFFIX)(); }

            static inline VectorType set(const int a) Vc_PURE { return CAT(_mm_set1_, SUFFIX)(a); }
            static inline VectorType set(const int a, const int b, const int c, const int d) Vc_PURE { return CAT(_mm_set_, SUFFIX)(a, b, c, d); }

            static inline void fma(VectorType &v1, VectorType v2, VectorType v3) { v1 = add(mul(v1, v2), v3); }

            static inline VectorType shiftLeft(VectorType a, int shift) {
                return CAT(_mm_slli_, SUFFIX)(a, shift);
            }
            static inline VectorType shiftRight(VectorType a, int shift) {
                return CAT(_mm_srai_, SUFFIX)(a, shift);
            }
            OP1(abs)

            MINMAX
            static inline EntryType min(VectorType a) Vc_PURE {
                a = min(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                // using lo_epi16 for speed here
                a = min(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                return _mm_cvtsi128_si32(a);
            }
            static inline EntryType max(VectorType a) Vc_PURE {
                a = max(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                // using lo_epi16 for speed here
                a = max(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                return _mm_cvtsi128_si32(a);
            }
            static inline EntryType add(VectorType a) Vc_PURE {
                a = add(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = add(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                return _mm_cvtsi128_si32(a);
            }
#if VC_IMPL_SSE4_1
            static inline VectorType mul(VectorType a, VectorType b) Vc_PURE { return _mm_mullo_epi32(a, b); }
            static inline EntryType mul(VectorType a) Vc_PURE {
                a = mul(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = mul(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                return _mm_cvtsi128_si32(a);
            }
#else
            static inline VectorType mul(const VectorType a, const VectorType b) Vc_PURE {
                const VectorType aShift = _mm_srli_si128(a, 4);
                const VectorType ab02 = _mm_mul_epu32(a, b); // [a0 * b0, a2 * b2]
                const VectorType bShift = _mm_srli_si128(b, 4);
                const VectorType ab13 = _mm_mul_epu32(aShift, bShift); // [a1 * b1, a3 * b3]
                return _mm_unpacklo_epi32(_mm_shuffle_epi32(ab02, 8), _mm_shuffle_epi32(ab13, 8));
            }
#endif
            static inline VectorType mul(const VectorType a, const VectorType b, _M128 _mask) Vc_PURE {
                return _mm_blendv_epi8(a, mul(a, b), _mm_castps_si128(_mask));
            }

            OP(add) OP(sub)
            OPcmp(eq)
            OPcmp(lt)
            OPcmp(gt)
            static inline VectorType cmpneq(const VectorType a, const VectorType b) Vc_PURE { _M128I x = cmpeq(a, b); return _mm_andnot_si128(x, _mm_setallone_si128()); }
            static inline VectorType cmpnlt(const VectorType a, const VectorType b) Vc_PURE { _M128I x = cmplt(a, b); return _mm_andnot_si128(x, _mm_setallone_si128()); }
            static inline VectorType cmple (const VectorType a, const VectorType b) Vc_PURE { _M128I x = cmpgt(a, b); return _mm_andnot_si128(x, _mm_setallone_si128()); }
            static inline VectorType cmpnle(const VectorType a, const VectorType b) Vc_PURE { return cmpgt(a, b); }
#undef SUFFIX
            static inline VectorType round(VectorType a) Vc_PURE { return a; }
        };

        template<> struct VectorHelper<unsigned int> {
            typedef unsigned int EntryType;
            typedef _M128I VectorType;
#define SUFFIX si128
            OP_CAST_(or_) OP_CAST_(and_) OP_CAST_(xor_)
            static inline VectorType zero() Vc_PURE { return CAT(_mm_setzero_, SUFFIX)(); }
            static inline VectorType notMaskedToZero(VectorType a, _M128 mask) Vc_PURE { return CAT(_mm_and_, SUFFIX)(_mm_castps_si128(mask), a); }

#undef SUFFIX
#define SUFFIX epu32
            static inline VectorType one() Vc_PURE { return CAT(_mm_setone_, SUFFIX)(); }

            MINMAX
            static inline EntryType min(VectorType a) Vc_PURE {
                a = min(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                // using lo_epi16 for speed here
                a = min(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                return _mm_cvtsi128_si32(a);
            }
            static inline EntryType max(VectorType a) Vc_PURE {
                a = max(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                // using lo_epi16 for speed here
                a = max(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                return _mm_cvtsi128_si32(a);
            }
            static inline EntryType mul(VectorType a) Vc_PURE {
                a = mul(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                // using lo_epi16 for speed here
                a = mul(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                return _mm_cvtsi128_si32(a);
            }
            static inline EntryType add(VectorType a) Vc_PURE {
                a = add(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                // using lo_epi16 for speed here
                a = add(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                return _mm_cvtsi128_si32(a);
            }

            static inline void fma(VectorType &v1, VectorType v2, VectorType v3) { v1 = add(mul(v1, v2), v3); }

            static inline VectorType mul(const VectorType a, const VectorType b, _M128 _mask) Vc_PURE {
                return _mm_blendv_epi8(a, mul(a, b), _mm_castps_si128(_mask));
            }
            static inline VectorType mul(const VectorType a, const VectorType b) Vc_PURE {
                return VectorHelper<int>::mul(a, b);
            }
//X             template<unsigned int b> static inline VectorType mul(const VectorType a) Vc_PURE {
//X                 switch (b) {
//X                     case    0: return zero();
//X                     case    1: return a;
//X                     case    2: return _mm_slli_epi32(a,  1);
//X                     case    4: return _mm_slli_epi32(a,  2);
//X                     case    8: return _mm_slli_epi32(a,  3);
//X                     case   16: return _mm_slli_epi32(a,  4);
//X                     case   32: return _mm_slli_epi32(a,  5);
//X                     case   64: return _mm_slli_epi32(a,  6);
//X                     case  128: return _mm_slli_epi32(a,  7);
//X                     case  256: return _mm_slli_epi32(a,  8);
//X                     case  512: return _mm_slli_epi32(a,  9);
//X                     case 1024: return _mm_slli_epi32(a, 10);
//X                     case 2048: return _mm_slli_epi32(a, 11);
//X                 }
//X                 return mul(a, set(b));
//X             }

#undef SUFFIX
#define SUFFIX epi32
            static inline VectorType shiftLeft(VectorType a, int shift) {
                return CAT(_mm_slli_, SUFFIX)(a, shift);
            }
            static inline VectorType shiftRight(VectorType a, int shift) {
                return CAT(_mm_srli_, SUFFIX)(a, shift);
            }
            static inline VectorType set(const unsigned int a) Vc_PURE { return CAT(_mm_set1_, SUFFIX)(a); }
            static inline VectorType set(const unsigned int a, const unsigned int b, const unsigned int c, const unsigned int d) Vc_PURE { return CAT(_mm_set_, SUFFIX)(a, b, c, d); }

            OP(add) OP(sub)
            OPcmp(eq)
            static inline VectorType cmpneq(const VectorType a, const VectorType b) Vc_PURE { return _mm_andnot_si128(cmpeq(a, b), _mm_setallone_si128()); }

#ifndef USE_INCORRECT_UNSIGNED_COMPARE
            static inline VectorType cmplt(const VectorType a, const VectorType b) Vc_PURE {
                return _mm_cmplt_epu32(a, b);
            }
            static inline VectorType cmpgt(const VectorType a, const VectorType b) Vc_PURE {
                return _mm_cmpgt_epu32(a, b);
            }
#else
            OPcmp(lt)
            OPcmp(gt)
#endif
            static inline VectorType cmpnlt(const VectorType a, const VectorType b) Vc_PURE { return _mm_andnot_si128(cmplt(a, b), _mm_setallone_si128()); }
            static inline VectorType cmple (const VectorType a, const VectorType b) Vc_PURE { return _mm_andnot_si128(cmpgt(a, b), _mm_setallone_si128()); }
            static inline VectorType cmpnle(const VectorType a, const VectorType b) Vc_PURE { return cmpgt(a, b); }

#undef SUFFIX
            static inline VectorType round(VectorType a) Vc_PURE { return a; }
        };

        template<> struct VectorHelper<signed short> {
            typedef _M128I VectorType;
            typedef signed short EntryType;
#define SUFFIX si128

            OP_(or_) OP_(and_) OP_(xor_)
            static inline VectorType zero() Vc_PURE { return CAT(_mm_setzero_, SUFFIX)(); }
            static inline VectorType notMaskedToZero(VectorType a, _M128 mask) Vc_PURE { return CAT(_mm_and_, SUFFIX)(_mm_castps_si128(mask), a); }
            static inline _M128I concat(_M128I a, _M128I b) Vc_PURE { return _mm_packs_epi32(a, b); }
            static inline _M128I expand0(_M128I x) Vc_PURE { return _mm_srai_epi32(_mm_unpacklo_epi16(x, x), 16); }
            static inline _M128I expand1(_M128I x) Vc_PURE { return _mm_srai_epi32(_mm_unpackhi_epi16(x, x), 16); }

#undef SUFFIX
#define SUFFIX epi16
            static inline VectorType one() Vc_PURE { return CAT(_mm_setone_, SUFFIX)(); }

            static inline VectorType shiftLeft(VectorType a, int shift) {
                return CAT(_mm_slli_, SUFFIX)(a, shift);
            }
            static inline VectorType shiftRight(VectorType a, int shift) {
                return CAT(_mm_srai_, SUFFIX)(a, shift);
            }
            static inline VectorType set(const EntryType a) Vc_PURE { return CAT(_mm_set1_, SUFFIX)(a); }
            static inline VectorType set(const EntryType a, const EntryType b, const EntryType c, const EntryType d,
                    const EntryType e, const EntryType f, const EntryType g, const EntryType h) Vc_PURE {
                return CAT(_mm_set_, SUFFIX)(a, b, c, d, e, f, g, h);
            }

            static inline void fma(VectorType &v1, VectorType v2, VectorType v3) {
                v1 = add(mul(v1, v2), v3); }

            OP1(abs)

            static inline VectorType mul(VectorType a, VectorType b, _M128 _mask) Vc_PURE {
                _M128I mask = _mm_castps_si128(_mask);
                return _mm_or_si128(
                    _mm_and_si128(mask, mul(a, b)),
                    _mm_andnot_si128(mask, a)
                    );
            }
            OPx(mul, mullo)
            OP(min) OP(max)
            static inline EntryType min(VectorType a) Vc_PURE {
                // reminder: _MM_SHUFFLE(3, 2, 1, 0) means "no change"
                a = min(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = min(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = min(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 1, 1, 1)));
                return _mm_cvtsi128_si32(a); // & 0xffff is implicit
            }
            static inline EntryType max(VectorType a) Vc_PURE {
                // reminder: _MM_SHUFFLE(3, 2, 1, 0) means "no change"
                a = max(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = max(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = max(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 1, 1, 1)));
                return _mm_cvtsi128_si32(a); // & 0xffff is implicit
            }
            static inline EntryType mul(VectorType a) Vc_PURE {
                a = mul(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = mul(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = mul(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 1, 1, 1)));
                return _mm_cvtsi128_si32(a); // & 0xffff is implicit
            }
            static inline EntryType add(VectorType a) Vc_PURE {
                a = add(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = add(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = add(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 1, 1, 1)));
                return _mm_cvtsi128_si32(a); // & 0xffff is implicit
            }

            OP(add) OP(sub)
            OPcmp(eq)
            OPcmp(lt)
            OPcmp(gt)
            static inline VectorType cmpneq(const VectorType a, const VectorType b) Vc_PURE { _M128I x = cmpeq(a, b); return _mm_andnot_si128(x, _mm_setallone_si128()); }
            static inline VectorType cmpnlt(const VectorType a, const VectorType b) Vc_PURE { _M128I x = cmplt(a, b); return _mm_andnot_si128(x, _mm_setallone_si128()); }
            static inline VectorType cmple (const VectorType a, const VectorType b) Vc_PURE { _M128I x = cmpgt(a, b); return _mm_andnot_si128(x, _mm_setallone_si128()); }
            static inline VectorType cmpnle(const VectorType a, const VectorType b) Vc_PURE { return cmpgt(a, b); }
#undef SUFFIX
            static inline VectorType round(VectorType a) Vc_PURE { return a; }
        };

        template<> struct VectorHelper<unsigned short> {
            typedef _M128I VectorType;
            typedef unsigned short EntryType;
#define SUFFIX si128
            OP_CAST_(or_) OP_CAST_(and_) OP_CAST_(xor_)
            static inline VectorType zero() Vc_PURE { return CAT(_mm_setzero_, SUFFIX)(); }
            static inline VectorType notMaskedToZero(VectorType a, _M128 mask) Vc_PURE { return CAT(_mm_and_, SUFFIX)(_mm_castps_si128(mask), a); }
#if VC_IMPL_SSE4_1
            static inline _M128I concat(_M128I a, _M128I b) Vc_PURE { return _mm_packus_epi32(a, b); }
#else
            // XXX too bad, but this is broken without SSE 4.1
            static inline _M128I concat(_M128I a, _M128I b) Vc_PURE { return _mm_packs_epi32(a, b); }
#endif
            static inline _M128I expand0(_M128I x) Vc_PURE { return _mm_srli_epi32(_mm_unpacklo_epi16(x, x), 16); }
            static inline _M128I expand1(_M128I x) Vc_PURE { return _mm_srli_epi32(_mm_unpackhi_epi16(x, x), 16); }

#undef SUFFIX
#define SUFFIX epu16
            static inline VectorType one() Vc_PURE { return CAT(_mm_setone_, SUFFIX)(); }

            static inline VectorType mul(VectorType a, VectorType b, _M128 _mask) Vc_PURE {
                _M128I mask = _mm_castps_si128(_mask);
                return _mm_or_si128(
                    _mm_and_si128(mask, mul(a, b)),
                    _mm_andnot_si128(mask, a)
                    );
            }
//X             template<unsigned int b> static inline VectorType mul(const VectorType a) Vc_PURE {
//X                 switch (b) {
//X                     case    0: return zero();
//X                     case    1: return a;
//X                     case    2: return _mm_slli_epi16(a,  1);
//X                     case    4: return _mm_slli_epi16(a,  2);
//X                     case    8: return _mm_slli_epi16(a,  3);
//X                     case   16: return _mm_slli_epi16(a,  4);
//X                     case   32: return _mm_slli_epi16(a,  5);
//X                     case   64: return _mm_slli_epi16(a,  6);
//X                     case  128: return _mm_slli_epi16(a,  7);
//X                     case  256: return _mm_slli_epi16(a,  8);
//X                     case  512: return _mm_slli_epi16(a,  9);
//X                     case 1024: return _mm_slli_epi16(a, 10);
//X                     case 2048: return _mm_slli_epi16(a, 11);
//X                 }
//X                 return mul(a, set(b));
//X             }
#if !defined(USE_INCORRECT_UNSIGNED_COMPARE) || VC_IMPL_SSE4_1
            OP(min) OP(max)
#endif
#undef SUFFIX
#define SUFFIX epi16
            static inline VectorType shiftLeft(VectorType a, int shift) {
                return CAT(_mm_slli_, SUFFIX)(a, shift);
            }
            static inline VectorType shiftRight(VectorType a, int shift) {
                return CAT(_mm_srli_, SUFFIX)(a, shift);
            }

            static inline void fma(VectorType &v1, VectorType v2, VectorType v3) { v1 = add(mul(v1, v2), v3); }

            OPx(mul, mullo) // should work correctly for all values
#if defined(USE_INCORRECT_UNSIGNED_COMPARE) && !defined(VC_IMPL_SSE4_1)
            OP(min) OP(max) // XXX breaks for values with MSB set
#endif
            static inline EntryType min(VectorType a) Vc_PURE {
                // reminder: _MM_SHUFFLE(3, 2, 1, 0) means "no change"
                a = min(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = min(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = min(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 1, 1, 1)));
                return _mm_cvtsi128_si32(a); // & 0xffff is implicit
            }
            static inline EntryType max(VectorType a) Vc_PURE {
                // reminder: _MM_SHUFFLE(3, 2, 1, 0) means "no change"
                a = max(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = max(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = max(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 1, 1, 1)));
                return _mm_cvtsi128_si32(a); // & 0xffff is implicit
            }
            static inline EntryType mul(VectorType a) Vc_PURE {
                // reminder: _MM_SHUFFLE(3, 2, 1, 0) means "no change"
                a = mul(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = mul(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = mul(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 1, 1, 1)));
                return _mm_cvtsi128_si32(a); // & 0xffff is implicit
            }
            static inline EntryType add(VectorType a) Vc_PURE {
                // reminder: _MM_SHUFFLE(3, 2, 1, 0) means "no change"
                a = add(a, _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = add(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 0, 3, 2)));
                a = add(a, _mm_shufflelo_epi16(a, _MM_SHUFFLE(1, 1, 1, 1)));
                return _mm_cvtsi128_si32(a); // & 0xffff is implicit
            }
            static inline VectorType set(const EntryType a) Vc_PURE { return CAT(_mm_set1_, SUFFIX)(a); }
            static inline VectorType set(const EntryType a, const EntryType b, const EntryType c,
                    const EntryType d, const EntryType e, const EntryType f,
                    const EntryType g, const EntryType h) Vc_PURE {
                return CAT(_mm_set_, SUFFIX)(a, b, c, d, e, f, g, h);
            }

            OP(add) OP(sub)
            OPcmp(eq)
            static inline VectorType cmpneq(const VectorType a, const VectorType b) Vc_PURE { return _mm_andnot_si128(cmpeq(a, b), _mm_setallone_si128()); }

#ifndef USE_INCORRECT_UNSIGNED_COMPARE
            static inline VectorType cmplt(const VectorType a, const VectorType b) Vc_PURE {
                return _mm_cmplt_epu16(a, b);
            }
            static inline VectorType cmpgt(const VectorType a, const VectorType b) Vc_PURE {
                return _mm_cmpgt_epu16(a, b);
            }
#else
            OPcmp(lt)
            OPcmp(gt)
#endif
            static inline VectorType cmpnlt(const VectorType a, const VectorType b) Vc_PURE { return _mm_andnot_si128(cmplt(a, b), _mm_setallone_si128()); }
            static inline VectorType cmple (const VectorType a, const VectorType b) Vc_PURE { return _mm_andnot_si128(cmpgt(a, b), _mm_setallone_si128()); }
            static inline VectorType cmpnle(const VectorType a, const VectorType b) Vc_PURE { return cmpgt(a, b); }
#undef SUFFIX
            static inline VectorType round(VectorType a) Vc_PURE { return a; }
        };
#undef OP1
#undef OP
#undef OP_
#undef OPx
#undef OPcmp

} // namespace SSE
} // namespace Vc

#include "vectorhelper.tcc"

#endif // SSE_VECTORHELPER_H
