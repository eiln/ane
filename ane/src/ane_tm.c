// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */

#define pr_fmt(fmt) "%s: %s: " fmt, KBUILD_MODNAME, __func__

#include <linux/iopoll.h>

#include "ane_tm.h"

#define ANE_TQ_COUNT 8
static const int TQ_PRTY_TABLE[ANE_TQ_COUNT] = { 0x1, 0x2, 0x3,	 0x4,
						 0x5, 0x6, 0x1e, 0x1f };

#define TM_ADDR		  0x0
#define TM_INFO		  0x4
#define TM_PUSH		  0x8
#define TM_TQ_EN	  0xc

#define TM_IRQ_EVTC(line) (0x14 + (line * 0x14))
#define TM_IRQ_INFO(line) (0x18 + (line * 0x14))
#define TM_IRQ_UNK1(line) (0x1c + (line * 0x14))
#define TM_IRQ_TMST(line) (0x20 + (line * 0x14))
#define TM_IRQ_UNK2(line) (0x24 + (line * 0x14))

#define TM_COMMITTED	  0x44
#define TM_STATUS	  0x54
#define TM_ENG_ERR1	  0x58
#define TM_ENG_ERR2	  0x5c
#define TM_ENG_ERR3	  0x60
#define TM_IRQ_EN1	  0x68
#define TM_IRQ_ACK	  0x6c
#define TM_IRQ_EN2	  0x70

#define TQ_STATUS(qid)	  (0x000 + (qid * 0x148))
#define TQ_PRTY(qid)	  (0x010 + (qid * 0x148))
#define TQ_FREESPACE(qid) (0x014 + (qid * 0x148))
#define TQ_INFO(qid)	  (0x01c + (qid * 0x148))

#define TQ_BAR1(qid, bdx) (0x020 + (qid * 0x148) + (bdx * 0x4))
#define TQ_NID1(qid)	  (0x0a0 + (qid * 0x148))
#define TQ_SIZE2(qid)	  (0x0a4 + (qid * 0x148))
#define TQ_ADDR2(qid)	  (0x0a8 + (qid * 0x148))

#define TQ_BAR2(qid, bdx) (0x0ac + (qid * 0x148) + (bdx * 0x4))
#define TQ_NID2(qid)	  (0x12c + (qid * 0x148))
#define TQ_SIZE1(qid)	  (0x130 + (qid * 0x148))
#define TQ_ADDR1(qid)	  (0x134 + (qid * 0x148))

static inline u32 ane_engine_read32(struct ane_device *ane, u32 offset)
{
	return readl(ane->engine + offset);
}

static inline void ane_engine_write32(struct ane_device *ane, u32 offset,
				      u32 value)
{
	writel(value, ane->engine + offset);
}

#define ANE_TM_BASE	       0x20000 // 0x26bc24000
#define ANE_TQ_BASE	       0x21000 // 0x26bc25000

#define tm_read32(ane, offset) (ane_engine_read32(ane, ANE_TM_BASE + offset))
#define tq_read32(ane, offset) (ane_engine_read32(ane, ANE_TQ_BASE + offset))
#define tm_write32(ane, offset, val) \
	(ane_engine_write32(ane, ANE_TM_BASE + offset, val))
#define tq_write32(ane, offset, val) \
	(ane_engine_write32(ane, ANE_TQ_BASE + offset, val))

#define ANE_TM_IRQ_LINES 2
#define ANE_DEFAULT_QID	 4

#define TM_IS_IDLE	 0x1
#define ENG_IS_FINE	 0x22222222

void ane_tm_enable(struct ane_device *ane)
{
	tm_write32(ane, TM_TQ_EN, 0x2000);
	tm_write32(ane, TM_TQ_EN, 0x3000);

	for (int qid = 0; qid < ANE_TQ_COUNT; qid++) {
		tq_write32(ane, TQ_PRTY(qid), TQ_PRTY_TABLE[qid]);
	}

	tm_write32(ane, TM_IRQ_EN1, 0x4000000);
	tm_write32(ane, TM_IRQ_EN2, 0x6);
}

static int ane_tm_schedule_tq(struct ane_device *ane)
{
	return ANE_DEFAULT_QID;
}

