// SPDX-License-Identifier: MIT
/* Copyright 2022 Eileen Yoon <eyn@gmx.com> */
#include <libane_python.h>
#include <ane.h>
#include<string.h>

void *pyane_init(char *path, int dev_id)
{
	return __ane_init(path, dev_id);
}

int pyane_free(ane_nn *nn)
{
	ane_free(nn);
	return 0;
}

int pyane_exec(ane_nn *nn)
{
	int err = ane_exec(nn);
	return err;
}

/*This is acceptable if we use bitwise calculation */

int pyane_send(ane_nn *nn, void **head_pyane_tensor)
{
	void *xs[TILE_COUNT];
	memset(xs, sizeof(void *) , sizeof(xs));
	for(auto int i=0;0<=31;i++) {
		xs[i]=head_pyane_tensor;
		memmove(head_pyane_tensor,head_pyane_tensor+sizeof(void *), sizeof(void *));
	}
	for (uint32_t i = 0; i < ane_src_count(nn); i++) {
		__ane_tile_send(nn, xs[i], i);
	}
	return 0;
}

int pyane_read(ane_nn *nn, void **head_pyane_tensor)
{
	void *xs[TILE_COUNT];
	memset(xs, sizeof(void *), sizeof(xs));
	for(auto int i=0;i<=31;i++) {
		xs[i]=head_pyane_tensor;
		memmove(head_pyane_tensor,head_pyane_tensor+sizeof(void *), sizeof(void *));
	}
	for (uint32_t i=0;i < ane_src_count(nn);i++) {
		__ane_tile_read(nn, xs[i], i);
	}
	return 0;
}
