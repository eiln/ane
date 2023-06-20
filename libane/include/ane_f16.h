// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANE_F16_H__
#define __ANE_F16_H__

#include <math.h>
#include <stdint.h>

/* FP16 <-> FP32 */
/* ref: https://github.com/ggerganov/ggml */
/* ref: https://github.com/Maratyszcza/FP16 */

static inline float f32_from_bits(uint32_t w)
{
	union {
		uint32_t as_bits;
		float as_value;
	} f32;
	f32.as_bits = w;
	return f32.as_value;
}

static inline uint32_t f32_to_bits(float f)
{
	union {
		float as_value;
		uint32_t as_bits;
	} f32;
	f32.as_value = f;
	return f32.as_bits;
}

static inline float ane_compute_f16_to_f32(const uint16_t h)
{
	const uint32_t w = (uint32_t)h << 16;
	const uint32_t sign = w & UINT32_C(0x80000000);
	const uint32_t two_w = w + w;

	const uint32_t exp_offset = UINT32_C(0xE0) << 23;
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) || \
	defined(__GNUC__) && !defined(__STRICT_ANSI__)
	const float exp_scale = 0x1.0p-112f;
#else
	const float exp_scale = f32_from_bits(UINT32_C(0x7800000));
#endif
	const float normalized_value =
		f32_from_bits((two_w >> 4) + exp_offset) * exp_scale;

	const uint32_t magic_mask = UINT32_C(126) << 23;
	const float magic_bias = 0.5f;
	const float denormalized_value =
		f32_from_bits((two_w >> 17) | magic_mask) - magic_bias;

	const uint32_t denormalized_cutoff = UINT32_C(1) << 27;
	const uint32_t result = sign |
				(two_w < denormalized_cutoff ?
					 f32_to_bits(denormalized_value) :
					 f32_to_bits(normalized_value));
	return f32_from_bits(result);
}

static inline uint16_t ane_compute_f32_to_f16(const float f)
{
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) || \
	defined(__GNUC__) && !defined(__STRICT_ANSI__)
	const float scale_to_inf = 0x1.0p+112f;
	const float scale_to_zero = 0x1.0p-110f;
#else
	const float scale_to_inf = f32_from_bits(UINT32_C(0x77800000));
	const float scale_to_zero = f32_from_bits(UINT32_C(0x08800000));
#endif
	float base = (fabsf(f) * scale_to_inf) * scale_to_zero;

	const uint32_t w = f32_to_bits(f);
	const uint32_t shl1_w = w + w;
	const uint32_t sign = w & UINT32_C(0x80000000);
	uint32_t bias = shl1_w & UINT32_C(0xFF000000);
	if (bias < UINT32_C(0x71000000)) {
		bias = UINT32_C(0x71000000);
	}

	base = f32_from_bits((bias >> 1) + UINT32_C(0x07800000)) + base;
	const uint32_t bits = f32_to_bits(base);
	const uint32_t exp_bits = (bits >> 13) & UINT32_C(0x00007C00);
	const uint32_t mantissa_bits = bits & UINT32_C(0x00000FFF);
	const uint32_t nonsign = exp_bits + mantissa_bits;
	return (sign >> 16) |
	       (shl1_w > UINT32_C(0xFF000000) ? UINT16_C(0x7E00) : nonsign);
}

static inline void ane_f16_to_f32_row(const uint16_t *x, float *y,
				      const uint64_t n)
{
	for (uint64_t i = 0; i < n; i++) {
		y[i] = ane_compute_f16_to_f32(x[i]);
	}
}

static inline void ane_f32_to_f16_row(const float *x, uint16_t *y,
				      const uint64_t n)
{
	for (uint64_t i = 0; i < n; i++) {
		y[i] = ane_compute_f32_to_f16(x[i]);
	}
}

#define ane_f16_to_f32(x) ane_compute_f16_to_f32(x)
#define ane_f32_to_f16(x) ane_compute_f32_to_f16(x)

#endif /* __ANE_F16_H__ */
