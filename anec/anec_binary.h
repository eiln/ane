// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANEC_BINARY_H__
#define __ANEC_BINARY_H__

#include "anec_base.h"

#define ANE_ADD 0x201
#define ANE_AVE 0x202
#define ANE_DIV 0x203
#define ANE_MAX 0x204
#define ANE_MIN 0x205
#define ANE_MUL 0x206
#define ANE_SUB 0x207

static const struct ane_model ane_add = {
    .name = "add",
    .fname = ANEC_PATH "binary/add.bin",
    .input_count = 2,
    .output_count = 1,
    .anec = {
        .size = 0x4280,
        .tsk_size = 0x274,
        .krn_size = 0x4000,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 2, // tsk 0x8000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .tiles[6] = 500, // src1 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
        .types[6] = TILE_SRC,
    },
};

static const struct ane_model ane_ave = {
    .name = "ave",
    .fname = ANEC_PATH "binary/ave.bin",
    .input_count = 2,
    .output_count = 1,
    .anec = {
        .size = 0x4280,
        .tsk_size = 0x274,
        .krn_size = 0x4000,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 2, // tsk 0x8000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .tiles[6] = 500, // src1 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
        .types[6] = TILE_SRC,
    },
};

static const struct ane_model ane_div = {
    .name = "div",
    .fname = ANEC_PATH "binary/div.bin",
    .input_count = 2,
    .output_count = 1,
    .anec = {
        .size = 0x600,
        .tsk_size = 0x574,
        .krn_size = 0x80,
        .td_count = 2,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[3] = 500, // itm0 0x7d0000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .tiles[6] = 500, // src1 0x7d0000
        .types[0] = TILE_CMD,
        .types[3] = TILE_ITM,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
        .types[6] = TILE_SRC,
    },
};

static const struct ane_model ane_max = {
    .name = "max",
    .fname = ANEC_PATH "binary/max.bin",
    .input_count = 2,
    .output_count = 1,
    .anec = {
        .size = 0x4280,
        .tsk_size = 0x274,
        .krn_size = 0x4000,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 2, // tsk 0x8000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .tiles[6] = 500, // src1 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
        .types[6] = TILE_SRC,
    },
};

static const struct ane_model ane_min = {
    .name = "min",
    .fname = ANEC_PATH "binary/min.bin",
    .input_count = 2,
    .output_count = 1,
    .anec = {
        .size = 0x4280,
        .tsk_size = 0x274,
        .krn_size = 0x4000,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 2, // tsk 0x8000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .tiles[6] = 500, // src1 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
        .types[6] = TILE_SRC,
    },
};

static const struct ane_model ane_mul = {
    .name = "mul",
    .fname = ANEC_PATH "binary/mul.bin",
    .input_count = 2,
    .output_count = 1,
    .anec = {
        .size = 0x4280,
        .tsk_size = 0x274,
        .krn_size = 0x4000,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 2, // tsk 0x8000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .tiles[6] = 500, // src1 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
        .types[6] = TILE_SRC,
    },
};

static const struct ane_model ane_sub = {
    .name = "sub",
    .fname = ANEC_PATH "binary/sub.bin",
    .input_count = 2,
    .output_count = 1,
    .anec = {
        .size = 0x4280,
        .tsk_size = 0x274,
        .krn_size = 0x4000,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 2, // tsk 0x8000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .tiles[6] = 500, // src1 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
        .types[6] = TILE_SRC,
    },
};

#endif /* __ANEC_BINARY_H__ */
