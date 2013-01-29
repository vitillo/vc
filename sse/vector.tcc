/*  This file is part of the Vc library.

    Copyright (C) 2010-2012 Matthias Kretz <kretz@kde.org>

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

#include "limits.h"
#include "../common/bitscanintrinsics.h"
#include "macros.h"

namespace Vc
{
ALIGN(64) extern unsigned int RandomState[16];

namespace SSE
{

template<typename T, int Size> static inline const T *_IndexesFromZero() {
    if (Size == 4) {
        return reinterpret_cast<const T *>(_IndexesFromZero4);
    } else if (Size == 8) {
        return reinterpret_cast<const T *>(_IndexesFromZero8);
    } else if (Size == 16) {
        return reinterpret_cast<const T *>(_IndexesFromZero16);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
// constants {{{1
template<typename T> inline Vector<T>::Vector(VectorSpecialInitializerZero::ZEnum)
    : d(VectorHelper<VectorType>::zero())
{
}

template<typename T> inline Vector<T>::Vector(VectorSpecialInitializerOne::OEnum)
    : d(VectorHelper<T>::one())
{
}

template<typename T> inline Vector<T>::Vector(VectorSpecialInitializerIndexesFromZero::IEnum)
    : d(VectorHelper<VectorType>::load(_IndexesFromZero<EntryType, Size>(), Aligned))
{
}

template<typename T> inline Vector<T> Vector<T>::Zero()
{
    return VectorHelper<VectorType>::zero();
}

template<typename T> inline Vector<T> Vector<T>::One()
{
    return VectorHelper<T>::one();
}

template<typename T> inline Vector<T> Vector<T>::IndexesFromZero()
{
    return VectorHelper<VectorType>::load(_IndexesFromZero<EntryType, Size>(), Aligned);
}

// conversion/casts {{{1
template<typename T> template<typename OtherT> inline Vc_INTRINSIC Vector<T>::Vector(const Vector<OtherT> &x)
    : d(StaticCastHelper<OtherT, T>::cast(x.data()))
{
}

template<> template<> inline Vc_INTRINSIC short_v &Vector<short>::operator=(const ushort_v &x) {
    data() = StaticCastHelper<unsigned short, short>::cast(x.data()); return *this;
}
template<> template<> inline Vc_INTRINSIC ushort_v &Vector<unsigned short>::operator=(const short_v &x) {
    data() = StaticCastHelper<short, unsigned short>::cast(x.data()); return *this;
}
template<> template<> inline Vc_INTRINSIC int_v &Vector<int>::operator=(const uint_v &x) {
    data() = StaticCastHelper<unsigned int, int>::cast(x.data()); return *this;
}
template<> template<> inline Vc_INTRINSIC uint_v &Vector<unsigned int>::operator=(const int_v &x) {
    data() = StaticCastHelper<int, unsigned int>::cast(x.data()); return *this;
}

// broadcasts {{{1
template<typename T> inline Vector<T>::Vector(EntryType a)
    : d(VectorHelper<T>::set(a))
{
}

///////////////////////////////////////////////////////////////////////////////////////////
// load ctors {{{1
template<typename T> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const EntryType *x) { load(x); }
template<typename T> template<typename A> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const EntryType *x, A a) { load(x, a); }
template<typename T> template<typename OtherT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const OtherT *x) { load(x); }
template<typename T> template<typename OtherT, typename A> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const OtherT *x, A a) { load(x, a); }

///////////////////////////////////////////////////////////////////////////////////////////
// load member functions {{{1
template<typename T> inline void Vc_INTRINSIC Vector<T>::load(const EntryType *mem)
{
    load(mem, Aligned);
}

template<typename T> template<typename A> inline void Vc_INTRINSIC Vector<T>::load(const EntryType *mem, A align)
{
    d.v() = VectorHelper<VectorType>::load(mem, align);
}

template<typename T> template<typename OtherT> inline void Vc_INTRINSIC Vector<T>::load(const OtherT *mem)
{
    load(mem, Aligned);
}

// float8: simply use the float implementation twice {{{2
template<> template<typename OtherT, typename A> inline void Vc_INTRINSIC Vector<float8>::load(const OtherT *x, A a)
{
    d.v() = M256::create(
            Vector<float>(&x[0], a).data(),
            Vector<float>(&x[4], a).data()
            );
}

// LoadHelper {{{2
template<typename DstT, typename SrcT, typename Flags> struct LoadHelper;

// float {{{2
template<typename Flags> struct LoadHelper<float, double, Flags> {
    static inline __m128 load(const double *mem, Flags f)
    {
        return _mm_movelh_ps(_mm_cvtpd_ps(VectorHelper<__m128d>::load(&mem[0], f)),
                             _mm_cvtpd_ps(VectorHelper<__m128d>::load(&mem[2], f)));
    }
};
template<typename Flags> struct LoadHelper<float, unsigned int, Flags> {
    static inline __m128 load(const unsigned int *mem, Flags f)
    {
        return StaticCastHelper<unsigned int, float>::cast(VectorHelper<__m128i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, int, Flags> {
    static inline __m128 load(const int *mem, Flags f)
    {
        return StaticCastHelper<int, float>::cast(VectorHelper<__m128i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, unsigned short, Flags> {
    static inline __m128 load(const unsigned short *mem, Flags f)
    {
        return _mm_cvtepi32_ps(LoadHelper<int, unsigned short, Flags>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, short, Flags> {
    static inline __m128 load(const short *mem, Flags f)
    {
        return _mm_cvtepi32_ps(LoadHelper<int, short, Flags>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, unsigned char, Flags> {
    static inline __m128 load(const unsigned char *mem, Flags f)
    {
        return _mm_cvtepi32_ps(LoadHelper<int, unsigned char, Flags>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, signed char, Flags> {
    static inline __m128 load(const signed char *mem, Flags f)
    {
        return _mm_cvtepi32_ps(LoadHelper<int, signed char, Flags>::load(mem, f));
    }
};

// int {{{2
template<typename Flags> struct LoadHelper<int, unsigned int, Flags> {
    static inline __m128i load(const unsigned int *mem, Flags f)
    {
        return VectorHelper<__m128i>::load(mem, f);
    }
};
// no difference between streaming and alignment, because the
// 32/64 bit loads are not available as streaming loads, and can always be unaligned
template<typename Flags> struct LoadHelper<int, unsigned short, Flags> {
    static inline __m128i load(const unsigned short *mem, Flags)
    {
        return _mm_cvtepu16_epi32( _mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem)));
    }
};
template<typename Flags> struct LoadHelper<int, short, Flags> {
    static inline __m128i load(const short *mem, Flags)
    {
        return _mm_cvtepi16_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem)));
    }
};
template<typename Flags> struct LoadHelper<int, unsigned char, Flags> {
    static inline __m128i load(const unsigned char *mem, Flags)
    {
        return _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*reinterpret_cast<const int *>(mem)));
    }
};
template<typename Flags> struct LoadHelper<int, signed char, Flags> {
    static inline __m128i load(const signed char *mem, Flags)
    {
        return _mm_cvtepi8_epi32(_mm_cvtsi32_si128(*reinterpret_cast<const int *>(mem)));
    }
};

// unsigned int {{{2
template<typename Flags> struct LoadHelper<unsigned int, unsigned short, Flags> {
    static inline __m128i load(const unsigned short *mem, Flags)
    {
        return _mm_cvtepu16_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem)));
    }
};
template<typename Flags> struct LoadHelper<unsigned int, unsigned char, Flags> {
    static inline __m128i load(const unsigned char *mem, Flags)
    {
        return _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*reinterpret_cast<const int *>(mem)));
    }
};

// short {{{2
template<typename Flags> struct LoadHelper<short, unsigned short, Flags> {
    static inline __m128i load(const unsigned short *mem, Flags f)
    {
        return VectorHelper<__m128i>::load(mem, f);
    }
};
template<typename Flags> struct LoadHelper<short, unsigned char, Flags> {
    static inline __m128i load(const unsigned char *mem, Flags)
    {
        return _mm_cvtepu8_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem)));
    }
};
template<typename Flags> struct LoadHelper<short, signed char, Flags> {
    static inline __m128i load(const signed char *mem, Flags)
    {
        return _mm_cvtepi8_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem)));
    }
};

// unsigned short {{{2
template<typename Flags> struct LoadHelper<unsigned short, unsigned char, Flags> {
    static inline __m128i load(const unsigned char *mem, Flags)
    {
        return _mm_cvtepu8_epi16(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem)));
    }
};

// general load, implemented via LoadHelper {{{2
template<typename DstT> template<typename SrcT, typename Flags> inline void Vc_INTRINSIC Vector<DstT>::load(const SrcT *x, Flags f)
{
    d.v() = LoadHelper<DstT, SrcT, Flags>::load(x, f);
}

///////////////////////////////////////////////////////////////////////////////////////////
// expand/combine {{{1
template<typename T> inline Vector<T>::Vector(const Vector<typename CtorTypeHelper<T>::Type> *a)
    : d(VectorHelper<T>::concat(a[0].data(), a[1].data()))
{
}

template<typename T> inline void Vector<T>::expand(Vector<typename ExpandTypeHelper<T>::Type> *x) const
{
    if (Size == 8u) {
        x[0].data() = VectorHelper<T>::expand0(data());
        x[1].data() = VectorHelper<T>::expand1(data());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
// zeroing {{{1
template<typename T> inline void Vector<T>::setZero()
{
    data() = VectorHelper<VectorType>::zero();
}

template<typename T> inline void Vector<T>::setZero(const Mask &k)
{
    data() = VectorHelper<VectorType>::andnot_(mm128_reinterpret_cast<VectorType>(k.data()), data());
}

template<> inline void Vc_INTRINSIC Vector<double>::setQnan()
{
    data() = _mm_setallone_pd();
}
template<> inline void Vc_INTRINSIC Vector<double>::setQnan(Mask::Argument k)
{
    data() = _mm_or_pd(data(), k.dataD());
}
template<> inline void Vc_INTRINSIC Vector<float>::setQnan()
{
    data() = _mm_setallone_ps();
}
template<> inline void Vc_INTRINSIC Vector<float>::setQnan(Mask::Argument k)
{
    data() = _mm_or_ps(data(), k.data());
}
template<> inline void Vc_INTRINSIC Vector<float8>::setQnan()
{
    d.v()[0] = _mm_setallone_ps();
    d.v()[1] = _mm_setallone_ps();
}
template<> inline void Vc_INTRINSIC Vector<float8>::setQnan(Mask::Argument k)
{
    d.v()[0] = _mm_or_ps(d.v()[0], k.data()[0]);
    d.v()[1] = _mm_or_ps(d.v()[1], k.data()[1]);
}

///////////////////////////////////////////////////////////////////////////////////////////
// stores {{{1
template<typename T> inline void Vector<T>::store(EntryType *mem) const
{
    VectorHelper<VectorType>::store(mem, data(), Aligned);
}

template<typename T> inline void Vector<T>::store(EntryType *mem, const Mask &mask) const
{
    VectorHelper<VectorType>::store(mem, data(), mm128_reinterpret_cast<VectorType>(mask.data()), Aligned);
}

template<typename T> template<typename A> inline void Vector<T>::store(EntryType *mem, A align) const
{
    VectorHelper<VectorType>::store(mem, data(), align);
}

template<typename T> template<typename A> inline void Vector<T>::store(EntryType *mem, const Mask &mask, A align) const
{
    HV::store(mem, data(), mm128_reinterpret_cast<VectorType>(mask.data()), align);
}

///////////////////////////////////////////////////////////////////////////////////////////
// division {{{1
template<typename T> inline Vc_INTRINSIC Vc_CONST Vector<T> &WriteMaskedVector<T>::operator/=(const Vector<T> &x)
{
    return operator=(*vec / x);
}
template<> inline Vc_INTRINSIC Vc_CONST int_v &WriteMaskedVector<int>::operator/=(const int_v &x)
{
    Vc_foreach_bit (int i, mask) {
        vec->d.m(i) /= x.d.m(i);
    }
    return *vec;
}
template<> inline Vc_INTRINSIC Vc_CONST uint_v &WriteMaskedVector<unsigned int>::operator/=(const uint_v &x)
{
    Vc_foreach_bit (int i, mask) {
        vec->d.m(i) /= x.d.m(i);
    }
    return *vec;
}
template<> inline Vc_INTRINSIC Vc_CONST short_v &WriteMaskedVector<short>::operator/=(const short_v &x)
{
    Vc_foreach_bit (int i, mask) {
        vec->d.m(i) /= x.d.m(i);
    }
    return *vec;
}
template<> inline Vc_INTRINSIC Vc_CONST ushort_v &WriteMaskedVector<unsigned short>::operator/=(const ushort_v &x)
{
    Vc_foreach_bit (int i, mask) {
        vec->d.m(i) /= x.d.m(i);
    }
    return *vec;
}

template<typename T> inline Vector<T> &Vector<T>::operator/=(EntryType x)
{
    if (VectorTraits<T>::HasVectorDivision) {
        return operator/=(Vector<T>(x));
    }
    for_all_vector_entries(i,
            d.m(i) /= x;
            );
    return *this;
}

template<typename T> template<typename TT> inline Vc_PURE Vc_INTRINSIC VC_EXACT_TYPE(TT, typename DetermineEntryType<T>::Type, Vector<T>) Vector<T>::operator/(TT x) const
{
    if (VectorTraits<T>::HasVectorDivision) {
        return operator/(Vector<T>(x));
    }
    Vector<T> r;
    for_all_vector_entries(i,
            r.d.m(i) = d.m(i) / x;
            );
    return r;
}

template<typename T> inline Vector<T> &Vector<T>::operator/=(const Vector<T> &x)
{
    for_all_vector_entries(i,
            d.m(i) /= x.d.m(i);
            );
    return *this;
}

template<typename T> inline Vector<T> Vector<T>::operator/(const Vector<T> &x) const
{
    Vector<T> r;
    for_all_vector_entries(i,
            r.d.m(i) = d.m(i) / x.d.m(i);
            );
    return r;
}

template<> inline Vector<short> &Vector<short>::operator/=(const Vector<short> &x)
{
    __m128 lo = _mm_cvtepi32_ps(VectorHelper<short>::expand0(d.v()));
    __m128 hi = _mm_cvtepi32_ps(VectorHelper<short>::expand1(d.v()));
    lo = _mm_div_ps(lo, _mm_cvtepi32_ps(VectorHelper<short>::expand0(x.d.v())));
    hi = _mm_div_ps(hi, _mm_cvtepi32_ps(VectorHelper<short>::expand1(x.d.v())));
    d.v() = _mm_packs_epi32(_mm_cvtps_epi32(lo), _mm_cvtps_epi32(hi));
    return *this;
}

template<> inline Vector<short> Vc_ALWAYS_INLINE Vector<short>::operator/(const Vector<short> &x) const
{
    __m128 lo = _mm_cvtepi32_ps(VectorHelper<short>::expand0(d.v()));
    __m128 hi = _mm_cvtepi32_ps(VectorHelper<short>::expand1(d.v()));
    lo = _mm_div_ps(lo, _mm_cvtepi32_ps(VectorHelper<short>::expand0(x.d.v())));
    hi = _mm_div_ps(hi, _mm_cvtepi32_ps(VectorHelper<short>::expand1(x.d.v())));
    return _mm_packs_epi32(_mm_cvtps_epi32(lo), _mm_cvtps_epi32(hi));
}

template<> inline Vector<unsigned short> &Vector<unsigned short>::operator/=(const Vector<unsigned short> &x)
{
    __m128 lo = _mm_cvtepi32_ps(VectorHelper<short>::expand0(d.v()));
    __m128 hi = _mm_cvtepi32_ps(VectorHelper<short>::expand1(d.v()));
    lo = _mm_div_ps(lo, _mm_cvtepi32_ps(VectorHelper<short>::expand0(x.d.v())));
    hi = _mm_div_ps(hi, _mm_cvtepi32_ps(VectorHelper<short>::expand1(x.d.v())));
    d.v() = _mm_packs_epi32(_mm_cvtps_epi32(lo), _mm_cvtps_epi32(hi));
    return *this;
}

template<> inline Vector<unsigned short> Vc_ALWAYS_INLINE Vector<unsigned short>::operator/(const Vector<unsigned short> &x) const
{
    __m128 lo = _mm_cvtepi32_ps(VectorHelper<short>::expand0(d.v()));
    __m128 hi = _mm_cvtepi32_ps(VectorHelper<short>::expand1(d.v()));
    lo = _mm_div_ps(lo, _mm_cvtepi32_ps(VectorHelper<short>::expand0(x.d.v())));
    hi = _mm_div_ps(hi, _mm_cvtepi32_ps(VectorHelper<short>::expand1(x.d.v())));
    return _mm_packs_epi32(_mm_cvtps_epi32(lo), _mm_cvtps_epi32(hi));
}

template<> inline Vector<float> &Vector<float>::operator/=(const Vector<float> &x)
{
    d.v() = _mm_div_ps(d.v(), x.d.v());
    return *this;
}

template<> inline Vector<float> Vector<float>::operator/(const Vector<float> &x) const
{
    return _mm_div_ps(d.v(), x.d.v());
}

template<> inline Vector<float8> &Vector<float8>::operator/=(const Vector<float8> &x)
{
    d.v()[0] = _mm_div_ps(d.v()[0], x.d.v()[0]);
    d.v()[1] = _mm_div_ps(d.v()[1], x.d.v()[1]);
    return *this;
}

template<> inline Vector<float8> Vector<float8>::operator/(const Vector<float8> &x) const
{
    Vector<float8> r;
    r.d.v()[0] = _mm_div_ps(d.v()[0], x.d.v()[0]);
    r.d.v()[1] = _mm_div_ps(d.v()[1], x.d.v()[1]);
    return r;
}

template<> inline Vector<double> &Vector<double>::operator/=(const Vector<double> &x)
{
    d.v() = _mm_div_pd(d.v(), x.d.v());
    return *this;
}

template<> inline Vector<double> Vector<double>::operator/(const Vector<double> &x) const
{
    return _mm_div_pd(d.v(), x.d.v());
}

///////////////////////////////////////////////////////////////////////////////////////////
// operator- {{{1
template<> inline Vector<double> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<double>::operator-() const
{
    return _mm_xor_pd(d.v(), _mm_setsignmask_pd());
}
template<> inline Vector<float> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::operator-() const
{
    return _mm_xor_ps(d.v(), _mm_setsignmask_ps());
}
template<> inline Vector<float8> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float8>::operator-() const
{
    return M256::create(
            _mm_xor_ps(d.v()[0], _mm_setsignmask_ps()),
            _mm_xor_ps(d.v()[1], _mm_setsignmask_ps()));
}
template<> inline Vector<int> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::operator-() const
{
#ifdef VC_IMPL_SSSE3
    return _mm_sign_epi32(d.v(), _mm_setallone_si128());
#else
    return _mm_add_epi32(_mm_xor_si128(d.v(), _mm_setallone_si128()), _mm_setone_epi32());
#endif
}
template<> inline Vector<int> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::operator-() const
{
#ifdef VC_IMPL_SSSE3
    return _mm_sign_epi32(d.v(), _mm_setallone_si128());
#else
    return _mm_add_epi32(_mm_xor_si128(d.v(), _mm_setallone_si128()), _mm_setone_epi32());
#endif
}
template<> inline Vector<short> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<short>::operator-() const
{
#ifdef VC_IMPL_SSSE3
    return _mm_sign_epi16(d.v(), _mm_setallone_si128());
#else
    return _mm_mullo_epi16(d.v(), _mm_setallone_si128());
#endif
}
template<> inline Vector<short> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned short>::operator-() const
{
#ifdef VC_IMPL_SSSE3
    return _mm_sign_epi16(d.v(), _mm_setallone_si128());
#else
    return _mm_mullo_epi16(d.v(), _mm_setallone_si128());
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////
// integer ops {{{1
#define OP_IMPL(T, symbol, fun) \
template<> inline Vector<T> &Vector<T>::operator symbol##=(const Vector<T> &x) \
{ \
    d.v() = VectorHelper<T>::fun(d.v(), x.d.v()); \
    return *this; \
} \
template<> inline Vector<T>  Vector<T>::operator symbol(const Vector<T> &x) const \
{ \
    return VectorHelper<T>::fun(d.v(), x.d.v()); \
}
OP_IMPL(int, &, and_)
OP_IMPL(int, |, or_)
OP_IMPL(int, ^, xor_)
OP_IMPL(unsigned int, &, and_)
OP_IMPL(unsigned int, |, or_)
OP_IMPL(unsigned int, ^, xor_)
OP_IMPL(short, &, and_)
OP_IMPL(short, |, or_)
OP_IMPL(short, ^, xor_)
OP_IMPL(unsigned short, &, and_)
OP_IMPL(unsigned short, |, or_)
OP_IMPL(unsigned short, ^, xor_)
OP_IMPL(float, &, and_)
OP_IMPL(float, |, or_)
OP_IMPL(float, ^, xor_)
OP_IMPL(float8, &, and_)
OP_IMPL(float8, |, or_)
OP_IMPL(float8, ^, xor_)
OP_IMPL(double, &, and_)
OP_IMPL(double, |, or_)
OP_IMPL(double, ^, xor_)
#undef OP_IMPL

#ifdef VC_IMPL_XOP
static inline Vc_INTRINSIC Vc_CONST __m128i shiftLeft (const    int_v &value, const    int_v &count) { return _mm_sha_epi32(value.data(), count.data()); }
static inline Vc_INTRINSIC Vc_CONST __m128i shiftLeft (const   uint_v &value, const   uint_v &count) { return _mm_shl_epi32(value.data(), count.data()); }
static inline Vc_INTRINSIC Vc_CONST __m128i shiftLeft (const  short_v &value, const  short_v &count) { return _mm_sha_epi16(value.data(), count.data()); }
static inline Vc_INTRINSIC Vc_CONST __m128i shiftLeft (const ushort_v &value, const ushort_v &count) { return _mm_shl_epi16(value.data(), count.data()); }
static inline Vc_INTRINSIC Vc_CONST __m128i shiftRight(const    int_v &value, const    int_v &count) { return shiftLeft(value,          -count ); }
static inline Vc_INTRINSIC Vc_CONST __m128i shiftRight(const   uint_v &value, const   uint_v &count) { return shiftLeft(value,   uint_v(-count)); }
static inline Vc_INTRINSIC Vc_CONST __m128i shiftRight(const  short_v &value, const  short_v &count) { return shiftLeft(value,          -count ); }
static inline Vc_INTRINSIC Vc_CONST __m128i shiftRight(const ushort_v &value, const ushort_v &count) { return shiftLeft(value, ushort_v(-count)); }

#define _VC_OP(T, symbol, impl) \
template<> inline Vc_INTRINSIC T &T::operator symbol##=(T::AsArg shift) \
{ \
    d.v() = impl(*this, shift); \
    return *this; \
} \
template<> inline Vc_INTRINSIC T  T::operator symbol   (T::AsArg shift) const \
{ \
    return impl(*this, shift); \
}
VC_APPLY_2(VC_LIST_INT_VECTOR_TYPES, _VC_OP, <<, shiftLeft)
VC_APPLY_2(VC_LIST_INT_VECTOR_TYPES, _VC_OP, >>, shiftRight)
#undef _VC_OP
#else
#if defined(VC_GCC) && VC_GCC == 0x40600 && VC_IMPL_XOP
#define VC_WORKAROUND_IN
#define VC_WORKAROUND __attribute__((optimize("no-tree-vectorize"),weak))
#else
#define VC_WORKAROUND_IN inline
#define VC_WORKAROUND Vc_INTRINSIC
#endif

#define OP_IMPL(T, symbol) \
template<> VC_WORKAROUND_IN Vector<T> VC_WORKAROUND &Vector<T>::operator symbol##=(Vector<T>::AsArg x) \
{ \
    for_all_vector_entries(i, \
            d.m(i) symbol##= x.d.m(i); \
            ); \
    return *this; \
} \
template<> inline Vector<T>  Vector<T>::operator symbol(Vector<T>::AsArg x) const \
{ \
    Vector<T> r; \
    for_all_vector_entries(i, \
            r.d.m(i) = d.m(i) symbol x.d.m(i); \
            ); \
    return r; \
}
OP_IMPL(int, <<)
OP_IMPL(int, >>)
OP_IMPL(unsigned int, <<)
OP_IMPL(unsigned int, >>)
OP_IMPL(short, <<)
OP_IMPL(short, >>)
OP_IMPL(unsigned short, <<)
OP_IMPL(unsigned short, >>)
#undef OP_IMPL
#undef VC_WORKAROUND
#undef VC_WORKAROUND_IN
#endif

template<typename T> inline Vector<T> &Vector<T>::operator>>=(int shift) {
    d.v() = VectorHelper<T>::shiftRight(d.v(), shift);
    return *this;
}
template<typename T> inline Vector<T> Vector<T>::operator>>(int shift) const {
    return VectorHelper<T>::shiftRight(d.v(), shift);
}
template<typename T> inline Vector<T> &Vector<T>::operator<<=(int shift) {
    d.v() = VectorHelper<T>::shiftLeft(d.v(), shift);
    return *this;
}
template<typename T> inline Vector<T> Vector<T>::operator<<(int shift) const {
    return VectorHelper<T>::shiftLeft(d.v(), shift);
}

///////////////////////////////////////////////////////////////////////////////////////////
// swizzles {{{1
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST &Vector<T>::abcd() const { return *this; }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::cdab() const { return Mem::permute<X2, X3, X0, X1>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::badc() const { return Mem::permute<X1, X0, X3, X2>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::aaaa() const { return Mem::permute<X0, X0, X0, X0>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::bbbb() const { return Mem::permute<X1, X1, X1, X1>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::cccc() const { return Mem::permute<X2, X2, X2, X2>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::dddd() const { return Mem::permute<X3, X3, X3, X3>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::bcad() const { return Mem::permute<X1, X2, X0, X3>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::bcda() const { return Mem::permute<X1, X2, X3, X0>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::dabc() const { return Mem::permute<X3, X0, X1, X2>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::acbd() const { return Mem::permute<X0, X2, X1, X3>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::dbca() const { return Mem::permute<X3, X1, X2, X0>(data()); }
template<typename T> inline const Vector<T> Vc_INTRINSIC Vc_CONST  Vector<T>::dcba() const { return Mem::permute<X3, X2, X1, X0>(data()); }

template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::cdab() const { return M256::create(Mem::permute<X2, X3, X0, X1>(d.v()[0]), Mem::permute<X2, X3, X0, X1>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::badc() const { return M256::create(Mem::permute<X1, X0, X3, X2>(d.v()[0]), Mem::permute<X1, X0, X3, X2>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::aaaa() const { return M256::create(Mem::permute<X0, X0, X0, X0>(d.v()[0]), Mem::permute<X0, X0, X0, X0>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::bbbb() const { return M256::create(Mem::permute<X1, X1, X1, X1>(d.v()[0]), Mem::permute<X1, X1, X1, X1>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::cccc() const { return M256::create(Mem::permute<X2, X2, X2, X2>(d.v()[0]), Mem::permute<X2, X2, X2, X2>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::dddd() const { return M256::create(Mem::permute<X3, X3, X3, X3>(d.v()[0]), Mem::permute<X3, X3, X3, X3>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::bcad() const { return M256::create(Mem::permute<X1, X2, X0, X3>(d.v()[0]), Mem::permute<X1, X2, X0, X3>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::bcda() const { return M256::create(Mem::permute<X1, X2, X3, X0>(d.v()[0]), Mem::permute<X1, X2, X3, X0>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::dabc() const { return M256::create(Mem::permute<X3, X0, X1, X2>(d.v()[0]), Mem::permute<X3, X0, X1, X2>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::acbd() const { return M256::create(Mem::permute<X0, X2, X1, X3>(d.v()[0]), Mem::permute<X0, X2, X1, X3>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::dbca() const { return M256::create(Mem::permute<X3, X1, X2, X0>(d.v()[0]), Mem::permute<X3, X1, X2, X0>(d.v()[1])); }
template<> inline const sfloat_v Vc_INTRINSIC Vc_CONST Vector<sfloat>::dcba() const { return M256::create(Mem::permute<X3, X2, X1, X0>(d.v()[0]), Mem::permute<X3, X2, X1, X0>(d.v()[1])); }

#define VC_SWIZZLES_16BIT_IMPL(T) \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::cdab() const { return Mem::permute<X2, X3, X0, X1, X6, X7, X4, X5>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::badc() const { return Mem::permute<X1, X0, X3, X2, X5, X4, X7, X6>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::aaaa() const { return Mem::permute<X0, X0, X0, X0, X4, X4, X4, X4>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::bbbb() const { return Mem::permute<X1, X1, X1, X1, X5, X5, X5, X5>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::cccc() const { return Mem::permute<X2, X2, X2, X2, X6, X6, X6, X6>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::dddd() const { return Mem::permute<X3, X3, X3, X3, X7, X7, X7, X7>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::bcad() const { return Mem::permute<X1, X2, X0, X3, X5, X6, X4, X7>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::bcda() const { return Mem::permute<X1, X2, X3, X0, X5, X6, X7, X4>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::dabc() const { return Mem::permute<X3, X0, X1, X2, X7, X4, X5, X6>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::acbd() const { return Mem::permute<X0, X2, X1, X3, X4, X6, X5, X7>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::dbca() const { return Mem::permute<X3, X1, X2, X0, X7, X5, X6, X4>(data()); } \
template<> inline const Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::dcba() const { return Mem::permute<X3, X2, X1, X0, X7, X6, X5, X4>(data()); }
VC_SWIZZLES_16BIT_IMPL(short)
VC_SWIZZLES_16BIT_IMPL(unsigned short)
#undef VC_SWIZZLES_16BIT_IMPL

// operators {{{1
#include "../common/operators.h"
// isNegative {{{1
template<> inline Vc_PURE Vc_INTRINSIC float_m float_v::isNegative() const
{
    return sse_cast<__m128>(_mm_srai_epi32(sse_cast<__m128i>(_mm_and_ps(_mm_setsignmask_ps(), d.v())), 31));
}
template<> inline Vc_PURE Vc_INTRINSIC sfloat_m sfloat_v::isNegative() const
{
    return M256::create(
            sse_cast<__m128>(_mm_srai_epi32(sse_cast<__m128i>(_mm_and_ps(_mm_setsignmask_ps(), d.v()[0])), 31)),
            sse_cast<__m128>(_mm_srai_epi32(sse_cast<__m128i>(_mm_and_ps(_mm_setsignmask_ps(), d.v()[1])), 31))
            );
}
template<> inline Vc_PURE Vc_INTRINSIC double_m double_v::isNegative() const
{
    return Mem::permute<X1, X1, X3, X3>(sse_cast<__m128>(
                _mm_srai_epi32(sse_cast<__m128i>(_mm_and_pd(_mm_setsignmask_pd(), d.v())), 31)
                ));
}
// gathers {{{1
template<typename T> template<typename IndexT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const EntryType *mem, const IndexT *indexes)
{
    gather(mem, indexes);
}
template<typename T> template<typename IndexT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const EntryType *mem, VC_ALIGNED_PARAMETER(Vector<IndexT>) indexes)
{
    gather(mem, indexes);
}

template<typename T> template<typename IndexT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const EntryType *mem, const IndexT *indexes, MaskArg mask)
    : d(HT::zero())
{
    gather(mem, indexes, mask);
}

template<typename T> template<typename IndexT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const EntryType *mem, VC_ALIGNED_PARAMETER(Vector<IndexT>) indexes, MaskArg mask)
    : d(HT::zero())
{
    gather(mem, indexes, mask);
}

template<typename T> template<typename S1, typename IT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    gather(array, member1, indexes);
}
template<typename T> template<typename S1, typename IT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes, MaskArg mask)
    : d(HT::zero())
{
    gather(array, member1, indexes, mask);
}
template<typename T> template<typename S1, typename S2, typename IT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    gather(array, member1, member2, indexes);
}
template<typename T> template<typename S1, typename S2, typename IT> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes, MaskArg mask)
    : d(HT::zero())
{
    gather(array, member1, member2, indexes, mask);
}
template<typename T> template<typename S1, typename IT1, typename IT2> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    gather(array, ptrMember1, outerIndexes, innerIndexes);
}
template<typename T> template<typename S1, typename IT1, typename IT2> inline Vc_ALWAYS_INLINE Vector<T>::Vector(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes, MaskArg mask)
    : d(HT::zero())
{
    gather(array, ptrMember1, outerIndexes, innerIndexes, mask);
}

template<typename T, size_t Size> struct IndexSizeChecker { static void check() {} };
template<typename T, size_t Size> struct IndexSizeChecker<Vector<T>, Size>
{
    static void check() {
        VC_STATIC_ASSERT(Vector<T>::Size >= Size, IndexVector_must_have_greater_or_equal_number_of_entries);
    }
};
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<double>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm_setr_pd(mem[indexes[0]], mem[indexes[1]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm_setr_ps(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float8>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v()[0] = _mm_setr_ps(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]]);
    d.v()[1] = _mm_setr_ps(mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm_setr_epi32(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm_setr_epi32(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<short>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm_setr_epi16(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]],
            mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned short>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm_setr_epi16(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]],
                mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
}

#ifdef VC_USE_SET_GATHERS
template<typename T> template<typename IT> inline void Vc_ALWAYS_INLINE Vector<T>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Vector<IT>) indexes, MaskArg mask)
{
    IndexSizeChecker<Vector<IT>, Size>::check();
    Vector<IT> indexesTmp = indexes;
    indexesTmp.setZero(!static_cast<typename Vector<IT>::Mask>(mask));
    (*this)(mask) = Vector<T>(mem, indexesTmp);
}
#endif

#ifdef VC_USE_BSF_GATHERS
#define VC_MASKED_GATHER                        \
    int bits = mask.toInt();                    \
    while (bits) {                              \
        const int i = _bit_scan_forward(bits);  \
        bits &= ~(1 << i); /* btr? */           \
        d.m(i) = ith_value(i);                  \
    }
