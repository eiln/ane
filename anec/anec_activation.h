#ifndef __ANEC_ACTIVATION_H__
#define __ANEC_ACTIVATION_H__

#include "anec_base.h"

#define ANE_GELU       0x301
#define ANE_HARDTANH   0x302
#define ANE_LEAKYRELU  0x303
#define ANE_LOGSOFTMAX 0x304
#define ANE_RELU       0x305
#define ANE_RELU6      0x306
#define ANE_RRELU      0x307
#define ANE_SELU       0x308
#define ANE_SILU       0x309
#define ANE_SOFTMAX    0x30a
#define ANE_SOFTMIN    0x30b

static const struct ane_model ane_gelu = {
    .name = "gelu",
    .fname = ANEC_PATH "activation/gelu.bin",
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

static const struct ane_model ane_hardtanh = {
    .name = "hardtanh",
    .fname = ANEC_PATH "activation/hardtanh.bin",
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

static const struct ane_model ane_leakyrelu = {
    .name = "leakyrelu",
    .fname = ANEC_PATH "activation/leakyrelu.bin",
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

static const struct ane_model ane_logsoftmax = {
    .name = "logsoftmax",
    .fname = ANEC_PATH "activation/logsoftmax.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x1580,
        .tsk_size = 0x1474,
        .krn_size = 0x100,
        .td_count = 7,
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

static const struct ane_model ane_relu = {
    .name = "relu",
    .fname = ANEC_PATH "activation/relu.bin",
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

static const struct ane_model ane_relu6 = {
    .name = "relu6",
    .fname = ANEC_PATH "activation/relu6.bin",
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

static const struct ane_model ane_rrelu = {
    .name = "rrelu",
    .fname = ANEC_PATH "activation/rrelu.bin",
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

static const struct ane_model ane_selu = {
    .name = "selu",
    .fname = ANEC_PATH "activation/selu.bin",
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

static const struct ane_model ane_silu = {
    .name = "silu",
    .fname = ANEC_PATH "activation/silu.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x900,
        .tsk_size = 0x874,
        .krn_size = 0x80,
        .td_count = 3,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[3] = 500, // itm0 0x7d0000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[3] = TILE_ITM,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_softmax = {
    .name = "softmax",
    .fname = ANEC_PATH "activation/softmax.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0xf00,
        .tsk_size = 0xe74,
        .krn_size = 0x80,
        .td_count = 5,
        .td_size = 0x274,
        .tiles[0] = 1, // tsk 0x4000
        .tiles[3] = 500, // itm0 0x7d0000
        .tiles[4] = 500, // dst0 0x7d0000
        .tiles[5] = 500, // src0 0x7d0000
        .types[0] = TILE_CMD,
        .types[3] = TILE_ITM,
        .types[4] = TILE_DST,
        .types[5] = TILE_SRC,
    },
};

static const struct ane_model ane_softmin = {
    .name = "softmin",
    .fname = ANEC_PATH "activation/softmin.bin",
    .input_count = 1,
    .output_count = 1,
    .anec = {
        .size = 0x1900,
        .tsk_size = 0x1774,
        .krn_size = 0x180,
        .td_count = 6,
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

#endif /* __ANEC_ACTIVATION_H__ */
