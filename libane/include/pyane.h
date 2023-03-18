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

int pyane_nchw(struct ane_nn *nn, uint64_t *x0, uint64_t *x1, uint64_t *x2,
	       uint64_t *x3, uint64_t *x4, uint64_t *x5, uint64_t *x6,
	       uint64_t *x7, uint64_t *x8, uint64_t *x9, uint64_t *x10,
	       uint64_t *x11, uint64_t *x12, uint64_t *x13, uint64_t *x14,
	       uint64_t *x15, uint64_t *x16, uint64_t *x17, uint64_t *x18,
	       uint64_t *x19, uint64_t *x20, uint64_t *x21, uint64_t *x22,
	       uint64_t *x23, uint64_t *x24, uint64_t *x25, uint64_t *x26,
	       uint64_t *x27, uint64_t *x28, uint64_t *x29, uint64_t *x30,
	       uint64_t *x31, uint64_t *x32, uint64_t *x33, uint64_t *x34,
	       uint64_t *x35, uint64_t *x36, uint64_t *x37, uint64_t *x38,
	       uint64_t *x39, uint64_t *x40, uint64_t *x41, uint64_t *x42,
	       uint64_t *x43, uint64_t *x44, uint64_t *x45, uint64_t *x46,
	       uint64_t *x47, uint64_t *x48, uint64_t *x49, uint64_t *x50,
	       uint64_t *x51, uint64_t *x52, uint64_t *x53, uint64_t *x54,
	       uint64_t *x55, uint64_t *x56, uint64_t *x57, uint64_t *x58,
	       uint64_t *x59, uint64_t *x60, uint64_t *x61, uint64_t *x62,
	       uint64_t *x63, uint64_t *x64, uint64_t *x65, uint64_t *x66,
	       uint64_t *x67, uint64_t *x68, uint64_t *x69, uint64_t *x70,
	       uint64_t *x71, uint64_t *x72, uint64_t *x73, uint64_t *x74,
	       uint64_t *x75, uint64_t *x76, uint64_t *x77, uint64_t *x78,
	       uint64_t *x79, uint64_t *x80, uint64_t *x81, uint64_t *x82,
	       uint64_t *x83, uint64_t *x84, uint64_t *x85, uint64_t *x86,
	       uint64_t *x87, uint64_t *x88, uint64_t *x89, uint64_t *x90,
	       uint64_t *x91, uint64_t *x92, uint64_t *x93, uint64_t *x94,
	       uint64_t *x95, uint64_t *x96, uint64_t *x97, uint64_t *x98,
	       uint64_t *x99, uint64_t *x100, uint64_t *x101, uint64_t *x102,
	       uint64_t *x103, uint64_t *x104, uint64_t *x105, uint64_t *x106,
	       uint64_t *x107, uint64_t *x108, uint64_t *x109, uint64_t *x110,
	       uint64_t *x111, uint64_t *x112, uint64_t *x113, uint64_t *x114,
	       uint64_t *x115, uint64_t *x116, uint64_t *x117, uint64_t *x118,
	       uint64_t *x119, uint64_t *x120, uint64_t *x121, uint64_t *x122,
	       uint64_t *x123, uint64_t *x124, uint64_t *x125, uint64_t *x126,
	       uint64_t *x127, uint64_t *x128, uint64_t *x129, uint64_t *x130,
	       uint64_t *x131, uint64_t *x132, uint64_t *x133, uint64_t *x134,
	       uint64_t *x135, uint64_t *x136, uint64_t *x137, uint64_t *x138,
	       uint64_t *x139, uint64_t *x140, uint64_t *x141, uint64_t *x142,
	       uint64_t *x143, uint64_t *x144, uint64_t *x145, uint64_t *x146,
	       uint64_t *x147, uint64_t *x148, uint64_t *x149, uint64_t *x150,
	       uint64_t *x151, uint64_t *x152, uint64_t *x153, uint64_t *x154,
	       uint64_t *x155, uint64_t *x156, uint64_t *x157, uint64_t *x158,
	       uint64_t *x159, uint64_t *x160, uint64_t *x161, uint64_t *x162,
	       uint64_t *x163, uint64_t *x164, uint64_t *x165, uint64_t *x166,
	       uint64_t *x167, uint64_t *x168, uint64_t *x169, uint64_t *x170,
	       uint64_t *x171, uint64_t *x172, uint64_t *x173, uint64_t *x174,
	       uint64_t *x175, uint64_t *x176, uint64_t *x177, uint64_t *x178,
	       uint64_t *x179, uint64_t *x180, uint64_t *x181, uint64_t *x182,
	       uint64_t *x183, uint64_t *x184, uint64_t *x185, uint64_t *x186,
	       uint64_t *x187, uint64_t *x188, uint64_t *x189, uint64_t *x190,
	       uint64_t *x191, uint64_t *x192, uint64_t *x193, uint64_t *x194,
	       uint64_t *x195, uint64_t *x196, uint64_t *x197, uint64_t *x198,
	       uint64_t *x199, uint64_t *x200, uint64_t *x201, uint64_t *x202,
	       uint64_t *x203, uint64_t *x204, uint64_t *x205, uint64_t *x206,
	       uint64_t *x207, uint64_t *x208, uint64_t *x209, uint64_t *x210,
	       uint64_t *x211, uint64_t *x212, uint64_t *x213, uint64_t *x214,
	       uint64_t *x215, uint64_t *x216, uint64_t *x217, uint64_t *x218,
	       uint64_t *x219, uint64_t *x220, uint64_t *x221, uint64_t *x222,
	       uint64_t *x223, uint64_t *x224, uint64_t *x225, uint64_t *x226,
	       uint64_t *x227, uint64_t *x228, uint64_t *x229, uint64_t *x230,
	       uint64_t *x231, uint64_t *x232, uint64_t *x233, uint64_t *x234,
	       uint64_t *x235, uint64_t *x236, uint64_t *x237, uint64_t *x238,
	       uint64_t *x239, uint64_t *x240, uint64_t *x241, uint64_t *x242,
	       uint64_t *x243, uint64_t *x244, uint64_t *x245, uint64_t *x246,
	       uint64_t *x247, uint64_t *x248, uint64_t *x249, uint64_t *x250,
	       uint64_t *x251, uint64_t *x252, uint64_t *x253, uint64_t *x254,
	       uint64_t *x255, uint64_t *x256, uint64_t *x257, uint64_t *x258,
	       uint64_t *x259, uint64_t *x260, uint64_t *x261, uint64_t *x262,
	       uint64_t *x263, uint64_t *x264, uint64_t *x265, uint64_t *x266,
	       uint64_t *x267, uint64_t *x268, uint64_t *x269, uint64_t *x270,
	       uint64_t *x271, uint64_t *x272, uint64_t *x273, uint64_t *x274,
	       uint64_t *x275, uint64_t *x276, uint64_t *x277, uint64_t *x278,
	       uint64_t *x279, uint64_t *x280, uint64_t *x281, uint64_t *x282,
	       uint64_t *x283, uint64_t *x284, uint64_t *x285, uint64_t *x286,
	       uint64_t *x287, uint64_t *x288, uint64_t *x289, uint64_t *x290,
	       uint64_t *x291, uint64_t *x292, uint64_t *x293, uint64_t *x294,
	       uint64_t *x295, uint64_t *x296, uint64_t *x297, uint64_t *x298,
	       uint64_t *x299, uint64_t *x300, uint64_t *x301, uint64_t *x302,
	       uint64_t *x303, uint64_t *x304, uint64_t *x305, uint64_t *x306,
	       uint64_t *x307, uint64_t *x308, uint64_t *x309, uint64_t *x310,
	       uint64_t *x311, uint64_t *x312, uint64_t *x313, uint64_t *x314,
	       uint64_t *x315, uint64_t *x316, uint64_t *x317, uint64_t *x318,
	       uint64_t *x319, uint64_t *x320, uint64_t *x321, uint64_t *x322,
	       uint64_t *x323, uint64_t *x324, uint64_t *x325, uint64_t *x326,
	       uint64_t *x327, uint64_t *x328, uint64_t *x329, uint64_t *x330,
	       uint64_t *x331, uint64_t *x332, uint64_t *x333, uint64_t *x334,
	       uint64_t *x335, uint64_t *x336, uint64_t *x337, uint64_t *x338,
	       uint64_t *x339, uint64_t *x340, uint64_t *x341, uint64_t *x342,
	       uint64_t *x343, uint64_t *x344, uint64_t *x345, uint64_t *x346,
	       uint64_t *x347, uint64_t *x348, uint64_t *x349, uint64_t *x350,
	       uint64_t *x351, uint64_t *x352, uint64_t *x353, uint64_t *x354,
	       uint64_t *x355, uint64_t *x356, uint64_t *x357, uint64_t *x358,
	       uint64_t *x359, uint64_t *x360, uint64_t *x361, uint64_t *x362,
	       uint64_t *x363, uint64_t *x364, uint64_t *x365, uint64_t *x366,
	       uint64_t *x367, uint64_t *x368, uint64_t *x369, uint64_t *x370,
	       uint64_t *x371, uint64_t *x372, uint64_t *x373, uint64_t *x374,
	       uint64_t *x375, uint64_t *x376, uint64_t *x377, uint64_t *x378,
	       uint64_t *x379, uint64_t *x380, uint64_t *x381, uint64_t *x382,
	       uint64_t *x383)
{
	uint64_t *is[ANE_TILE_COUNT * 6] = {
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
	uint64_t *os[ANE_TILE_COUNT * 6] = {
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