#elif defined(VC_USE_POPCNT_BSF_GATHERS)
#define VC_MASKED_GATHER                        \
    unsigned int bits = mask.toInt();           \
    unsigned int low, high = 0;                 \
    switch (mask.count()) {             \
    case 8:                                     \
        high = _bit_scan_reverse(bits);         \
        d.m(high) = ith_value(high);            \
        high = (1 << high);                     \
    case 7:                                     \
        low = _bit_scan_forward(bits);          \
        bits ^= high | (1 << low);              \
        d.m(low) = ith_value(low);              \
    case 6:                                     \
        high = _bit_scan_reverse(bits);         \
        d.m(high) = ith_value(high);            \
        high = (1 << high);                     \
    case 5:                                     \
        low = _bit_scan_forward(bits);          \
        bits ^= high | (1 << low);              \
        d.m(low) = ith_value(low);              \
    case 4:                                     \
        high = _bit_scan_reverse(bits);         \
        d.m(high) = ith_value(high);            \
        high = (1 << high);                     \
    case 3:                                     \
        low = _bit_scan_forward(bits);          \
        bits ^= high | (1 << low);              \
        d.m(low) = ith_value(low);              \
    case 2:                                     \
        high = _bit_scan_reverse(bits);         \
        d.m(high) = ith_value(high);            \
    case 1:                                     \
        low = _bit_scan_forward(bits);          \
        d.m(low) = ith_value(low);              \
    case 0:                                     \
        break;                                  \
    }