static int ane_tm_arbiter_tq(struct ane_device *ane)
{
	return ANE_DEFAULT_QID;
}

int ane_tm_enqueue(struct ane_device *ane, struct ane_engine_req *req)
{
	int qid = ane_tm_schedule_tq(ane);
	req->qid = qid;

	/*
	 * Very occasionally coreml will output a model so jacked
	 * attempting to execute it causes an engine failure.
	 * Recovery requires TLB invalidation + power domain reset.
	 * As this sequence happens to be identical to runtime pm,
	 * 1) pausing for three seconds should do the job
	 * 2) I'm holding this off bc there's bigger fish to fry.
	 *
	 */

	if ((tm_read32(ane, TM_ENG_ERR1) != ENG_IS_FINE) ||
	    (tm_read32(ane, TM_ENG_ERR2) != ENG_IS_FINE)) {
		pr_err("engine at failure state\n");
		return -EINVAL;
	}

	if (tq_read32(ane, TQ_STATUS(qid))) {
		pr_err("tq %d is already occupied\n", qid);
		return -EBUSY;
	}

	if (tq_read32(ane, TQ_PRTY(qid)) != TQ_PRTY_TABLE[qid]) {
		pr_err("invalid priority setup for tq %d\n", qid);
		return -EINVAL;
	}

	tq_write32(ane, TQ_STATUS(qid), 0x1);

	for (int bdx = 0; bdx < ANE_BAR_SLOTS; bdx++) {
		tq_write32(ane, TQ_BAR1(qid, bdx), req->bar[bdx]);
	}

	tq_write32(ane, TQ_SIZE1(qid),
		   ((req->td_size << 0xe) + 0x1ff0000) & 0x1ff0000);
	tq_write32(ane, TQ_ADDR1(qid), req->fifo_addr & 0xffffffff);
	tq_write32(ane, TQ_NID1(qid), (req->nid & 0xff) << 8 | 1);

	/* clear other slot */
	tq_write32(ane, TQ_SIZE2(qid), 0x0);
	tq_write32(ane, TQ_ADDR2(qid), 0x0);
	tq_write32(ane, TQ_NID2(qid), 0x0);

	return 0;
}

static void ane_tm_push_tq(struct ane_device *ane, struct ane_engine_req *req)
{
	int qid = req->qid;
	tm_write32(ane, TM_ADDR, tq_read32(ane, TQ_ADDR1(qid)));
	tm_write32(ane, TM_INFO, tq_read32(ane, TQ_SIZE1(qid)) | req->td_count);
	/* magic word */
	tm_write32(ane, TM_PUSH, TQ_PRTY_TABLE[qid] | (qid & 7) << 8);
}

static int ane_tm_get_status(struct ane_device *ane)
{
	int err;
	u32 status;

	err = readl_poll_timeout(ane->engine + ANE_TM_BASE + TM_STATUS, status,
				 (status & TM_IS_IDLE), 1, 1000000);
	if (err)
		pr_err("tm execution failed w/ %d\n", err);

	return err;
}

static void ane_tm_handle_irq(struct ane_device *ane)
{
	for (int line = 0; line < ANE_TM_IRQ_LINES; line++) {
		u32 evtc = tm_read32(ane, TM_IRQ_EVTC(line));

		for (u32 evtn = 0; evtn < evtc; evtn++) {
			tm_read32(ane, TM_IRQ_INFO(line));
			tm_read32(ane, TM_IRQ_UNK1(line));
			tm_read32(ane, TM_IRQ_TMST(line));
			tm_read32(ane, TM_IRQ_UNK2(line));
		}

		if (!line) {
			tm_write32(ane, TM_IRQ_ACK,
				   tm_read32(ane, TM_IRQ_ACK) | 2);
		}
	}
}

int ane_tm_execute(struct ane_device *ane, struct ane_engine_req *req)
{
	int qid, err;

	qid = ane_tm_arbiter_tq(ane);
	if (qid != req->qid) {
		pr_err("mismatch between req->qid & arbitered qid\n");
		return -EINVAL;
	}

	ane_tm_push_tq(ane, req);

	err = ane_tm_get_status(ane);

	ane_tm_handle_irq(ane);

	tq_write32(ane, TQ_STATUS(req->qid), 0x0);

	return err;
}
