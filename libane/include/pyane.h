// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __PYANE_H__
#define __PYANE_H__

#include "ane.h"

void *pyane_init(void);
int pyane_free(struct ane_nn *nn);
int pyane_exec(struct ane_nn *nn);

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
		ane_tiled_send(nn, xs[i], i);
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

int pyane_info(struct ane_nn *nn, unsigned long *src_count,
	       unsigned long *dst_count, unsigned long *x0, unsigned long *x1,
	       unsigned long *x2, unsigned long *x3, unsigned long *x4,
	       unsigned long *x5, unsigned long *x6, unsigned long *x7,
	       unsigned long *x8, unsigned long *x9, unsigned long *x10,
	       unsigned long *x11, unsigned long *x12, unsigned long *x13,
	       unsigned long *x14, unsigned long *x15, unsigned long *x16,
	       unsigned long *x17, unsigned long *x18, unsigned long *x19,
	       unsigned long *x20, unsigned long *x21, unsigned long *x22,
	       unsigned long *x23, unsigned long *x24, unsigned long *x25,
	       unsigned long *x26, unsigned long *x27, unsigned long *x28,
	       unsigned long *x29, unsigned long *x30, unsigned long *x31,
	       unsigned long *x32, unsigned long *x33, unsigned long *x34,
	       unsigned long *x35, unsigned long *x36, unsigned long *x37,
	       unsigned long *x38, unsigned long *x39, unsigned long *x40,
	       unsigned long *x41, unsigned long *x42, unsigned long *x43,
	       unsigned long *x44, unsigned long *x45, unsigned long *x46,
	       unsigned long *x47, unsigned long *x48, unsigned long *x49,
	       unsigned long *x50, unsigned long *x51, unsigned long *x52,
	       unsigned long *x53, unsigned long *x54, unsigned long *x55,
	       unsigned long *x56, unsigned long *x57, unsigned long *x58,
	       unsigned long *x59, unsigned long *x60, unsigned long *x61,
	       unsigned long *x62, unsigned long *x63, unsigned long *x64,
	       unsigned long *x65, unsigned long *x66, unsigned long *x67,
	       unsigned long *x68, unsigned long *x69, unsigned long *x70,
	       unsigned long *x71, unsigned long *x72, unsigned long *x73,
	       unsigned long *x74, unsigned long *x75, unsigned long *x76,
	       unsigned long *x77, unsigned long *x78, unsigned long *x79,
	       unsigned long *x80, unsigned long *x81, unsigned long *x82,
	       unsigned long *x83, unsigned long *x84, unsigned long *x85,
	       unsigned long *x86, unsigned long *x87, unsigned long *x88,
	       unsigned long *x89, unsigned long *x90, unsigned long *x91,
	       unsigned long *x92, unsigned long *x93, unsigned long *x94,
	       unsigned long *x95, unsigned long *x96, unsigned long *x97,
	       unsigned long *x98, unsigned long *x99, unsigned long *x100,
	       unsigned long *x101, unsigned long *x102, unsigned long *x103,
	       unsigned long *x104, unsigned long *x105, unsigned long *x106,
	       unsigned long *x107, unsigned long *x108, unsigned long *x109,
	       unsigned long *x110, unsigned long *x111, unsigned long *x112,
	       unsigned long *x113, unsigned long *x114, unsigned long *x115,
	       unsigned long *x116, unsigned long *x117, unsigned long *x118,
	       unsigned long *x119, unsigned long *x120, unsigned long *x121,
	       unsigned long *x122, unsigned long *x123, unsigned long *x124,
	       unsigned long *x125, unsigned long *x126, unsigned long *x127,
	       unsigned long *x128, unsigned long *x129, unsigned long *x130,
	       unsigned long *x131, unsigned long *x132, unsigned long *x133,
	       unsigned long *x134, unsigned long *x135, unsigned long *x136,
	       unsigned long *x137, unsigned long *x138, unsigned long *x139,
	       unsigned long *x140, unsigned long *x141, unsigned long *x142,
	       unsigned long *x143, unsigned long *x144, unsigned long *x145,
	       unsigned long *x146, unsigned long *x147, unsigned long *x148,
	       unsigned long *x149, unsigned long *x150, unsigned long *x151,
	       unsigned long *x152, unsigned long *x153, unsigned long *x154,
	       unsigned long *x155, unsigned long *x156, unsigned long *x157,
	       unsigned long *x158, unsigned long *x159, unsigned long *x160,
	       unsigned long *x161, unsigned long *x162, unsigned long *x163,
	       unsigned long *x164, unsigned long *x165, unsigned long *x166,
	       unsigned long *x167, unsigned long *x168, unsigned long *x169,
	       unsigned long *x170, unsigned long *x171, unsigned long *x172,
	       unsigned long *x173, unsigned long *x174, unsigned long *x175,
	       unsigned long *x176, unsigned long *x177, unsigned long *x178,
	       unsigned long *x179, unsigned long *x180, unsigned long *x181,
	       unsigned long *x182, unsigned long *x183, unsigned long *x184,
	       unsigned long *x185, unsigned long *x186, unsigned long *x187,
	       unsigned long *x188, unsigned long *x189, unsigned long *x190,
	       unsigned long *x191, unsigned long *x192, unsigned long *x193,
	       unsigned long *x194, unsigned long *x195, unsigned long *x196,
	       unsigned long *x197, unsigned long *x198, unsigned long *x199,
	       unsigned long *x200, unsigned long *x201, unsigned long *x202,
	       unsigned long *x203, unsigned long *x204, unsigned long *x205,
	       unsigned long *x206, unsigned long *x207, unsigned long *x208,
	       unsigned long *x209, unsigned long *x210, unsigned long *x211,
	       unsigned long *x212, unsigned long *x213, unsigned long *x214,
	       unsigned long *x215, unsigned long *x216, unsigned long *x217,
	       unsigned long *x218, unsigned long *x219, unsigned long *x220,
	       unsigned long *x221, unsigned long *x222, unsigned long *x223,
	       unsigned long *x224, unsigned long *x225, unsigned long *x226,
	       unsigned long *x227, unsigned long *x228, unsigned long *x229,
	       unsigned long *x230, unsigned long *x231, unsigned long *x232,
	       unsigned long *x233, unsigned long *x234, unsigned long *x235,
	       unsigned long *x236, unsigned long *x237, unsigned long *x238,
	       unsigned long *x239, unsigned long *x240, unsigned long *x241,
	       unsigned long *x242, unsigned long *x243, unsigned long *x244,
	       unsigned long *x245, unsigned long *x246, unsigned long *x247,
	       unsigned long *x248, unsigned long *x249, unsigned long *x250,
	       unsigned long *x251, unsigned long *x252, unsigned long *x253,
	       unsigned long *x254, unsigned long *x255, unsigned long *x256,
	       unsigned long *x257, unsigned long *x258, unsigned long *x259,
	       unsigned long *x260, unsigned long *x261, unsigned long *x262,
	       unsigned long *x263, unsigned long *x264, unsigned long *x265,
	       unsigned long *x266, unsigned long *x267, unsigned long *x268,
	       unsigned long *x269, unsigned long *x270, unsigned long *x271,
	       unsigned long *x272, unsigned long *x273, unsigned long *x274,
	       unsigned long *x275, unsigned long *x276, unsigned long *x277,
	       unsigned long *x278, unsigned long *x279, unsigned long *x280,
	       unsigned long *x281, unsigned long *x282, unsigned long *x283,
	       unsigned long *x284, unsigned long *x285, unsigned long *x286,
	       unsigned long *x287, unsigned long *x288, unsigned long *x289,
	       unsigned long *x290, unsigned long *x291, unsigned long *x292,
	       unsigned long *x293, unsigned long *x294, unsigned long *x295,
	       unsigned long *x296, unsigned long *x297, unsigned long *x298,
	       unsigned long *x299, unsigned long *x300, unsigned long *x301,
	       unsigned long *x302, unsigned long *x303, unsigned long *x304,
	       unsigned long *x305, unsigned long *x306, unsigned long *x307,
	       unsigned long *x308, unsigned long *x309, unsigned long *x310,
	       unsigned long *x311, unsigned long *x312, unsigned long *x313,
	       unsigned long *x314, unsigned long *x315, unsigned long *x316,
	       unsigned long *x317, unsigned long *x318, unsigned long *x319,
	       unsigned long *x320, unsigned long *x321, unsigned long *x322,
	       unsigned long *x323, unsigned long *x324, unsigned long *x325,
	       unsigned long *x326, unsigned long *x327, unsigned long *x328,
	       unsigned long *x329, unsigned long *x330, unsigned long *x331,
	       unsigned long *x332, unsigned long *x333, unsigned long *x334,
	       unsigned long *x335, unsigned long *x336, unsigned long *x337,
	       unsigned long *x338, unsigned long *x339, unsigned long *x340,
	       unsigned long *x341, unsigned long *x342, unsigned long *x343,
	       unsigned long *x344, unsigned long *x345, unsigned long *x346,
	       unsigned long *x347, unsigned long *x348, unsigned long *x349,
	       unsigned long *x350, unsigned long *x351, unsigned long *x352,
	       unsigned long *x353, unsigned long *x354, unsigned long *x355,
	       unsigned long *x356, unsigned long *x357, unsigned long *x358,
	       unsigned long *x359, unsigned long *x360, unsigned long *x361,
	       unsigned long *x362, unsigned long *x363, unsigned long *x364,
	       unsigned long *x365, unsigned long *x366, unsigned long *x367,
	       unsigned long *x368, unsigned long *x369, unsigned long *x370,
	       unsigned long *x371, unsigned long *x372, unsigned long *x373,
	       unsigned long *x374, unsigned long *x375, unsigned long *x376,
	       unsigned long *x377, unsigned long *x378, unsigned long *x379,
	       unsigned long *x380, unsigned long *x381, unsigned long *x382,
	       unsigned long *x383)
{
	unsigned long *is[ANE_TILE_COUNT * 6] = {
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
	unsigned long *os[ANE_TILE_COUNT * 6] = {
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
	*src_count = input_count(nn);
	*dst_count = output_count(nn);
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