#else
#define VC_MASKED_GATHER                        \
    if (mask.isEmpty()) {                       \
        return;                                 \
    }                                           \
    for_all_vector_entries(i,                   \
            if (mask[i]) d.m(i) = ith_value(i); \
            );
#endif

template<typename T> template<typename Index>
inline void Vc_INTRINSIC Vector<T>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes, MaskArg mask)
{
    IndexSizeChecker<Index, Size>::check();
#define ith_value(_i_) (mem[indexes[_i_]])
    VC_MASKED_GATHER
#undef ith_value
}

template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<double>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_pd(array[indexes[0]].*(member1), array[indexes[1]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_ps(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float8>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v()[0] = _mm_setr_ps(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1));
    d.v()[1] = _mm_setr_ps(array[indexes[4]].*(member1), array[indexes[5]].*(member1), array[indexes[6]].*(member1),
            array[indexes[7]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_epi32(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_epi32(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<short>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_epi16(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1), array[indexes[4]].*(member1), array[indexes[5]].*(member1),
            array[indexes[6]].*(member1), array[indexes[7]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned short>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_epi16(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1), array[indexes[4]].*(member1), array[indexes[5]].*(member1),
            array[indexes[6]].*(member1), array[indexes[7]].*(member1));
}
template<typename T> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes, MaskArg mask)
{
    IndexSizeChecker<IT, Size>::check();
#define ith_value(_i_) (array[indexes[_i_]].*(member1))
    VC_MASKED_GATHER
#undef ith_value
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<double>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_pd(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_ps(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2), array[indexes[2]].*(member1).*(member2),
            array[indexes[3]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float8>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v()[0] = _mm_setr_ps(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2),
            array[indexes[2]].*(member1).*(member2), array[indexes[3]].*(member1).*(member2));
    d.v()[1] = _mm_setr_ps(array[indexes[4]].*(member1).*(member2), array[indexes[5]].*(member1).*(member2),
            array[indexes[6]].*(member1).*(member2), array[indexes[7]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_epi32(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2),
            array[indexes[2]].*(member1).*(member2), array[indexes[3]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_epi32(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2),
            array[indexes[2]].*(member1).*(member2), array[indexes[3]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<short>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_epi16(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2), array[indexes[2]].*(member1).*(member2),
            array[indexes[3]].*(member1).*(member2), array[indexes[4]].*(member1).*(member2), array[indexes[5]].*(member1).*(member2),
            array[indexes[6]].*(member1).*(member2), array[indexes[7]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned short>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm_setr_epi16(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2), array[indexes[2]].*(member1).*(member2),
            array[indexes[3]].*(member1).*(member2), array[indexes[4]].*(member1).*(member2), array[indexes[5]].*(member1).*(member2),
            array[indexes[6]].*(member1).*(member2), array[indexes[7]].*(member1).*(member2));
}
template<typename T> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes, MaskArg mask)
{
    IndexSizeChecker<IT, Size>::check();
#define ith_value(_i_) (array[indexes[_i_]].*(member1).*(member2))
    VC_MASKED_GATHER
#undef ith_value
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<double>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm_setr_pd((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]],
            (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm_setr_ps((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float8>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v()[0] = _mm_setr_ps((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]]);
    d.v()[1] = _mm_setr_ps((array[outerIndexes[4]].*(ptrMember1))[innerIndexes[4]], (array[outerIndexes[5]].*(ptrMember1))[innerIndexes[5]],
            (array[outerIndexes[6]].*(ptrMember1))[innerIndexes[6]], (array[outerIndexes[7]].*(ptrMember1))[innerIndexes[7]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm_setr_epi32((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm_setr_epi32((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<short>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm_setr_epi16((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]],
            (array[outerIndexes[4]].*(ptrMember1))[innerIndexes[4]], (array[outerIndexes[5]].*(ptrMember1))[innerIndexes[5]],
            (array[outerIndexes[6]].*(ptrMember1))[innerIndexes[6]], (array[outerIndexes[7]].*(ptrMember1))[innerIndexes[7]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned short>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm_setr_epi16((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]],
            (array[outerIndexes[4]].*(ptrMember1))[innerIndexes[4]], (array[outerIndexes[5]].*(ptrMember1))[innerIndexes[5]],
            (array[outerIndexes[6]].*(ptrMember1))[innerIndexes[6]], (array[outerIndexes[7]].*(ptrMember1))[innerIndexes[7]]);
}
template<typename T> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes, MaskArg mask)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
#define ith_value(_i_) (array[outerIndexes[_i_]].*(ptrMember1))[innerIndexes[_i_]]
    VC_MASKED_GATHER
#undef ith_value
}
// scatters {{{1
#undef VC_MASKED_GATHER
#ifdef VC_USE_BSF_SCATTERS
#define VC_MASKED_SCATTER                       \
    int bits = mask.toInt();                    \
    while (bits) {                              \
        const int i = _bit_scan_forward(bits);  \
        bits ^= (1 << i); /* btr? */            \
        ith_value(i) = d.m(i);                  \
    }
#elif defined(VC_USE_POPCNT_BSF_SCATTERS)
#define VC_MASKED_SCATTER                       \
    unsigned int bits = mask.toInt();           \
    unsigned int low, high = 0;                 \
    switch (mask.count()) {             \
    case 8:                                     \
        high = _bit_scan_reverse(bits);         \
        ith_value(high) = d.m(high);            \
        high = (1 << high);                     \
    case 7:                                     \
        low = _bit_scan_forward(bits);          \
        bits ^= high | (1 << low);              \
        ith_value(low) = d.m(low);              \
    case 6:                                     \
        high = _bit_scan_reverse(bits);         \
        ith_value(high) = d.m(high);            \
        high = (1 << high);                     \
    case 5:                                     \
        low = _bit_scan_forward(bits);          \
        bits ^= high | (1 << low);              \
        ith_value(low) = d.m(low);              \
    case 4:                                     \
        high = _bit_scan_reverse(bits);         \
        ith_value(high) = d.m(high);            \
        high = (1 << high);                     \
    case 3:                                     \
        low = _bit_scan_forward(bits);          \
        bits ^= high | (1 << low);              \
        ith_value(low) = d.m(low);              \
    case 2:                                     \
        high = _bit_scan_reverse(bits);         \
        ith_value(high) = d.m(high);            \
    case 1:                                     \
        low = _bit_scan_forward(bits);          \
        ith_value(low) = d.m(low);              \
    case 0:                                     \
        break;                                  \
    }
#else
#define VC_MASKED_SCATTER                       \
    if (mask.isEmpty()) {                       \
        return;                                 \
    }                                           \
    for_all_vector_entries(i,                   \
            if (mask[i]) ith_value(i) = d.m(i); \
            );
#endif

template<typename T> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::scatter(EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes) const
{
    for_all_vector_entries(i,
            mem[indexes[i]] = d.m(i);
            );
}
template<typename T> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::scatter(EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes, MaskArg mask) const
{
#define ith_value(_i_) mem[indexes[_i_]]
    VC_MASKED_SCATTER
#undef ith_value
}
template<typename T> template<typename S1, typename IT> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::scatter(S1 *array, EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes) const
{
    for_all_vector_entries(i,
            array[indexes[i]].*(member1) = d.m(i);
            );
}
template<typename T> template<typename S1, typename IT> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::scatter(S1 *array, EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes, MaskArg mask) const
{
#define ith_value(_i_) array[indexes[_i_]].*(member1)
    VC_MASKED_SCATTER
#undef ith_value
}
template<typename T> template<typename S1, typename S2, typename IT> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::scatter(S1 *array, S2 S1::* member1, EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes) const
{
    for_all_vector_entries(i,
            array[indexes[i]].*(member1).*(member2) = d.m(i);
            );
}
template<typename T> template<typename S1, typename S2, typename IT> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::scatter(S1 *array, S2 S1::* member1, EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes, MaskArg mask) const
{
#define ith_value(_i_) array[indexes[_i_]].*(member1).*(member2)
    VC_MASKED_SCATTER
#undef ith_value
}
template<typename T> template<typename S1, typename IT1, typename IT2> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::scatter(S1 *array, EntryType *S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes) const
{
    for_all_vector_entries(i,
            (array[innerIndexes[i]].*(ptrMember1))[outerIndexes[i]] = d.m(i);
            );
}
template<typename T> template<typename S1, typename IT1, typename IT2> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::scatter(S1 *array, EntryType *S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes, MaskArg mask) const
{
#define ith_value(_i_) (array[outerIndexes[_i_]].*(ptrMember1))[innerIndexes[_i_]]
    VC_MASKED_SCATTER
#undef ith_value
}

///////////////////////////////////////////////////////////////////////////////////////////
// operator[] {{{1
template<typename T> inline typename Vector<T>::EntryType Vc_PURE Vc_INTRINSIC Vector<T>::operator[](size_t index) const
{
    return d.m(index);
}
#ifdef VC_GCC
template<> inline double Vc_PURE Vc_INTRINSIC Vector<double>::operator[](size_t index) const
{
    if (__builtin_constant_p(index)) {
        return extract_double_imm(d.v(), index);
    }
    return d.m(index);
}
template<> inline float Vc_PURE Vc_INTRINSIC Vector<float>::operator[](size_t index) const
{
    return extract_float(d.v(), index);
}
template<> inline float Vc_PURE Vc_INTRINSIC Vector<float8>::operator[](size_t index) const
{
    if (__builtin_constant_p(index)) {
        if (index < 4) {
            return extract_float_imm(d.v()[0], index);
        }
        return extract_float_imm(d.v()[1], index - 4);
    }
    return d.m(index);
}
template<> inline int Vc_PURE Vc_INTRINSIC Vector<int>::operator[](size_t index) const
{
    if (__builtin_constant_p(index)) {
#if VC_GCC >= 0x40601 || !defined(VC_USE_VEX_CODING) // GCC < 4.6.1 incorrectly uses vmovq instead of movq for the following
#ifdef __x86_64__
        if (index == 0) return _mm_cvtsi128_si64(d.v()) & 0xFFFFFFFFull;
        if (index == 1) return _mm_cvtsi128_si64(d.v()) >> 32;
#else
        if (index == 0) return _mm_cvtsi128_si32(d.v());
#endif
#endif
#ifdef VC_IMPL_SSE4_1
        return _mm_extract_epi32(d.v(), index);
#else
        return _mm_cvtsi128_si32(_mm_srli_si128(d.v(), index * 4));
#endif
    }
    return d.m(index);
}
template<> inline unsigned int Vc_PURE Vc_INTRINSIC Vector<unsigned int>::operator[](size_t index) const
{
    if (__builtin_constant_p(index)) {
#if VC_GCC >= 0x40601 || !defined(VC_USE_VEX_CODING) // GCC < 4.6.1 incorrectly uses vmovq instead of movq for the following
#ifdef __x86_64__
        if (index == 0) return _mm_cvtsi128_si64(d.v()) & 0xFFFFFFFFull;
        if (index == 1) return _mm_cvtsi128_si64(d.v()) >> 32;
#else
        if (index == 0) return _mm_cvtsi128_si32(d.v());
#endif
#endif
#ifdef VC_IMPL_SSE4_1
        return _mm_extract_epi32(d.v(), index);
#else
        return _mm_cvtsi128_si32(_mm_srli_si128(d.v(), index * 4));
#endif
    }
    return d.m(index);
}
template<> inline short Vc_PURE Vc_INTRINSIC Vector<short>::operator[](size_t index) const
{
    if (__builtin_constant_p(index)) {
        return _mm_extract_epi16(d.v(), index);
    }
    return d.m(index);
}
template<> inline unsigned short Vc_PURE Vc_INTRINSIC Vector<unsigned short>::operator[](size_t index) const
{
    if (__builtin_constant_p(index)) {
        return _mm_extract_epi16(d.v(), index);
    }
    return d.m(index);
}
#endif // GCC
///////////////////////////////////////////////////////////////////////////////////////////
// horizontal ops {{{1
#ifndef VC_IMPL_SSE4_1
// without SSE4.1 integer multiplication is slow and we rather multiply the scalars
template<> inline int Vc_INTRINSIC Vector<int>::product() const
{
    return (d.m(0) * d.m(1)) * (d.m(2) * d.m(3));
}
template<> inline unsigned int Vc_INTRINSIC Vector<unsigned int>::product() const
{
    return (d.m(0) * d.m(1)) * (d.m(2) * d.m(3));
}
#endif
template<typename T> inline typename Vector<T>::EntryType Vector<T>::min(MaskArg m) const
{
    Vector<T> tmp = std::numeric_limits<Vector<T> >::max();
    tmp(m) = *this;
    return tmp.min();
}
template<typename T> inline typename Vector<T>::EntryType Vector<T>::max(MaskArg m) const
{
    Vector<T> tmp = std::numeric_limits<Vector<T> >::min();
    tmp(m) = *this;
    return tmp.max();
}
template<typename T> inline typename Vector<T>::EntryType Vector<T>::product(MaskArg m) const
{
    Vector<T> tmp(VectorSpecialInitializerOne::One);
    tmp(m) = *this;
    return tmp.product();
}
template<typename T> inline typename Vector<T>::EntryType Vector<T>::sum(MaskArg m) const
{
    Vector<T> tmp(VectorSpecialInitializerZero::Zero);
    tmp(m) = *this;
    return tmp.sum();
}

///////////////////////////////////////////////////////////////////////////////////////////
// copySign {{{1
template<> inline Vector<float> Vc_INTRINSIC Vector<float>::copySign(Vector<float>::AsArg reference) const
{
    return _mm_or_ps(
            _mm_and_ps(reference.d.v(), _mm_setsignmask_ps()),
            _mm_and_ps(d.v(), _mm_setabsmask_ps())
            );
}
template<> inline Vector<float8> Vc_INTRINSIC Vector<float8>::copySign(Vector<float8>::AsArg reference) const
{
    return M256::create( _mm_or_ps(
                _mm_and_ps(reference.d.v()[0], _mm_setsignmask_ps()),
                _mm_and_ps(d.v()[0], _mm_setabsmask_ps())
                ), _mm_or_ps(
                _mm_and_ps(reference.d.v()[1], _mm_setsignmask_ps()),
                _mm_and_ps(d.v()[1], _mm_setabsmask_ps())
                )
            );
}
template<> inline Vector<double> Vc_INTRINSIC Vector<double>::copySign(Vector<double>::AsArg reference) const
{
    return _mm_or_pd(
            _mm_and_pd(reference.d.v(), _mm_setsignmask_pd()),
            _mm_and_pd(d.v(), _mm_setabsmask_pd())
            );
}//}}}1
// exponent {{{1
template<> inline Vector<float> Vc_INTRINSIC Vector<float>::exponent() const
{
    VC_ASSERT((*this >= 0.f).isFull());
    __m128i tmp = _mm_srli_epi32(_mm_castps_si128(d.v()), 23);
    tmp = _mm_sub_epi32(tmp, _mm_set1_epi32(0x7f));
    return _mm_cvtepi32_ps(tmp);
}
template<> inline Vector<float8> Vc_INTRINSIC Vector<float8>::exponent() const
{
    VC_ASSERT((*this >= 0.f).isFull());
    __m128i tmp0 = _mm_srli_epi32(_mm_castps_si128(d.v()[0]), 23);
    __m128i tmp1 = _mm_srli_epi32(_mm_castps_si128(d.v()[1]), 23);
    tmp0 = _mm_sub_epi32(tmp0, _mm_set1_epi32(0x7f));
    tmp1 = _mm_sub_epi32(tmp1, _mm_set1_epi32(0x7f));
    return M256::create( _mm_cvtepi32_ps(tmp0), _mm_cvtepi32_ps(tmp1));
}
template<> inline Vector<double> Vc_INTRINSIC Vector<double>::exponent() const
{
    VC_ASSERT((*this >= 0.).isFull());
    __m128i tmp = _mm_srli_epi64(_mm_castpd_si128(d.v()), 52);
    tmp = _mm_sub_epi32(tmp, _mm_set1_epi32(0x3ff));
    return _mm_cvtepi32_pd(_mm_shuffle_epi32(tmp, 0x08));
}
// }}}1
// Random {{{1
static inline Vc_ALWAYS_INLINE void _doRandomStep(Vector<unsigned int> &state0,
        Vector<unsigned int> &state1)
{
    state0.load(&Vc::RandomState[0]);
    state1.load(&Vc::RandomState[uint_v::Size]);
    (state1 * 0xdeece66du + 11).store(&Vc::RandomState[uint_v::Size]);
    uint_v(_mm_xor_si128((state0 * 0xdeece66du + 11).data(), _mm_srli_epi32(state1.data(), 16))).store(&Vc::RandomState[0]);
}

template<typename T> inline Vc_ALWAYS_INLINE Vector<T> Vector<T>::Random()
{
    Vector<unsigned int> state0, state1;
    _doRandomStep(state0, state1);
    return state0.reinterpretCast<Vector<T> >();
}

template<> inline Vc_ALWAYS_INLINE Vector<float> Vector<float>::Random()
{
    Vector<unsigned int> state0, state1;
    _doRandomStep(state0, state1);
    return _mm_sub_ps(_mm_or_ps(_mm_castsi128_ps(_mm_srli_epi32(state0.data(), 2)), HT::one()), HT::one());
}

template<> inline Vc_ALWAYS_INLINE Vector<float8> Vector<float8>::Random()
{
    Vector<unsigned int> state0, state1;
    _doRandomStep(state0, state1);
    state1 ^= state0 >> 16;
    return M256::create(
            _mm_sub_ps(_mm_or_ps(_mm_castsi128_ps(_mm_srli_epi32(state0.data(), 2)), VectorHelper<float>::one()), VectorHelper<float>::one()),
            _mm_sub_ps(_mm_or_ps(_mm_castsi128_ps(_mm_srli_epi32(state1.data(), 2)), VectorHelper<float>::one()), VectorHelper<float>::one())
            );
}

template<> inline Vc_ALWAYS_INLINE Vector<double> Vector<double>::Random()
{
    typedef unsigned long long uint64 Vc_MAY_ALIAS;
    uint64 state0 = *reinterpret_cast<const uint64 *>(&Vc::RandomState[8]);
    uint64 state1 = *reinterpret_cast<const uint64 *>(&Vc::RandomState[10]);
    const __m128i state = _mm_load_si128(reinterpret_cast<const __m128i *>(&Vc::RandomState[8]));
    *reinterpret_cast<uint64 *>(&Vc::RandomState[ 8]) = (state0 * 0x5deece66dull + 11);
    *reinterpret_cast<uint64 *>(&Vc::RandomState[10]) = (state1 * 0x5deece66dull + 11);
    return (Vector<double>(_mm_castsi128_pd(_mm_srli_epi64(state, 12))) | One()) - One();
}
// shifted / rotated {{{1
template<typename T> inline Vc_INTRINSIC Vector<T> Vector<T>::shifted(int amount) const
{
    switch (amount) {
    case  0: return *this;
    case  1: return mm128_reinterpret_cast<VectorType>(_mm_srli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 1 * sizeof(EntryType)));
    case  2: return mm128_reinterpret_cast<VectorType>(_mm_srli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 2 * sizeof(EntryType)));
    case  3: return mm128_reinterpret_cast<VectorType>(_mm_srli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 3 * sizeof(EntryType)));
    case  4: return mm128_reinterpret_cast<VectorType>(_mm_srli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 4 * sizeof(EntryType)));
    case  5: return mm128_reinterpret_cast<VectorType>(_mm_srli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 5 * sizeof(EntryType)));
    case  6: return mm128_reinterpret_cast<VectorType>(_mm_srli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 6 * sizeof(EntryType)));
    case  7: return mm128_reinterpret_cast<VectorType>(_mm_srli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 7 * sizeof(EntryType)));
    case  8: return mm128_reinterpret_cast<VectorType>(_mm_srli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 8 * sizeof(EntryType)));
    case -1: return mm128_reinterpret_cast<VectorType>(_mm_slli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 1 * sizeof(EntryType)));
    case -2: return mm128_reinterpret_cast<VectorType>(_mm_slli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 2 * sizeof(EntryType)));
    case -3: return mm128_reinterpret_cast<VectorType>(_mm_slli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 3 * sizeof(EntryType)));
    case -4: return mm128_reinterpret_cast<VectorType>(_mm_slli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 4 * sizeof(EntryType)));
    case -5: return mm128_reinterpret_cast<VectorType>(_mm_slli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 5 * sizeof(EntryType)));
    case -6: return mm128_reinterpret_cast<VectorType>(_mm_slli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 6 * sizeof(EntryType)));
    case -7: return mm128_reinterpret_cast<VectorType>(_mm_slli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 7 * sizeof(EntryType)));
    case -8: return mm128_reinterpret_cast<VectorType>(_mm_slli_si128(mm128_reinterpret_cast<__m128i>(d.v()), 8 * sizeof(EntryType)));
    }
    return Zero();
}
template<> inline Vc_INTRINSIC sfloat_v sfloat_v::shifted(int amount) const
{
    switch (amount) {
    case -7: return M256::create(_mm_setzero_ps(), _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(d.v()[0]), 3 * sizeof(EntryType))));
    case -6: return M256::create(_mm_setzero_ps(), _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(d.v()[0]), 2 * sizeof(EntryType))));
    case -5: return M256::create(_mm_setzero_ps(), _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(d.v()[0]), 1 * sizeof(EntryType))));
    case -4: return M256::create(_mm_setzero_ps(), d.v()[0]);
    case -3: return M256::create(_mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(d.v()[0]), 3 * sizeof(EntryType))), _mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(d.v()[1]), _mm_castps_si128(d.v()[0]), 1 * sizeof(EntryType))));
    case -2: return M256::create(_mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(d.v()[0]), 2 * sizeof(EntryType))), _mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(d.v()[1]), _mm_castps_si128(d.v()[0]), 2 * sizeof(EntryType))));
    case -1: return M256::create(_mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(d.v()[0]), 1 * sizeof(EntryType))), _mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(d.v()[1]), _mm_castps_si128(d.v()[0]), 3 * sizeof(EntryType))));
    case  0: return *this;
    case  1: return M256::create(_mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(d.v()[1]), _mm_castps_si128(d.v()[0]), 1 * sizeof(EntryType))), _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(d.v()[1]), 1 * sizeof(EntryType))));
    case  2: return M256::create(_mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(d.v()[1]), _mm_castps_si128(d.v()[0]), 2 * sizeof(EntryType))), _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(d.v()[1]), 2 * sizeof(EntryType))));
    case  3: return M256::create(_mm_castsi128_ps(_mm_alignr_epi8(_mm_castps_si128(d.v()[1]), _mm_castps_si128(d.v()[0]), 3 * sizeof(EntryType))), _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(d.v()[1]), 3 * sizeof(EntryType))));
    case  4: return M256::create(d.v()[1], _mm_setzero_ps());
    case  5: return M256::create(_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(d.v()[1]), 1 * sizeof(EntryType))), _mm_setzero_ps());
    case  6: return M256::create(_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(d.v()[1]), 2 * sizeof(EntryType))), _mm_setzero_ps());
    case  7: return M256::create(_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(d.v()[1]), 3 * sizeof(EntryType))), _mm_setzero_ps());
    }
    return Zero();
}
template<typename T> inline Vc_INTRINSIC Vector<T> Vector<T>::rotated(int amount) const
{
    const __m128i v = mm128_reinterpret_cast<__m128i>(d.v());
    switch (static_cast<unsigned int>(amount) % Size) {
    case  0: return *this;
    case  1: return mm128_reinterpret_cast<VectorType>(_mm_alignr_epi8(v, v, 1 * sizeof(EntryType)));
    case  2: return mm128_reinterpret_cast<VectorType>(_mm_alignr_epi8(v, v, 2 * sizeof(EntryType)));
    case  3: return mm128_reinterpret_cast<VectorType>(_mm_alignr_epi8(v, v, 3 * sizeof(EntryType)));
             // warning "Immediate parameter to intrinsic call too large" disabled in VcMacros.cmake.
             // ICC fails to see that the modulo operation (Size == sizeof(VectorType) / sizeof(EntryType))
             // disables the following four calls unless sizeof(EntryType) == 2.
    case  4: return mm128_reinterpret_cast<VectorType>(_mm_alignr_epi8(v, v, 4 * sizeof(EntryType)));
    case  5: return mm128_reinterpret_cast<VectorType>(_mm_alignr_epi8(v, v, 5 * sizeof(EntryType)));
    case  6: return mm128_reinterpret_cast<VectorType>(_mm_alignr_epi8(v, v, 6 * sizeof(EntryType)));
    case  7: return mm128_reinterpret_cast<VectorType>(_mm_alignr_epi8(v, v, 7 * sizeof(EntryType)));
    }
    return Zero();
}
template<> inline Vc_INTRINSIC sfloat_v sfloat_v::rotated(int amount) const
{
    const __m128i v0 = sse_cast<__m128i>(d.v()[0]);
    const __m128i v1 = sse_cast<__m128i>(d.v()[1]);
    switch (static_cast<unsigned int>(amount) % Size) {
    case  0: return *this;
    case  1: return M256::create(sse_cast<__m128>(_mm_alignr_epi8(v1, v0, 1 * sizeof(EntryType))), sse_cast<__m128>(_mm_alignr_epi8(v0, v1, 1 * sizeof(EntryType))));
    case  2: return M256::create(sse_cast<__m128>(_mm_alignr_epi8(v1, v0, 2 * sizeof(EntryType))), sse_cast<__m128>(_mm_alignr_epi8(v0, v1, 2 * sizeof(EntryType))));
    case  3: return M256::create(sse_cast<__m128>(_mm_alignr_epi8(v1, v0, 3 * sizeof(EntryType))), sse_cast<__m128>(_mm_alignr_epi8(v0, v1, 3 * sizeof(EntryType))));
    case  4: return M256::create(d.v()[1], d.v()[0]);
    case  5: return M256::create(sse_cast<__m128>(_mm_alignr_epi8(v0, v1, 1 * sizeof(EntryType))), sse_cast<__m128>(_mm_alignr_epi8(v1, v0, 1 * sizeof(EntryType))));
    case  6: return M256::create(sse_cast<__m128>(_mm_alignr_epi8(v0, v1, 2 * sizeof(EntryType))), sse_cast<__m128>(_mm_alignr_epi8(v1, v0, 2 * sizeof(EntryType))));
    case  7: return M256::create(sse_cast<__m128>(_mm_alignr_epi8(v0, v1, 3 * sizeof(EntryType))), sse_cast<__m128>(_mm_alignr_epi8(v1, v0, 3 * sizeof(EntryType))));
    }
    return Zero();
}
// }}}1
} // namespace SSE
} // namespace Vc

#include "undomacros.h"

// vim: foldmethod=marker
