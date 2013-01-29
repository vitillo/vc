/*  This file is part of the Vc library.

    Copyright (C) 2011-2012 Matthias Kretz <kretz@kde.org>

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
#include "const.h"
#include "macros.h"

namespace Vc
{
ALIGN(64) extern unsigned int RandomState[16];

namespace AVX
{

///////////////////////////////////////////////////////////////////////////////////////////
// constants {{{1
template<typename T> inline Vc_ALWAYS_INLINE Vector<T>::Vector(VectorSpecialInitializerZero::ZEnum) : d(HT::zero()) {}
template<typename T> inline Vc_ALWAYS_INLINE Vector<T>::Vector(VectorSpecialInitializerOne::OEnum) : d(HT::one()) {}
template<typename T> inline Vc_ALWAYS_INLINE Vector<T>::Vector(VectorSpecialInitializerIndexesFromZero::IEnum)
    : d(HV::load(IndexesFromZeroData<T>::address(), Aligned)) {}

template<typename T> inline Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::Zero() { return HT::zero(); }
template<typename T> inline Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::One() { return HT::one(); }
template<typename T> inline Vector<T> Vc_INTRINSIC Vc_CONST Vector<T>::IndexesFromZero() { return HV::load(IndexesFromZeroData<T>::address(), Aligned); }

template<typename T> template<typename T2> inline Vc_ALWAYS_INLINE Vector<T>::Vector(VC_ALIGNED_PARAMETER(Vector<T2>) x)
    : d(StaticCastHelper<T2, T>::cast(x.data())) {}

template<typename T> inline Vc_ALWAYS_INLINE Vector<T>::Vector(EntryType x) : d(HT::set(x)) {}
template<> inline Vc_ALWAYS_INLINE Vector<double>::Vector(EntryType x) : d(_mm256_set1_pd(x)) {}


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
    d.v() = HV::load(mem, align);
}

template<typename T> template<typename OtherT> inline void Vc_INTRINSIC Vector<T>::load(const OtherT *mem)
{
    load(mem, Aligned);
}

// LoadHelper {{{2
template<typename DstT, typename SrcT, typename Flags> struct LoadHelper;

// float {{{2
template<typename Flags> struct LoadHelper<float, double, Flags> {
    static m256 load(const double *mem, Flags f)
    {
        return concat(_mm256_cvtpd_ps(VectorHelper<m256d>::load(&mem[0], f)),
                      _mm256_cvtpd_ps(VectorHelper<m256d>::load(&mem[4], f)));
    }
};
template<typename Flags> struct LoadHelper<float, unsigned int, Flags> {
    static m256 load(const unsigned int *mem, Flags f)
    {
        return StaticCastHelper<unsigned int, float>::cast(VectorHelper<m256i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, int, Flags> {
    static m256 load(const int *mem, Flags f)
    {
        return StaticCastHelper<int, float>::cast(VectorHelper<m256i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, unsigned short, Flags> {
    static m256 load(const unsigned short *mem, Flags f)
    {
        return StaticCastHelper<unsigned short, float>::cast(VectorHelper<m128i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, short, Flags> {
    static m256 load(const short *mem, Flags f)
    {
        return StaticCastHelper<short, float>::cast(VectorHelper<m128i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, unsigned char, Flags> {
    static m256 load(const unsigned char *mem, Flags f)
    {
        return StaticCastHelper<unsigned int, float>::cast(LoadHelper<unsigned int, unsigned char, Flags>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<float, signed char, Flags> {
    static m256 load(const signed char *mem, Flags f)
    {
        return StaticCastHelper<int, float>::cast(LoadHelper<int, signed char, Flags>::load(mem, f));
    }
};

template<typename SrcT, typename Flags> struct LoadHelper<sfloat, SrcT, Flags> : public LoadHelper<float, SrcT, Flags> {};

// int {{{2
template<typename Flags> struct LoadHelper<int, unsigned int, Flags> {
    static m256i load(const unsigned int *mem, Flags f)
    {
        return VectorHelper<m256i>::load(mem, f);
    }
};
template<typename Flags> struct LoadHelper<int, unsigned short, Flags> {
    static m256i load(const unsigned short *mem, Flags f)
    {
        return StaticCastHelper<unsigned short, unsigned int>::cast(VectorHelper<m128i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<int, short, Flags> {
    static m256i load(const short *mem, Flags f)
    {
        return StaticCastHelper<short, int>::cast(VectorHelper<m128i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<int, unsigned char, Flags> {
    static m256i load(const unsigned char *mem, Flags)
    {
        // the only available streaming load loads 16 bytes - twice as much as we need => can't use
        // it, or we risk an out-of-bounds read and an unaligned load exception
        const m128i epu8 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem));
        const m128i epu16 = _mm_cvtepu8_epi16(epu8);
        return StaticCastHelper<unsigned short, unsigned int>::cast(epu16);
    }
};
template<typename Flags> struct LoadHelper<int, signed char, Flags> {
    static m256i load(const signed char *mem, Flags)
    {
        // the only available streaming load loads 16 bytes - twice as much as we need => can't use
        // it, or we risk an out-of-bounds read and an unaligned load exception
        const m128i epi8 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem));
        const m128i epi16 = _mm_cvtepi8_epi16(epi8);
        return StaticCastHelper<short, int>::cast(epi16);
    }
};

// unsigned int {{{2
template<typename Flags> struct LoadHelper<unsigned int, unsigned short, Flags> {
    static m256i load(const unsigned short *mem, Flags f)
    {
        return StaticCastHelper<unsigned short, unsigned int>::cast(VectorHelper<m128i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<unsigned int, unsigned char, Flags> {
    static m256i load(const unsigned char *mem, Flags)
    {
        // the only available streaming load loads 16 bytes - twice as much as we need => can't use
        // it, or we risk an out-of-bounds read and an unaligned load exception
        const m128i epu8 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem));
        const m128i epu16 = _mm_cvtepu8_epi16(epu8);
        return StaticCastHelper<unsigned short, unsigned int>::cast(epu16);
    }
};

// short {{{2
template<typename Flags> struct LoadHelper<short, unsigned short, Flags> {
    static m128i load(const unsigned short *mem, Flags f)
    {
        return StaticCastHelper<unsigned short, short>::cast(VectorHelper<m128i>::load(mem, f));
    }
};
template<typename Flags> struct LoadHelper<short, unsigned char, Flags> {
    static m128i load(const unsigned char *mem, Flags)
    {
        // the only available streaming load loads 16 bytes - twice as much as we need => can't use
        // it, or we risk an out-of-bounds read and an unaligned load exception
        const m128i epu8 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem));
        return _mm_cvtepu8_epi16(epu8);
    }
};
template<typename Flags> struct LoadHelper<short, signed char, Flags> {
    static m128i load(const signed char *mem, Flags)
    {
        // the only available streaming load loads 16 bytes - twice as much as we need => can't use
        // it, or we risk an out-of-bounds read and an unaligned load exception
        const m128i epi8 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem));
        return _mm_cvtepi8_epi16(epi8);
    }
};

// unsigned short {{{2
template<typename Flags> struct LoadHelper<unsigned short, unsigned char, Flags> {
    static m128i load(const unsigned char *mem, Flags)
    {
        // the only available streaming load loads 16 bytes - twice as much as we need => can't use
        // it, or we risk an out-of-bounds read and an unaligned load exception
        const m128i epu8 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(mem));
        return _mm_cvtepu8_epi16(epu8);
    }
};

// general load, implemented via LoadHelper {{{2
template<typename DstT> template<typename SrcT, typename Flags> inline void Vc_INTRINSIC Vector<DstT>::load(const SrcT *x, Flags f)
{
    d.v() = LoadHelper<DstT, SrcT, Flags>::load(x, f);
}

///////////////////////////////////////////////////////////////////////////////////////////
// zeroing {{{1
template<typename T> inline void Vc_INTRINSIC Vector<T>::setZero()
{
    data() = HV::zero();
}
template<typename T> inline void Vc_INTRINSIC Vector<T>::setZero(const Mask &k)
{
    data() = HV::andnot_(avx_cast<VectorType>(k.data()), data());
}

template<> inline void Vc_INTRINSIC Vector<double>::setQnan()
{
    data() = _mm256_setallone_pd();
}
template<> inline void Vc_INTRINSIC Vector<double>::setQnan(MaskArg k)
{
    data() = _mm256_or_pd(data(), k.dataD());
}
template<> inline void Vc_INTRINSIC Vector<float>::setQnan()
{
    data() = _mm256_setallone_ps();
}
template<> inline void Vc_INTRINSIC Vector<float>::setQnan(MaskArg k)
{
    data() = _mm256_or_ps(data(), k.data());
}
template<> inline void Vc_INTRINSIC Vector<sfloat>::setQnan()
{
    data() = _mm256_setallone_ps();
}
template<> inline void Vc_INTRINSIC Vector<sfloat>::setQnan(MaskArg k)
{
    data() = _mm256_or_ps(data(), k.data());
}

///////////////////////////////////////////////////////////////////////////////////////////
// stores {{{1
template<typename T> inline void Vc_INTRINSIC Vector<T>::store(EntryType *mem) const
{
    HV::store(mem, data(), Aligned);
}
template<typename T> inline void Vc_INTRINSIC Vector<T>::store(EntryType *mem, const Mask &mask) const
{
    HV::store(mem, data(), avx_cast<VectorType>(mask.data()), Aligned);
}
template<typename T> template<typename A> inline void Vc_INTRINSIC Vector<T>::store(EntryType *mem, A align) const
{
    HV::store(mem, data(), align);
}
template<typename T> template<typename A> inline void Vc_INTRINSIC Vector<T>::store(EntryType *mem, const Mask &mask, A align) const
{
    HV::store(mem, data(), avx_cast<VectorType>(mask.data()), align);
}

///////////////////////////////////////////////////////////////////////////////////////////
// expand/merge 1 float_v <=> 2 double_v          XXX rationale? remove it for release? XXX {{{1
template<typename T> inline Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::Vector(const Vector<typename HT::ConcatType> *a)
    : d(a[0])
{
}
template<> inline Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::Vector(const Vector<HT::ConcatType> *a)
    : d(concat(_mm256_cvtpd_ps(a[0].data()), _mm256_cvtpd_ps(a[1].data())))
{
}
template<> inline Vc_ALWAYS_INLINE Vc_FLATTEN Vector<short>::Vector(const Vector<HT::ConcatType> *a)
    : d(_mm_packs_epi32(lo128(a->data()), hi128(a->data())))
{
}
template<> inline Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned short>::Vector(const Vector<HT::ConcatType> *a)
    : d(_mm_packus_epi32(lo128(a->data()), hi128(a->data())))
{
}
template<typename T> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<T>::expand(Vector<typename HT::ConcatType> *x) const
{
    x[0] = *this;
}
template<> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::expand(Vector<HT::ConcatType> *x) const
{
    x[0].data() = _mm256_cvtps_pd(lo128(d.v()));
    x[1].data() = _mm256_cvtps_pd(hi128(d.v()));
}
template<> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<short>::expand(Vector<HT::ConcatType> *x) const
{
    x[0].data() = concat(_mm_cvtepi16_epi32(d.v()),
            _mm_cvtepi16_epi32(_mm_unpackhi_epi64(d.v(), d.v())));
}
template<> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned short>::expand(Vector<HT::ConcatType> *x) const
{
    x[0].data() = concat(_mm_cvtepu16_epi32(d.v()),
            _mm_cvtepu16_epi32(_mm_unpackhi_epi64(d.v(), d.v())));
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

template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::cdab() const { return Mem::shuffle128<X1, X0>(data(), data()); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::badc() const { return Mem::permute<X1, X0, X3, X2>(data()); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::aaaa() const { const double &tmp = d.m(0); return _mm256_broadcast_sd(&tmp); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::bbbb() const { const double &tmp = d.m(1); return _mm256_broadcast_sd(&tmp); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::cccc() const { const double &tmp = d.m(2); return _mm256_broadcast_sd(&tmp); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::dddd() const { const double &tmp = d.m(3); return _mm256_broadcast_sd(&tmp); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::bcad() const { return Mem::shuffle<X1, Y0, X2, Y3>(Mem::shuffle128<X0, X0>(data(), data()), Mem::shuffle128<X1, X1>(data(), data())); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::bcda() const { return Mem::shuffle<X1, Y0, X3, Y2>(data(), Mem::shuffle128<X1, X0>(data(), data())); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::dabc() const { return Mem::shuffle<X1, Y0, X3, Y2>(Mem::shuffle128<X1, X0>(data(), data()), data()); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::acbd() const { return Mem::shuffle<X0, Y0, X3, Y3>(Mem::shuffle128<X0, X0>(data(), data()), Mem::shuffle128<X1, X1>(data(), data())); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::dbca() const { return Mem::shuffle<X1, Y1, X2, Y2>(Mem::shuffle128<X1, X1>(data(), data()), Mem::shuffle128<X0, X0>(data(), data())); }
template<> inline const double_v Vc_INTRINSIC Vc_CONST Vector<double>::dcba() const { return cdab().badc(); }

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

///////////////////////////////////////////////////////////////////////////////////////////
// division {{{1
template<typename T> inline Vector<T> &Vector<T>::operator/=(EntryType x)
{
    if (HasVectorDivision) {
        return operator/=(Vector<T>(x));
    }
    for_all_vector_entries(i,
            d.m(i) /= x;
            );
    return *this;
}
template<typename T> template<typename TT> inline Vc_PURE VC_EXACT_TYPE(TT, typename DetermineEntryType<T>::Type, Vector<T>) Vector<T>::operator/(TT x) const
{
    if (HasVectorDivision) {
        return operator/(Vector<T>(x));
    }
    Vector<T> r;
    for_all_vector_entries(i,
            r.d.m(i) = d.m(i) / x;
            );
    return r;
}
// per default fall back to scalar division
template<typename T> inline Vector<T> &Vector<T>::operator/=(const Vector<T> &x)
{
    for_all_vector_entries(i,
            d.m(i) /= x.d.m(i);
            );
    return *this;
}

template<typename T> inline Vector<T> Vc_PURE Vector<T>::operator/(const Vector<T> &x) const
{
    Vector<T> r;
    for_all_vector_entries(i,
            r.d.m(i) = d.m(i) / x.d.m(i);
            );
    return r;
}
// specialize division on type
static inline m256i Vc_INTRINSIC Vc_CONST divInt(param256i a, param256i b) {
    const m256d lo1 = _mm256_cvtepi32_pd(lo128(a));
    const m256d lo2 = _mm256_cvtepi32_pd(lo128(b));
    const m256d hi1 = _mm256_cvtepi32_pd(hi128(a));
    const m256d hi2 = _mm256_cvtepi32_pd(hi128(b));
    return concat(
            _mm256_cvttpd_epi32(_mm256_div_pd(lo1, lo2)),
            _mm256_cvttpd_epi32(_mm256_div_pd(hi1, hi2))
            );
}
template<> inline Vector<int> &Vector<int>::operator/=(const Vector<int> &x)
{
    d.v() = divInt(d.v(), x.d.v());
    return *this;
}
template<> inline Vector<int> Vc_PURE Vector<int>::operator/(const Vector<int> &x) const
{
    return divInt(d.v(), x.d.v());
}
static inline m256i Vc_CONST divUInt(param256i a, param256i b) {
    m256d loa = _mm256_cvtepi32_pd(lo128(a));
    m256d hia = _mm256_cvtepi32_pd(hi128(a));
    m256d lob = _mm256_cvtepi32_pd(lo128(b));
    m256d hib = _mm256_cvtepi32_pd(hi128(b));
    // if a >= 2^31 then after conversion to double it will contain a negative number (i.e. a-2^32)
    // to get the right number back we have to add 2^32 where a >= 2^31
    loa = _mm256_add_pd(loa, _mm256_and_pd(_mm256_cmp_pd(loa, _mm256_setzero_pd(), _CMP_LT_OS), _mm256_set1_pd(4294967296.)));
    hia = _mm256_add_pd(hia, _mm256_and_pd(_mm256_cmp_pd(hia, _mm256_setzero_pd(), _CMP_LT_OS), _mm256_set1_pd(4294967296.)));
    // we don't do the same for b because division by b >= 2^31 should be a seldom corner case and
    // we rather want the standard stuff fast
    //
    // there is one remaining problem: a >= 2^31 and b == 1
    // in that case the return value would be 2^31
    return avx_cast<m256i>(_mm256_blendv_ps(avx_cast<m256>(concat(
                        _mm256_cvttpd_epi32(_mm256_div_pd(loa, lob)),
                        _mm256_cvttpd_epi32(_mm256_div_pd(hia, hib))
                        )), avx_cast<m256>(a), avx_cast<m256>(concat(
                            _mm_cmpeq_epi32(lo128(b), _mm_setone_epi32()),
                            _mm_cmpeq_epi32(hi128(b), _mm_setone_epi32())))));
}
template<> inline Vector<unsigned int> Vc_ALWAYS_INLINE &Vector<unsigned int>::operator/=(const Vector<unsigned int> &x)
{
    d.v() = divUInt(d.v(), x.d.v());
    return *this;
}
template<> inline Vector<unsigned int> Vc_ALWAYS_INLINE Vc_PURE Vector<unsigned int>::operator/(const Vector<unsigned int> &x) const
{
    return divUInt(d.v(), x.d.v());
}
template<typename T> static inline m128i Vc_CONST divShort(param128i a, param128i b)
{
    const m256 r = _mm256_div_ps(StaticCastHelper<T, float>::cast(a),
            StaticCastHelper<T, float>::cast(b));
    return StaticCastHelper<float, T>::cast(r);
}
template<> inline Vector<short> Vc_ALWAYS_INLINE &Vector<short>::operator/=(const Vector<short> &x)
{
    d.v() = divShort<short>(d.v(), x.d.v());
    return *this;
}
template<> inline Vector<short> Vc_ALWAYS_INLINE Vc_PURE Vector<short>::operator/(const Vector<short> &x) const
{
    return divShort<short>(d.v(), x.d.v());
}
template<> inline Vector<unsigned short> Vc_ALWAYS_INLINE &Vector<unsigned short>::operator/=(const Vector<unsigned short> &x)
{
    d.v() = divShort<unsigned short>(d.v(), x.d.v());
    return *this;
}
template<> inline Vector<unsigned short> Vc_ALWAYS_INLINE Vc_PURE Vector<unsigned short>::operator/(const Vector<unsigned short> &x) const
{
    return divShort<unsigned short>(d.v(), x.d.v());
}
template<> inline Vector<float> Vc_INTRINSIC &Vector<float>::operator/=(const Vector<float> &x)
{
    d.v() = _mm256_div_ps(d.v(), x.d.v());
    return *this;
}
template<> inline Vector<float> Vc_INTRINSIC Vc_PURE Vector<float>::operator/(const Vector<float> &x) const
{
    return _mm256_div_ps(d.v(), x.d.v());
}
template<> inline Vector<double> Vc_INTRINSIC &Vector<double>::operator/=(const Vector<double> &x)
{
    d.v() = _mm256_div_pd(d.v(), x.d.v());
    return *this;
}
template<> inline Vector<double> Vc_INTRINSIC Vc_PURE Vector<double>::operator/(const Vector<double> &x) const
{
    return _mm256_div_pd(d.v(), x.d.v());
}

///////////////////////////////////////////////////////////////////////////////////////////
// integer ops {{{1
#define OP_IMPL(T, symbol) \
template<> inline Vector<T> &Vector<T>::operator symbol##=(AsArg x) \
{ \
    for_all_vector_entries(i, d.m(i) symbol##= x.d.m(i); ); \
    return *this; \
} \
template<> inline Vector<T>  Vector<T>::operator symbol(AsArg x) const \
{ \
    Vector<T> r; \
    for_all_vector_entries(i, r.d.m(i) = d.m(i) symbol x.d.m(i); ); \
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

template<typename T> inline Vector<T> &Vector<T>::operator>>=(int shift) {
    d.v() = VectorHelper<T>::shiftRight(d.v(), shift);
    return *static_cast<Vector<T> *>(this);
}
template<typename T> inline Vector<T> Vector<T>::operator>>(int shift) const {
    return VectorHelper<T>::shiftRight(d.v(), shift);
}
template<typename T> inline Vector<T> &Vector<T>::operator<<=(int shift) {
    d.v() = VectorHelper<T>::shiftLeft(d.v(), shift);
    return *static_cast<Vector<T> *>(this);
}
template<typename T> inline Vector<T> Vector<T>::operator<<(int shift) const {
    return VectorHelper<T>::shiftLeft(d.v(), shift);
}

#define OP_IMPL(T, symbol, fun) \
  template<> inline Vector<T> &Vector<T>::operator symbol##=(AsArg x) { d.v() = HV::fun(d.v(), x.d.v()); return *this; } \
  template<> inline Vector<T>  Vector<T>::operator symbol(AsArg x) const { return Vector<T>(HV::fun(d.v(), x.d.v())); }
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
  OP_IMPL(sfloat, &, and_)
  OP_IMPL(sfloat, |, or_)
  OP_IMPL(sfloat, ^, xor_)
  OP_IMPL(double, &, and_)
  OP_IMPL(double, |, or_)
  OP_IMPL(double, ^, xor_)
#undef OP_IMPL

// operators {{{1
#include "../common/operators.h"
// isNegative {{{1
template<> inline Vc_PURE Vc_INTRINSIC float_m float_v::isNegative() const
{
    return avx_cast<m256>(_mm256_srai_epi32(avx_cast<m256i>(_mm256_and_ps(_mm256_setsignmask_ps(), d.v())), 31));
}
template<> inline Vc_PURE Vc_INTRINSIC sfloat_m sfloat_v::isNegative() const
{
    return avx_cast<m256>(_mm256_srai_epi32(avx_cast<m256i>(_mm256_and_ps(_mm256_setsignmask_ps(), d.v())), 31));
}
template<> inline Vc_PURE Vc_INTRINSIC double_m double_v::isNegative() const
{
    return Mem::permute<X1, X1, X3, X3>(avx_cast<m256>(
                _mm256_srai_epi32(avx_cast<m256i>(_mm256_and_pd(_mm256_setsignmask_pd(), d.v())), 31)
                ));
}
// gathers {{{1
// Better implementation (hopefully) with _mm256_set_
//X template<typename T> template<typename Index> Vector<T>::Vector(const EntryType *mem, const Index *indexes)
//X {
//X     for_all_vector_entries(int i,
//X             d.m(i) = mem[indexes[i]];
//X             );
//X }
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
    d.v() = _mm256_setr_pd(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm256_setr_ps(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]],
            mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<sfloat>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm256_setr_ps(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]],
            mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm256_setr_epi32(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]],
            mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
}
template<> template<typename Index> inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::gather(const EntryType *mem, VC_ALIGNED_PARAMETER(Index) indexes)
{
    IndexSizeChecker<Index, Size>::check();
    d.v() = _mm256_setr_epi32(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]],
            mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
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
    indexesTmp.setZero(!mask);
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
    switch (_mm_popcnt_u32(bits)) {             \
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
    d.v() = _mm256_setr_pd(array[indexes[0]].*(member1), array[indexes[1]].*(member1),
            array[indexes[2]].*(member1), array[indexes[3]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm256_setr_ps(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1), array[indexes[4]].*(member1), array[indexes[5]].*(member1),
            array[indexes[6]].*(member1), array[indexes[7]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<sfloat>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm256_setr_ps(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1), array[indexes[4]].*(member1), array[indexes[5]].*(member1),
            array[indexes[6]].*(member1), array[indexes[7]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm256_setr_epi32(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1), array[indexes[4]].*(member1), array[indexes[5]].*(member1),
            array[indexes[6]].*(member1), array[indexes[7]].*(member1));
}
template<> template<typename S1, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::gather(const S1 *array, const EntryType S1::* member1, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm256_setr_epi32(array[indexes[0]].*(member1), array[indexes[1]].*(member1), array[indexes[2]].*(member1),
            array[indexes[3]].*(member1), array[indexes[4]].*(member1), array[indexes[5]].*(member1),
            array[indexes[6]].*(member1), array[indexes[7]].*(member1));
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
    d.v() = _mm256_setr_pd(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2),
            array[indexes[2]].*(member1).*(member2), array[indexes[3]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm256_setr_ps(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2), array[indexes[2]].*(member1).*(member2),
            array[indexes[3]].*(member1).*(member2), array[indexes[4]].*(member1).*(member2), array[indexes[5]].*(member1).*(member2),
            array[indexes[6]].*(member1).*(member2), array[indexes[7]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<sfloat>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm256_setr_ps(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2), array[indexes[2]].*(member1).*(member2),
            array[indexes[3]].*(member1).*(member2), array[indexes[4]].*(member1).*(member2), array[indexes[5]].*(member1).*(member2),
            array[indexes[6]].*(member1).*(member2), array[indexes[7]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm256_setr_epi32(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2), array[indexes[2]].*(member1).*(member2),
            array[indexes[3]].*(member1).*(member2), array[indexes[4]].*(member1).*(member2), array[indexes[5]].*(member1).*(member2),
            array[indexes[6]].*(member1).*(member2), array[indexes[7]].*(member1).*(member2));
}
template<> template<typename S1, typename S2, typename IT>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::gather(const S1 *array, const S2 S1::* member1, const EntryType S2::* member2, VC_ALIGNED_PARAMETER(IT) indexes)
{
    IndexSizeChecker<IT, Size>::check();
    d.v() = _mm256_setr_epi32(array[indexes[0]].*(member1).*(member2), array[indexes[1]].*(member1).*(member2), array[indexes[2]].*(member1).*(member2),
            array[indexes[3]].*(member1).*(member2), array[indexes[4]].*(member1).*(member2), array[indexes[5]].*(member1).*(member2),
            array[indexes[6]].*(member1).*(member2), array[indexes[7]].*(member1).*(member2));
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
    d.v() = _mm256_setr_pd((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm256_setr_ps((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]],
            (array[outerIndexes[4]].*(ptrMember1))[innerIndexes[4]], (array[outerIndexes[5]].*(ptrMember1))[innerIndexes[5]],
            (array[outerIndexes[6]].*(ptrMember1))[innerIndexes[6]], (array[outerIndexes[7]].*(ptrMember1))[innerIndexes[7]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<sfloat>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm256_setr_ps((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]],
            (array[outerIndexes[4]].*(ptrMember1))[innerIndexes[4]], (array[outerIndexes[5]].*(ptrMember1))[innerIndexes[5]],
            (array[outerIndexes[6]].*(ptrMember1))[innerIndexes[6]], (array[outerIndexes[7]].*(ptrMember1))[innerIndexes[7]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm256_setr_epi32((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]],
            (array[outerIndexes[4]].*(ptrMember1))[innerIndexes[4]], (array[outerIndexes[5]].*(ptrMember1))[innerIndexes[5]],
            (array[outerIndexes[6]].*(ptrMember1))[innerIndexes[6]], (array[outerIndexes[7]].*(ptrMember1))[innerIndexes[7]]);
}
template<> template<typename S1, typename IT1, typename IT2>
inline void Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::gather(const S1 *array, const EntryType *const S1::* ptrMember1, VC_ALIGNED_PARAMETER(IT1) outerIndexes, VC_ALIGNED_PARAMETER(IT2) innerIndexes)
{
    IndexSizeChecker<IT1, Size>::check();
    IndexSizeChecker<IT2, Size>::check();
    d.v() = _mm256_setr_epi32((array[outerIndexes[0]].*(ptrMember1))[innerIndexes[0]], (array[outerIndexes[1]].*(ptrMember1))[innerIndexes[1]],
            (array[outerIndexes[2]].*(ptrMember1))[innerIndexes[2]], (array[outerIndexes[3]].*(ptrMember1))[innerIndexes[3]],
            (array[outerIndexes[4]].*(ptrMember1))[innerIndexes[4]], (array[outerIndexes[5]].*(ptrMember1))[innerIndexes[5]],
            (array[outerIndexes[6]].*(ptrMember1))[innerIndexes[6]], (array[outerIndexes[7]].*(ptrMember1))[innerIndexes[7]]);
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
    switch (_mm_popcnt_u32(bits)) {             \
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
// operator- {{{1
template<> inline Vector<double> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<double>::operator-() const
{
    return _mm256_xor_pd(d.v(), _mm256_setsignmask_pd());
}
template<> inline Vector<float> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<float>::operator-() const
{
    return _mm256_xor_ps(d.v(), _mm256_setsignmask_ps());
}
template<> inline Vector<sfloat> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<sfloat>::operator-() const
{
    return _mm256_xor_ps(d.v(), _mm256_setsignmask_ps());
}
template<> inline Vector<int> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<int>::operator-() const
{
    return _mm256_sign_epi32(d.v(), _mm256_setallone_si256());
}
template<> inline Vector<int> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned int>::operator-() const
{
    return _mm256_sign_epi32(d.v(), _mm256_setallone_si256());
}
template<> inline Vector<short> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<short>::operator-() const
{
    return _mm_sign_epi16(d.v(), _mm_setallone_si128());
}
template<> inline Vector<short> Vc_PURE Vc_ALWAYS_INLINE Vc_FLATTEN Vector<unsigned short>::operator-() const
{
    return _mm_sign_epi16(d.v(), _mm_setallone_si128());
}

///////////////////////////////////////////////////////////////////////////////////////////
// horizontal ops {{{1
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
}//}}}
// copySign {{{1
template<> inline Vector<float> Vc_INTRINSIC Vector<float>::copySign(Vector<float>::AsArg reference) const
{
    return _mm256_or_ps(
            _mm256_and_ps(reference.d.v(), _mm256_setsignmask_ps()),
            _mm256_and_ps(d.v(), _mm256_setabsmask_ps())
            );
}
template<> inline Vector<sfloat> Vc_INTRINSIC Vector<sfloat>::copySign(Vector<sfloat>::AsArg reference) const
{
    return _mm256_or_ps(
            _mm256_and_ps(reference.d.v(), _mm256_setsignmask_ps()),
            _mm256_and_ps(d.v(), _mm256_setabsmask_ps())
            );
}
template<> inline Vector<double> Vc_INTRINSIC Vector<double>::copySign(Vector<double>::AsArg reference) const
{
    return _mm256_or_pd(
            _mm256_and_pd(reference.d.v(), _mm256_setsignmask_pd()),
            _mm256_and_pd(d.v(), _mm256_setabsmask_pd())
            );
}//}}}1
// exponent {{{1
template<> inline Vector<float> Vc_INTRINSIC Vector<float>::exponent() const
{
    VC_ASSERT((*this >= 0.f).isFull());
    m128i tmp0 = _mm_srli_epi32(avx_cast<m128i>(d.v()), 23);
    m128i tmp1 = _mm_srli_epi32(avx_cast<m128i>(hi128(d.v())), 23);
    tmp0 = _mm_sub_epi32(tmp0, _mm_set1_epi32(0x7f));
    tmp1 = _mm_sub_epi32(tmp1, _mm_set1_epi32(0x7f));
    return _mm256_cvtepi32_ps(concat(tmp0, tmp1));
}
template<> inline Vector<sfloat> Vc_INTRINSIC Vector<sfloat>::exponent() const
{
    VC_ASSERT((*this >= 0.f).isFull());
    m128i tmp0 = _mm_srli_epi32(avx_cast<m128i>(d.v()), 23);
    m128i tmp1 = _mm_srli_epi32(avx_cast<m128i>(hi128(d.v())), 23);
    tmp0 = _mm_sub_epi32(tmp0, _mm_set1_epi32(0x7f));
    tmp1 = _mm_sub_epi32(tmp1, _mm_set1_epi32(0x7f));
    return _mm256_cvtepi32_ps(concat(tmp0, tmp1));
}
template<> inline Vector<double> Vc_INTRINSIC Vector<double>::exponent() const
{
    VC_ASSERT((*this >= 0.).isFull());
    m128i tmp0 = _mm_srli_epi64(avx_cast<m128i>(d.v()), 52);
    m128i tmp1 = _mm_srli_epi64(avx_cast<m128i>(hi128(d.v())), 52);
    tmp0 = _mm_sub_epi32(tmp0, _mm_set1_epi32(0x3ff));
    tmp1 = _mm_sub_epi32(tmp1, _mm_set1_epi32(0x3ff));
    return _mm256_cvtepi32_pd(avx_cast<m128i>(Mem::shuffle<X0, X2, Y0, Y2>(avx_cast<m128>(tmp0), avx_cast<m128>(tmp1))));
}
// }}}1
// Random {{{1
static inline Vc_ALWAYS_INLINE void _doRandomStep(Vector<unsigned int> &state0,
        Vector<unsigned int> &state1)
{
    state0.load(&Vc::RandomState[0]);
    state1.load(&Vc::RandomState[uint_v::Size]);
    (state1 * 0xdeece66du + 11).store(&Vc::RandomState[uint_v::Size]);
    uint_v(_mm256_xor_si256((state0 * 0xdeece66du + 11).data(), _mm256_srli_epi32(state1.data(), 16))).store(&Vc::RandomState[0]);
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
    return HT::sub(HV::or_(_cast(_mm256_srli_epi32(state0.data(), 2)), HT::one()), HT::one());
}

template<> inline Vc_ALWAYS_INLINE Vector<sfloat> Vector<sfloat>::Random()
{
    Vector<unsigned int> state0, state1;
    _doRandomStep(state0, state1);
    return HT::sub(HV::or_(_cast(_mm256_srli_epi32(state0.data(), 2)), HT::one()), HT::one());
}

template<> inline Vc_ALWAYS_INLINE Vector<double> Vector<double>::Random()
{
    const m256i state = VectorHelper<m256i>::load(&Vc::RandomState[0], Vc::Aligned);
    for (size_t k = 0; k < 8; k += 2) {
        typedef unsigned long long uint64 Vc_MAY_ALIAS;
        const uint64 stateX = *reinterpret_cast<const uint64 *>(&Vc::RandomState[k]);
        *reinterpret_cast<uint64 *>(&Vc::RandomState[k]) = (stateX * 0x5deece66dull + 11);
    }
    return (Vector<double>(_cast(_mm256_srli_epi64(state, 12))) | One()) - One();
}
// }}}1
// shifted / rotated {{{1
template<size_t SIMDWidth, size_t Size, typename VectorType, typename EntryType> struct VectorShift;
template<> struct VectorShift<32, 4, m256d, double>
{
    static inline Vc_INTRINSIC m256d shifted(param256d v, int amount)
    {
        const m128i vLo = avx_cast<m128i>(lo128(v));
        const m128i vHi = avx_cast<m128i>(hi128(v));
        switch (amount) {
        case  0: return v;
        case  1: return concat(avx_cast<m128d>(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(double))), avx_cast<m128d>(_mm_srli_si128(vHi, 1 * sizeof(double))));
        case  2: return avx_cast<m256d>(hi128(v));
        case  3: return avx_cast<m256d>(_mm_srli_si128(vHi, 1 * sizeof(double)));
        case -1: return concat(avx_cast<m128d>(_mm_slli_si128(vLo, 1 * sizeof(double))), avx_cast<m128d>(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(double))));
        case -2: return _mm256_permute2f128_pd(v, v, 0x8);
        case -3: return concat(_mm_setzero_pd(), avx_cast<m128d>(_mm_slli_si128(vLo, 1 * sizeof(double))));
        }
        return _mm256_setzero_pd();
    }
};
template<typename VectorType, typename EntryType> struct VectorShift<32, 8, VectorType, EntryType>
{
    typedef typename SseVectorType<VectorType>::Type SmallV;
    static inline Vc_INTRINSIC VectorType shifted(VC_ALIGNED_PARAMETER(VectorType) v, int amount)
    {
        const m128i vLo = avx_cast<m128i>(lo128(v));
        const m128i vHi = avx_cast<m128i>(hi128(v));
        const SmallV z = avx_cast<SmallV>(_mm_setzero_ps());
        switch (amount) {
        case  0: return v;
        case  1: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(EntryType))), avx_cast<SmallV>(_mm_srli_si128(vHi, 1 * sizeof(EntryType))));
        case  2: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 2 * sizeof(EntryType))), avx_cast<SmallV>(_mm_srli_si128(vHi, 2 * sizeof(EntryType))));
        case  3: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 3 * sizeof(EntryType))), avx_cast<SmallV>(_mm_srli_si128(vHi, 3 * sizeof(EntryType))));
        case  4: return avx_cast<VectorType>(hi128(v));
        case  5: return avx_cast<VectorType>(_mm_srli_si128(vHi, 1 * sizeof(EntryType)));
        case  6: return avx_cast<VectorType>(_mm_srli_si128(vHi, 2 * sizeof(EntryType)));
        case  7: return avx_cast<VectorType>(_mm_srli_si128(vHi, 3 * sizeof(EntryType)));
        case -1: return concat(avx_cast<SmallV>(_mm_slli_si128(vLo, 1 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 3 * sizeof(EntryType))));
        case -2: return concat(avx_cast<SmallV>(_mm_slli_si128(vLo, 2 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 2 * sizeof(EntryType))));
        case -3: return concat(avx_cast<SmallV>(_mm_slli_si128(vLo, 3 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(EntryType))));
        case -4: return avx_cast<VectorType>(_mm256_permute2f128_ps(avx_cast<m256>(v), avx_cast<m256>(v), 0x8));
        case -5: return concat(z, avx_cast<SmallV>(_mm_slli_si128(vLo, 1 * sizeof(EntryType))));
        case -6: return concat(z, avx_cast<SmallV>(_mm_slli_si128(vLo, 2 * sizeof(EntryType))));
        case -7: return concat(z, avx_cast<SmallV>(_mm_slli_si128(vLo, 3 * sizeof(EntryType))));
        }
        return avx_cast<VectorType>(_mm256_setzero_ps());
    }
};
template<typename VectorType, typename EntryType> struct VectorShift<16, 8, VectorType, EntryType>
{
    static inline Vc_INTRINSIC VectorType shifted(VC_ALIGNED_PARAMETER(VectorType) v, int amount)
    {
        switch (amount) {
        case  0: return v;
        case  1: return avx_cast<VectorType>(_mm_srli_si128(avx_cast<m128i>(v), 1 * sizeof(EntryType)));
        case  2: return avx_cast<VectorType>(_mm_srli_si128(avx_cast<m128i>(v), 2 * sizeof(EntryType)));
        case  3: return avx_cast<VectorType>(_mm_srli_si128(avx_cast<m128i>(v), 3 * sizeof(EntryType)));
        case  4: return avx_cast<VectorType>(_mm_srli_si128(avx_cast<m128i>(v), 4 * sizeof(EntryType)));
        case  5: return avx_cast<VectorType>(_mm_srli_si128(avx_cast<m128i>(v), 5 * sizeof(EntryType)));
        case  6: return avx_cast<VectorType>(_mm_srli_si128(avx_cast<m128i>(v), 6 * sizeof(EntryType)));
        case  7: return avx_cast<VectorType>(_mm_srli_si128(avx_cast<m128i>(v), 7 * sizeof(EntryType)));
        case -1: return avx_cast<VectorType>(_mm_slli_si128(avx_cast<m128i>(v), 1 * sizeof(EntryType)));
        case -2: return avx_cast<VectorType>(_mm_slli_si128(avx_cast<m128i>(v), 2 * sizeof(EntryType)));
        case -3: return avx_cast<VectorType>(_mm_slli_si128(avx_cast<m128i>(v), 3 * sizeof(EntryType)));
        case -4: return avx_cast<VectorType>(_mm_slli_si128(avx_cast<m128i>(v), 4 * sizeof(EntryType)));
        case -5: return avx_cast<VectorType>(_mm_slli_si128(avx_cast<m128i>(v), 5 * sizeof(EntryType)));
        case -6: return avx_cast<VectorType>(_mm_slli_si128(avx_cast<m128i>(v), 6 * sizeof(EntryType)));
        case -7: return avx_cast<VectorType>(_mm_slli_si128(avx_cast<m128i>(v), 7 * sizeof(EntryType)));
        }
        return _mm_setzero_si128();
    }
};
template<typename T> inline Vc_INTRINSIC Vector<T> Vector<T>::shifted(int amount) const
{
    return VectorShift<sizeof(VectorType), Size, VectorType, EntryType>::shifted(d.v(), amount);
}
template<size_t SIMDWidth, size_t Size, typename VectorType, typename EntryType> struct VectorRotate;
template<typename VectorType, typename EntryType> struct VectorRotate<32, 4, VectorType, EntryType>
{
    typedef typename SseVectorType<VectorType>::Type SmallV;
    static inline Vc_INTRINSIC VectorType rotated(VC_ALIGNED_PARAMETER(VectorType) v, int amount)
    {
        const m128i vLo = avx_cast<m128i>(lo128(v));
        const m128i vHi = avx_cast<m128i>(hi128(v));
        switch (static_cast<unsigned int>(amount) % 4) {
        case  0: return v;
        case  1: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vLo, vHi, 1 * sizeof(EntryType))));
        case  2: return Mem::permute128<X1, X0>(v);
        case  3: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vLo, vHi, 1 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(EntryType))));
        }
        return _mm256_setzero_pd();
    }
};
template<typename VectorType, typename EntryType> struct VectorRotate<32, 8, VectorType, EntryType>
{
    typedef typename SseVectorType<VectorType>::Type SmallV;
    static inline Vc_INTRINSIC VectorType rotated(VC_ALIGNED_PARAMETER(VectorType) v, int amount)
    {
        const m128i vLo = avx_cast<m128i>(lo128(v));
        const m128i vHi = avx_cast<m128i>(hi128(v));
        switch (static_cast<unsigned int>(amount) % 8) {
        case  0: return v;
        case  1: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vLo, vHi, 1 * sizeof(EntryType))));
        case  2: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 2 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vLo, vHi, 2 * sizeof(EntryType))));
        case  3: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 3 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vLo, vHi, 3 * sizeof(EntryType))));
        case  4: return Mem::permute128<X1, X0>(v);
        case  5: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vLo, vHi, 1 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(EntryType))));
        case  6: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vLo, vHi, 2 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 2 * sizeof(EntryType))));
        case  7: return concat(avx_cast<SmallV>(_mm_alignr_epi8(vLo, vHi, 3 * sizeof(EntryType))), avx_cast<SmallV>(_mm_alignr_epi8(vHi, vLo, 3 * sizeof(EntryType))));
        }
        return avx_cast<VectorType>(_mm256_setzero_ps());
    }
};
template<typename VectorType, typename EntryType> struct VectorRotate<16, 8, VectorType, EntryType>
{
    static inline Vc_INTRINSIC VectorType rotated(VC_ALIGNED_PARAMETER(VectorType) v, int amount)
    {
        switch (static_cast<unsigned int>(amount) % 8) {
        case  0: return v;
        case  1: return avx_cast<VectorType>(_mm_alignr_epi8(v, v, 1 * sizeof(EntryType)));
        case  2: return avx_cast<VectorType>(_mm_alignr_epi8(v, v, 2 * sizeof(EntryType)));
        case  3: return avx_cast<VectorType>(_mm_alignr_epi8(v, v, 3 * sizeof(EntryType)));
        case  4: return avx_cast<VectorType>(_mm_alignr_epi8(v, v, 4 * sizeof(EntryType)));
        case  5: return avx_cast<VectorType>(_mm_alignr_epi8(v, v, 5 * sizeof(EntryType)));
        case  6: return avx_cast<VectorType>(_mm_alignr_epi8(v, v, 6 * sizeof(EntryType)));
        case  7: return avx_cast<VectorType>(_mm_alignr_epi8(v, v, 7 * sizeof(EntryType)));
        }
        return _mm_setzero_si128();
    }
};
template<typename T> inline Vc_INTRINSIC Vector<T> Vector<T>::rotated(int amount) const
{
    return VectorRotate<sizeof(VectorType), Size, VectorType, EntryType>::rotated(d.v(), amount);
    /*
    const m128i v0 = avx_cast<m128i>(d.v()[0]);
    const m128i v1 = avx_cast<m128i>(d.v()[1]);
    switch (static_cast<unsigned int>(amount) % Size) {
    case  0: return *this;
    case  1: return concat(avx_cast<m128>(_mm_alignr_epi8(v1, v0, 1 * sizeof(EntryType))), avx_cast<m128>(_mm_alignr_epi8(v0, v1, 1 * sizeof(EntryType))));
    case  2: return concat(avx_cast<m128>(_mm_alignr_epi8(v1, v0, 2 * sizeof(EntryType))), avx_cast<m128>(_mm_alignr_epi8(v0, v1, 2 * sizeof(EntryType))));
    case  3: return concat(avx_cast<m128>(_mm_alignr_epi8(v1, v0, 3 * sizeof(EntryType))), avx_cast<m128>(_mm_alignr_epi8(v0, v1, 3 * sizeof(EntryType))));
    case  4: return concat(d.v()[1], d.v()[0]);
    case  5: return concat(avx_cast<m128>(_mm_alignr_epi8(v0, v1, 1 * sizeof(EntryType))), avx_cast<m128>(_mm_alignr_epi8(v1, v0, 1 * sizeof(EntryType))));
    case  6: return concat(avx_cast<m128>(_mm_alignr_epi8(v0, v1, 2 * sizeof(EntryType))), avx_cast<m128>(_mm_alignr_epi8(v1, v0, 2 * sizeof(EntryType))));
    case  7: return concat(avx_cast<m128>(_mm_alignr_epi8(v0, v1, 3 * sizeof(EntryType))), avx_cast<m128>(_mm_alignr_epi8(v1, v0, 3 * sizeof(EntryType))));
    }
    */
}
// }}}1
} // namespace AVX
} // namespace Vc

#include "undomacros.h"

// vim: foldmethod=marker
