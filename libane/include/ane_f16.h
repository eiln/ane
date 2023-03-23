// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_F16_H__
#define __ANE_F16_H__

/* credits to
 * https://stackoverflow.com/a/60047308/20891128
 *
 * IEEE-754 16-bit floating-point format (without infinity):
 * 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
 */

static inline uint32_t __cast_as_uint(const float x)
{
	return *(uint32_t *)&x;
}

static inline float __cast_as_float(const uint32_t x)
{
	return *(float *)&x;
}

// clang-format off
static inline float half_to_float(const uint16_t x)
{
	const uint32_t e = (x & 0x7C00) >> 10; // exponent
	const uint32_t m = (x & 0x03FF) << 13; // mantissa
	const uint32_t v = __cast_as_uint((float)m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
	return __cast_as_float((x&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000))); // sign : normalized : denormalized
}

static inline uint16_t float_to_half(const float x)
{
	const uint32_t b = __cast_as_uint(x) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
	const uint32_t e = (b & 0x7F800000) >> 23; // exponent
	const uint32_t m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
	return (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
}
// clang-format on

#define float_to_half_array(src, dst, elem)   \
	uint16_t dst[(elem)];                 \
	for (uint64_t i = 0; i < (elem); i++) \
		dst[i] = float_to_half(src[i]);

#define half_to_float_array(src, dst, elem)   \
	float dst[(elem)];                    \
	for (uint64_t i = 0; i < (elem); i++) \
		dst[i] = half_to_float(src[i]);

#define float_to_half_c_array(src, dst) \
	float_to_half_array(src, dst, (sizeof(src) / sizeof(float)))

#define half_to_float_c_array(src, dst) \
	half_to_float_array(src, dst, (sizeof(src) / sizeof(uint16_t)))

#define init_half_array(name, elem) uint16_t name[(elem)]

#endif /* __ANE_F16_H__ */
