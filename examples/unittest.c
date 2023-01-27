#include <stdio.h>

#include "ane.h"
#include "anec.h"

static int unary_run(const struct ane_model *model)
{
	int err = 0;

	struct ane_nn *nn = ane_register(model);
	if (nn == NULL) {
		fprintf(stderr, "ane_register failed\n");
		return -EINVAL;
	}

	half *input_data = ane_zmemalign(input_size(nn, 0));
	if (input_data == NULL) {
		ane_destroy(nn);
		return -ENOMEM;
	}
	half *output_data = ane_zmemalign(output_size(nn, 0));
	if (output_data == NULL) {
		free(input_data);
		ane_destroy(nn);
		return -ENOMEM;
	}

	uint32_t elements = (input_size(nn, 0) / sizeof(half));
	for (uint32_t i = 0; i < elements; i++) {
		float x = (float)rand() / (float)(RAND_MAX / 1.0);
		input_data[i] = float_to_half(x);
	}

	ane_write(nn, input_data, 0);

	err = ane_exec(nn);
	if (err) {
		fprintf(stderr, "execution failed with 0x%x\n", err);
	}

	ane_read(nn, output_data, 0);

	free(input_data);
	free(output_data);

	ane_destroy(nn);
	
	return err;
}

int main(void)
{
	unary_run(&ane_abs);
	unary_run(&ane_atan);
	unary_run(&ane_ceil);
	unary_run(&ane_cube);
	unary_run(&ane_exp);
	unary_run(&ane_exp2);
	unary_run(&ane_frac);
	unary_run(&ane_inv);
	unary_run(&ane_log);
	unary_run(&ane_log10);
	unary_run(&ane_log2);
	unary_run(&ane_norm);
	unary_run(&ane_rsqrt);
	unary_run(&ane_sigmoid);
	unary_run(&ane_sign);
	unary_run(&ane_sqrt);
	unary_run(&ane_square);
	unary_run(&ane_tanh);

	return 0;
}
