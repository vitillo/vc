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

#ifndef VC_COMMON_STORAGE_H
#define VC_COMMON_STORAGE_H

#include "aliasingentryhelper.h"
#include "macros.h"
#include "types.h"

namespace Vc
{
namespace Common
{

template<typename _VectorType, typename _EntryType, typename VectorTypeBase = _VectorType> class VectorMemoryUnion
{
    public:
        typedef _VectorType VectorType;
        typedef _EntryType EntryType;
        typedef EntryType AliasingEntryType Vc_MAY_ALIAS;
        inline VectorMemoryUnion() { assertCorrectAlignment(&v()); }
#if defined VC_ICC || defined VC_MSVC
        inline VectorMemoryUnion(const VectorType &x) { data.v = x; assertCorrectAlignment(&data.v); }
        inline VectorMemoryUnion &operator=(const VectorType &x) {
            data.v = x; return *this;
        }

        VectorType &v() { return reinterpret_cast<VectorType &>(data.v); }
        const VectorType &v() const { return reinterpret_cast<const VectorType &>(data.v); }

#if defined VC_ICC
        AliasingEntryHelper<VectorMemoryUnion> m(size_t index) {
            return AliasingEntryHelper<VectorMemoryUnion>(this, index);
        }
        void assign(size_t index, EntryType x) {
            data.m[index] = x;
        }
        EntryType read(size_t index) const {
            return data.m[index];
        }
#else
        EntryType &m(size_t index) {
            return data.m[index];
        }
#endif

        EntryType m(size_t index) const {
            return data.m[index];
        }

    private:
        union VectorScalarUnion {
            VectorTypeBase v;
            EntryType m[sizeof(VectorTypeBase)/sizeof(EntryType)];
        } data;
#else
        inline VectorMemoryUnion(VectorType x) : data(x) { assertCorrectAlignment(&data); }
        inline VectorMemoryUnion &operator=(VectorType x) {
            data = x; return *this;
        }

        inline VectorType &v() { return data; }
        inline const VectorType &v() const { return data; }

        inline AliasingEntryType &m(size_t index) {
            return reinterpret_cast<AliasingEntryType *>(&data)[index];
        }

        inline EntryType m(size_t index) const {
            return reinterpret_cast<const AliasingEntryType *>(&data)[index];
        }

    private:
        VectorType data;
#endif
};

#if VC_GCC == 0x40700 || (VC_GCC >= 0x40600 && VC_GCC <= 0x40603)
// workaround bug 52736 in GCC
template<typename T, typename V> static inline T &vectorMemoryUnionAliasedMember(V *data, size_t index) {
    if (__builtin_constant_p(index) && index == 0) {
        T *ret;
        asm("mov %1,%0" : "=r"(ret) : "r"(data));
        return *ret;
    } else {
        return reinterpret_cast<T *>(data)[index];
    }
}
template<> inline VectorMemoryUnion<__m128d, double>::AliasingEntryType &VectorMemoryUnion<__m128d, double>::m(size_t index) {
    return vectorMemoryUnionAliasedMember<AliasingEntryType>(&data, index);
}
template<> inline VectorMemoryUnion<__m128i, long long>::AliasingEntryType &VectorMemoryUnion<__m128i, long long>::m(size_t index) {
    return vectorMemoryUnionAliasedMember<AliasingEntryType>(&data, index);
}
template<> inline VectorMemoryUnion<__m128i, unsigned long long>::AliasingEntryType &VectorMemoryUnion<__m128i, unsigned long long>::m(size_t index) {
    return vectorMemoryUnionAliasedMember<AliasingEntryType>(&data, index);
}
#endif

} // namespace Common
} // namespace Vc

#include "undomacros.h"

#endif // VC_COMMON_STORAGE_H
