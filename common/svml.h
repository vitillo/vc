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

#ifndef VC_COMMON_SVML_H
#define VC_COMMON_SVML_H

#if defined(USE_INTEL_SVML)
extern "C"{
__m128 __svml_sinf4(__m128 v1);
__m128d __svml_sin2(__m128d v1);
__m128 __svml_cosf4(__m128 v1);
__m128d __svml_cos2(__m128d v1);
__m128 __svml_sincosf4(__m128 v1);
__m128d __svml_sincos2(__m128d v1);
__m128 __svml_asinf4(__m128 v1);
__m128d __svml_asin2(__m128d v1);
__m128 __svml_atanf4(__m128 v1);
__m128d __svml_atan2(__m128d v1);
__m128 __svml_atan2f4(__m128 v1, __m128 v2);
__m128d __svml_atan22(__m128d v1, __m128d v2);
__m128 __svml_logf4(__m128 v1);
__m128d __svml_log2(__m128d v1);
__m128 __svml_log2f4(__m128 v1);
__m128d __svml_log22(__m128d v1);
__m128 __svml_log10f4(__m128 v1);
__m128d __svml_log102(__m128d v1);
__m128 __svml_expf4(__m128 v1);
__m128d __svml_exp2(__m128d v1);

__m256 __svml_sinf8(__m256 v1);
__m256d __svml_sin4(__m256d v1);
__m256 __svml_cosf8(__m256 v1);
__m256d __svml_cos4(__m256d v1);
__m256 __svml_sincosf8(__m256 v1);
__m256d __svml_sincos4(__m256d v1);
__m256 __svml_asinf8(__m256 v1);
__m256d __svml_asin4(__m256d v1);
__m256 __svml_atanf8(__m256 v1);
__m256d __svml_atan4(__m256d v1);
__m256 __svml_atan2f8(__m256 v1, __m256 v2);
__m256d __svml_atan24(__m256d v1, __m256d v2);
__m256 __svml_logf8(__m256 v1);
__m256d __svml_log4(__m256d v1);
__m256 __svml_log2f8(__m256 v1);
__m256d __svml_log24(__m256d v1);
__m256 __svml_log10f8(__m256 v1);
__m256d __svml_log104(__m256d v1);
__m256 __svml_expf8(__m256 v1);
__m256d __svml_exp4(__m256d v1);
}
#endif

#endif
