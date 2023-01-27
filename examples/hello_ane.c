#include <stdio.h>

#include "ane.h" /* userspace lib */
#include "anec.h" /* compute kernels */

int main(void)
{
	int err = 0;

	/* initiate a context */
	struct ane_nn *nn = ane_register(&ane_sqrt);
	if (nn == NULL) {
		fprintf(stderr, "ane_register failed\n");
		return -EINVAL;
	}

	/* single input, single output element-wise unary pass */
	printf("input_count: %d output_count: %d\n", input_count(nn),
	       output_count(nn));

	/* (2560 * 1600) * (sizeof(half)) = 0x7d0000 */
	printf("input_size: 0x%x output_size: 0x%x\n", input_size(nn, 0),
	       output_size(nn, 0));

	/* kernels were compiled for fp16 */
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

	/* generate some data */
	uint32_t elements = (input_size(nn, 0) / sizeof(half));
	for (uint32_t i = 0; i < elements; i++) {
		float x = (float)rand() / (float)(RAND_MAX / 1.0);
		input_data[i] = float_to_half(x);
	}

	ane_fwrite(input_data, input_size(nn, 0), "input.bin");

	/* set input */
	ane_write(nn, input_data, 0);

	/* trigger the hardware! */
	err = ane_exec(nn);
	if (err) {
		fprintf(stderr, "execution failed with 0x%x\n", err);
	}

	/* get output from hardware */
	ane_read(nn, output_data, 0);
	
	ane_fwrite(output_data, output_size(nn, 0), "output.bin");

	free(input_data);
	free(output_data);

	ane_destroy(nn); /* destroy context */

	return err;
}
