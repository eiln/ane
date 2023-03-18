// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __PYANE_H__
#define __PYANE_H__

#include "ane.h"
#include "ane_nchw.h"

int pyane_free(struct ane_nn *nn)
{
	ane_free(nn);
	return 0;
}

int pyane_exec(struct ane_nn *nn)
{
	int err = ane_exec(nn);
	return err;
}

int pyane_tile(struct ane_nn *nn, void *data, void *tile, int idx)
{
	nchw_tile(data, tile, (uint64_t *)nn->model->nchw[nn->src_bdx[idx]]);
	return 0;
}

// im sorry

int pyane_send(struct ane_nn *nn, void *x0, void *x1, void *x2, void *x3,
	       void *x4, void *x5, void *x6, void *x7, void *x8, void *x9,
	       void *x10, void *x11, void *x12, void *x13, void *x14, void *x15,
	       void *x16, void *x17, void *x18, void *x19, void *x20, void *x21,
	       void *x22, void *x23, void *x24, void *x25, void *x26, void *x27,
	       void *x28, void *x29, void *x30, void *x31)
{
	void *xs[ANE_TILE_COUNT] = { x0,  x1,  x2,  x3,	 x4,  x5,  x6,	x7,
				     x8,  x9,  x10, x11, x12, x13, x14, x15,
				     x16, x17, x18, x19, x20, x21, x22, x23,
				     x24, x25, x26, x27, x28, x29, x30, x31 };
	for (int i = 0; i < input_count(nn); i++) {
		ane_send(nn, xs[i], i);
	}
	return 0;
}

int pyane_read(struct ane_nn *nn, void *x0, void *x1, void *x2, void *x3,
	       void *x4, void *x5, void *x6, void *x7, void *x8, void *x9,
	       void *x10, void *x11, void *x12, void *x13, void *x14, void *x15,
	       void *x16, void *x17, void *x18, void *x19, void *x20, void *x21,
	       void *x22, void *x23, void *x24, void *x25, void *x26, void *x27,
	       void *x28, void *x29, void *x30, void *x31)
{
	void *xs[ANE_TILE_COUNT] = { x0,  x1,  x2,  x3,	 x4,  x5,  x6,	x7,
				     x8,  x9,  x10, x11, x12, x13, x14, x15,
				     x16, x17, x18, x19, x20, x21, x22, x23,
				     x24, x25, x26, x27, x28, x29, x30, x31 };
	for (int i = 0; i < output_count(nn); i++) {
		ane_read(nn, xs[i], i);
	}
	return 0;
}

int pyane_info(struct ane_nn *nn, int *src_count, int *dst_count)
{
	*src_count = input_count(nn);
	*dst_count = output_count(nn);
	return 0;
}

int pyane_size(struct ane_nn *nn, long *x0, long *x1, long *x2, long *x3,
	       long *x4, long *x5, long *x6, long *x7, long *x8, long *x9,
	       long *x10, long *x11, long *x12, long *x13, long *x14, long *x15,
	       long *x16, long *x17, long *x18, long *x19, long *x20, long *x21,
	       long *x22, long *x23, long *x24, long *x25, long *x26, long *x27,
	       long *x28, long *x29, long *x30, long *x31, long *x32, long *x33,
	       long *x34, long *x35, long *x36, long *x37, long *x38, long *x39,
	       long *x40, long *x41, long *x42, long *x43, long *x44, long *x45,
	       long *x46, long *x47, long *x48, long *x49, long *x50, long *x51,
	       long *x52, long *x53, long *x54, long *x55, long *x56, long *x57,
	       long *x58, long *x59, long *x60, long *x61, long *x62, long *x63)
{
	long *is[ANE_TILE_COUNT] = { x0,  x1,  x2,  x3,	 x4,  x5,  x6,	x7,
				     x8,  x9,  x10, x11, x12, x13, x14, x15,
				     x16, x17, x18, x19, x20, x21, x22, x23,
				     x24, x25, x26, x27, x28, x29, x30, x31 };
	long *os[ANE_TILE_COUNT] = { x32, x33, x34, x35, x36, x37, x38, x39,
				     x40, x41, x42, x43, x44, x45, x46, x47,
				     x48, x49, x50, x51, x52, x53, x54, x55,
				     x56, x57, x58, x59, x60, x61, x62, x63 };
	for (int i = 0; i < input_count(nn); i++) {
		*is[i] = input_size(nn, i);
	}
	for (int i = 0; i < output_count(nn); i++) {
		*os[i] = output_size(nn, i);
	}
	return 0;
}

