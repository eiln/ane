// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#ifndef __ANEC_UNARY_H__
#define __ANEC_UNARY_H__

#include "anec_base.h"

#define ANE_ABS	    0x101
#define ANE_ATAN    0x102
#define ANE_CEIL    0x103
#define ANE_CUBE    0x104
#define ANE_EXP	    0x105
#define ANE_EXP2    0x106
#define ANE_FRAC    0x107
#define ANE_INV	    0x108
#define ANE_LOG	    0x109
#define ANE_LOG10   0x10a
#define ANE_LOG2    0x10b
#define ANE_NORM    0x10c
#define ANE_RSQRT   0x10d
#define ANE_SIGMOID 0x10e
#define ANE_SIGN    0x10f
#define ANE_SQRT    0x110
#define ANE_SQUARE  0x111
#define ANE_TANH    0x112

static const struct ane_model ane_abs = {
    .name = "abs",
    .fname = ANEC_PATH "unary/abs.bin",
    .input_count = 1,
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
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_atan = {
    .name = "atan",
    .fname = ANEC_PATH "unary/atan.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x980,
        .tsk_size = 0x874,
        .krn_size = 0x100,
        .td_count = 3,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[3] = 1000, // itm0 0xfa0000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[3] = TILE_ITM,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_ceil = {
    .name = "ceil",
    .fname = ANEC_PATH "unary/ceil.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0xd00,
        .tsk_size = 0xb74,
        .krn_size = 0x180,
        .td_count = 4,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[3] = 1000, // itm0 0xfa0000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[3] = TILE_ITM,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_cube = {
    .name = "cube",
    .fname = ANEC_PATH "unary/cube.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x4580,
        .tsk_size = 0x574,
        .krn_size = 0x4000,
        .td_count = 2,
        .td_size = 0x274,
        .tiles[0] = 2, // tsk 0x8000
        .tiles[3] = 500, // itm0 0x7d0000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[3] = TILE_ITM,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_exp = {
    .name = "exp",
    .fname = ANEC_PATH "unary/exp.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x300,
        .tsk_size = 0x274,
        .krn_size = 0x80,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_exp2 = {
    .name = "exp2",
    .fname = ANEC_PATH "unary/exp2.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x300,
        .tsk_size = 0x274,
        .krn_size = 0x80,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_frac = {
    .name = "frac",
    .fname = ANEC_PATH "unary/frac.bin",
    .input_count = 2,
    .output_count = 1,
    .anec = {
        .size = 0x900,
        .tsk_size = 0x874,
        .krn_size = 0x80,
        .td_count = 3,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[3] = 1000, // itm0 0xfa0000
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

static const struct ane_model ane_inv = {
    .name = "inv",
    .fname = ANEC_PATH "unary/inv.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x300,
        .tsk_size = 0x274,
        .krn_size = 0x80,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_log = {
    .name = "log",
    .fname = ANEC_PATH "unary/log.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x600,
        .tsk_size = 0x574,
        .krn_size = 0x80,
        .td_count = 2,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_log10 = {
    .name = "log10",
    .fname = ANEC_PATH "unary/log10.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x600,
        .tsk_size = 0x574,
        .krn_size = 0x80,
        .td_count = 2,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_log2 = {
    .name = "log2",
    .fname = ANEC_PATH "unary/log2.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x300,
        .tsk_size = 0x274,
        .krn_size = 0x80,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_norm = {
    .name = "norm",
    .fname = ANEC_PATH "unary/norm.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x1900,
        .tsk_size = 0x1774,
        .krn_size = 0x180,
        .td_count = 8,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[3] = 1000, // itm0 0xfa0000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[3] = TILE_ITM,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_rsqrt = {
    .name = "rsqrt",
    .fname = ANEC_PATH "unary/rsqrt.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x300,
        .tsk_size = 0x274,
        .krn_size = 0x80,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_sigmoid = {
    .name = "sigmoid",
    .fname = ANEC_PATH "unary/sigmoid.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x600,
        .tsk_size = 0x574,
        .krn_size = 0x80,
        .td_count = 2,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_sign = {
    .name = "sign",
    .fname = ANEC_PATH "unary/sign.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x300,
        .tsk_size = 0x274,
        .krn_size = 0x80,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_sqrt = {
    .name = "sqrt",
    .fname = ANEC_PATH "unary/sqrt.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x300,
        .tsk_size = 0x274,
        .krn_size = 0x80,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_square = {
    .name = "square",
    .fname = ANEC_PATH "unary/square.bin",
    .input_count = 1,
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
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_tanh = {
    .name = "tanh",
    .fname = ANEC_PATH "unary/tanh.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x300,
        .tsk_size = 0x274,
        .krn_size = 0x80,
        .td_count = 1,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

#endif /* __ANEC_UNARY_H__ */