int pyane_nchw(
	struct ane_nn *nn, long *x0, long *x1, long *x2, long *x3, long *x4,
	long *x5, long *x6, long *x7, long *x8, long *x9, long *x10, long *x11,
	long *x12, long *x13, long *x14, long *x15, long *x16, long *x17,
	long *x18, long *x19, long *x20, long *x21, long *x22, long *x23,
	long *x24, long *x25, long *x26, long *x27, long *x28, long *x29,
	long *x30, long *x31, long *x32, long *x33, long *x34, long *x35,
	long *x36, long *x37, long *x38, long *x39, long *x40, long *x41,
	long *x42, long *x43, long *x44, long *x45, long *x46, long *x47,
	long *x48, long *x49, long *x50, long *x51, long *x52, long *x53,
	long *x54, long *x55, long *x56, long *x57, long *x58, long *x59,
	long *x60, long *x61, long *x62, long *x63, long *x64, long *x65,
	long *x66, long *x67, long *x68, long *x69, long *x70, long *x71,
	long *x72, long *x73, long *x74, long *x75, long *x76, long *x77,
	long *x78, long *x79, long *x80, long *x81, long *x82, long *x83,
	long *x84, long *x85, long *x86, long *x87, long *x88, long *x89,
	long *x90, long *x91, long *x92, long *x93, long *x94, long *x95,
	long *x96, long *x97, long *x98, long *x99, long *x100, long *x101,
	long *x102, long *x103, long *x104, long *x105, long *x106, long *x107,
	long *x108, long *x109, long *x110, long *x111, long *x112, long *x113,
	long *x114, long *x115, long *x116, long *x117, long *x118, long *x119,
	long *x120, long *x121, long *x122, long *x123, long *x124, long *x125,
	long *x126, long *x127, long *x128, long *x129, long *x130, long *x131,
	long *x132, long *x133, long *x134, long *x135, long *x136, long *x137,
	long *x138, long *x139, long *x140, long *x141, long *x142, long *x143,
	long *x144, long *x145, long *x146, long *x147, long *x148, long *x149,
	long *x150, long *x151, long *x152, long *x153, long *x154, long *x155,
	long *x156, long *x157, long *x158, long *x159, long *x160, long *x161,
	long *x162, long *x163, long *x164, long *x165, long *x166, long *x167,
	long *x168, long *x169, long *x170, long *x171, long *x172, long *x173,
	long *x174, long *x175, long *x176, long *x177, long *x178, long *x179,
	long *x180, long *x181, long *x182, long *x183, long *x184, long *x185,
	long *x186, long *x187, long *x188, long *x189, long *x190, long *x191,
	long *x192, long *x193, long *x194, long *x195, long *x196, long *x197,
	long *x198, long *x199, long *x200, long *x201, long *x202, long *x203,
	long *x204, long *x205, long *x206, long *x207, long *x208, long *x209,
	long *x210, long *x211, long *x212, long *x213, long *x214, long *x215,
	long *x216, long *x217, long *x218, long *x219, long *x220, long *x221,
	long *x222, long *x223, long *x224, long *x225, long *x226, long *x227,
	long *x228, long *x229, long *x230, long *x231, long *x232, long *x233,
	long *x234, long *x235, long *x236, long *x237, long *x238, long *x239,
	long *x240, long *x241, long *x242, long *x243, long *x244, long *x245,
	long *x246, long *x247, long *x248, long *x249, long *x250, long *x251,
	long *x252, long *x253, long *x254, long *x255, long *x256, long *x257,
	long *x258, long *x259, long *x260, long *x261, long *x262, long *x263,
	long *x264, long *x265, long *x266, long *x267, long *x268, long *x269,
	long *x270, long *x271, long *x272, long *x273, long *x274, long *x275,
	long *x276, long *x277, long *x278, long *x279, long *x280, long *x281,
	long *x282, long *x283, long *x284, long *x285, long *x286, long *x287,
	long *x288, long *x289, long *x290, long *x291, long *x292, long *x293,
	long *x294, long *x295, long *x296, long *x297, long *x298, long *x299,
	long *x300, long *x301, long *x302, long *x303, long *x304, long *x305,
	long *x306, long *x307, long *x308, long *x309, long *x310, long *x311,
	long *x312, long *x313, long *x314, long *x315, long *x316, long *x317,
	long *x318, long *x319, long *x320, long *x321, long *x322, long *x323,
	long *x324, long *x325, long *x326, long *x327, long *x328, long *x329,
	long *x330, long *x331, long *x332, long *x333, long *x334, long *x335,
	long *x336, long *x337, long *x338, long *x339, long *x340, long *x341,
	long *x342, long *x343, long *x344, long *x345, long *x346, long *x347,
	long *x348, long *x349, long *x350, long *x351, long *x352, long *x353,
	long *x354, long *x355, long *x356, long *x357, long *x358, long *x359,
	long *x360, long *x361, long *x362, long *x363, long *x364, long *x365,
	long *x366, long *x367, long *x368, long *x369, long *x370, long *x371,
	long *x372, long *x373, long *x374, long *x375, long *x376, long *x377,
	long *x378, long *x379, long *x380, long *x381, long *x382, long *x383)
{
	long *is[ANE_TILE_COUNT * 6] = {
		x0,   x1,   x2,	  x3,	x4,   x5,   x6,	  x7,	x8,   x9,
		x10,  x11,  x12,  x13,	x14,  x15,  x16,  x17,	x18,  x19,
		x20,  x21,  x22,  x23,	x24,  x25,  x26,  x27,	x28,  x29,
		x30,  x31,  x32,  x33,	x34,  x35,  x36,  x37,	x38,  x39,
		x40,  x41,  x42,  x43,	x44,  x45,  x46,  x47,	x48,  x49,
		x50,  x51,  x52,  x53,	x54,  x55,  x56,  x57,	x58,  x59,
		x60,  x61,  x62,  x63,	x64,  x65,  x66,  x67,	x68,  x69,
		x70,  x71,  x72,  x73,	x74,  x75,  x76,  x77,	x78,  x79,
		x80,  x81,  x82,  x83,	x84,  x85,  x86,  x87,	x88,  x89,
		x90,  x91,  x92,  x93,	x94,  x95,  x96,  x97,	x98,  x99,
		x100, x101, x102, x103, x104, x105, x106, x107, x108, x109,
		x110, x111, x112, x113, x114, x115, x116, x117, x118, x119,
		x120, x121, x122, x123, x124, x125, x126, x127, x128, x129,
		x130, x131, x132, x133, x134, x135, x136, x137, x138, x139,
		x140, x141, x142, x143, x144, x145, x146, x147, x148, x149,
		x150, x151, x152, x153, x154, x155, x156, x157, x158, x159,
		x160, x161, x162, x163, x164, x165, x166, x167, x168, x169,
		x170, x171, x172, x173, x174, x175, x176, x177, x178, x179,
		x180, x181, x182, x183, x184, x185, x186, x187, x188, x189,
		x190, x191
	};
	long *os[ANE_TILE_COUNT * 6] = {
		x192, x193, x194, x195, x196, x197, x198, x199, x200, x201,
		x202, x203, x204, x205, x206, x207, x208, x209, x210, x211,
		x212, x213, x214, x215, x216, x217, x218, x219, x220, x221,
		x222, x223, x224, x225, x226, x227, x228, x229, x230, x231,
		x232, x233, x234, x235, x236, x237, x238, x239, x240, x241,
		x242, x243, x244, x245, x246, x247, x248, x249, x250, x251,
		x252, x253, x254, x255, x256, x257, x258, x259, x260, x261,
		x262, x263, x264, x265, x266, x267, x268, x269, x270, x271,
		x272, x273, x274, x275, x276, x277, x278, x279, x280, x281,
		x282, x283, x284, x285, x286, x287, x288, x289, x290, x291,
		x292, x293, x294, x295, x296, x297, x298, x299, x300, x301,
		x302, x303, x304, x305, x306, x307, x308, x309, x310, x311,
		x312, x313, x314, x315, x316, x317, x318, x319, x320, x321,
		x322, x323, x324, x325, x326, x327, x328, x329, x330, x331,
		x332, x333, x334, x335, x336, x337, x338, x339, x340, x341,
		x342, x343, x344, x345, x346, x347, x348, x349, x350, x351,
		x352, x353, x354, x355, x356, x357, x358, x359, x360, x361,
		x362, x363, x364, x365, x366, x367, x368, x369, x370, x371,
		x372, x373, x374, x375, x376, x377, x378, x379, x380, x381,
		x382, x383
	};
	for (int i = 0; i < input_count(nn); i++) {
		int bdx = nn->src_bdx[i];
		for (int j = 0; j < 6; j++) {
			*is[i * 6 + j] = nn->model->nchw[bdx][j];
		}
	}
	for (int i = 0; i < output_count(nn); i++) {
		int bdx = nn->dst_bdx[i];
		for (int j = 0; j < 6; j++) {
			*os[i * 6 + j] = nn->model->nchw[bdx][j];
		}
	}
	return 0;
}

#endif /* __PYANE_H__ */
