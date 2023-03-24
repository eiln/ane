

# libane

- [C/C++](#cc)
- [Python](#python)
- [Jupyter Notebook](#jupyter-notebook)



## C/C++

### Installation

	git clone https://github.com/eiln/ane.git
	cd ane
	make
	make install

will install

	/usr/lib/libane.a
	/usr/include/libane/ane*.h


### Dependencies

`libane` requires `libdrm` to make ioctl calls, but that really should be there.
The static archive /usr/lib/libane.a is compiled at `-std=gnu99 -O2`.
The exposed header, /usr/include/libane/ane.h
requires only `<asm/types.h>` (for kernel __u types) and `<stdint.h>`.


### Conversion

	anecc --c /path/to/mymodel.hwx

Generates

	anec_$name.h, anec_$name.o, $name.anec.o


### Usage

The `ane.h` header defines:

	struct ane_nn *ane_init(const struct ane_model *model);
	void ane_free(struct ane_nn *nn);
	int ane_exec(struct ane_nn *nn);
	int ane_send(struct ane_nn *nn, void *from, const int idx);
	int ane_read(struct ane_nn *nn, void *to, const int idx);


**`ane_init()`: register model**

Originally the header would each define the `struct ane_model`
to call `ane_init()`, but struct initializers are a sin in cpp apparently.
Also people shouldn't be editing that.
So now the model struct gets wrapped into
`struct ane_nn *ane_init_$name(void)`,
declared in `anec_$name.h` & `anec_$name.o`.
If you're curious of the struct in question,
run the [super-secret backend](https://github.com/eiln/anecc/tree/main/anect/anect):
`anect --help`.


**`ane_free()`: undo ane_init()**

Yeah. Do that.


**`ane_exec()`: execute neural network**

**`ane_send()`: memcpy to the idx-th input channel**

To send inputs to the device. Tiles data beforehand.

**`ane_read()`: memcpy from the idx-th output channel**

To receive outputs from the device. Untiles the received data afterhand.


Example like [cblas](https://www.gnu.org/software/gsl/doc/html/cblas.html):


	#include <stdio.h>
	#include "ane_f16.h"
	#include "anec_matmul.h"

	int main(void) {
		int err = 0;

		float Af[] = { 0.11, 0.12, 0.13,
	                       0.21, 0.22, 0.23 };
		float_to_half_c_array(Af, A);

		float Bf[] = { 1011, 1012,
		               1021, 1022,
		               1031, 1032 };
		float_to_half_c_array(Bf, B);

		float Cf[] = { 0.00, 0.00,
	                       0.00, 0.00 };
		init_half_array(C, sizeof(Cf) / sizeof(float));

		struct ane_nn *nn = ane_init_matmul();
		if (nn == NULL) {
			printf("failed to register model\n");
			return -1;
		}

		ane_send(nn, A, 0); // send inputs
		ane_send(nn, B, 1);
		err = ane_exec(nn); // execute at state
		ane_read(nn, C, 0); // read outputs

		half_to_float_c_array(C, Cff);
		printf("[ %g, %g\n", Cff[0], Cff[1]);
		printf("  %g, %g ]\n", Cff[2], Cff[3]);

		ane_free(nn);

		return err;
	}


Compile with `gcc` or `g++`:

	gcc -I/usr/include/accel?/idk -I/usr/include/libane
		matmul.anec.o anec_matmul.o \
		main.c -o main.out \
		/usr/lib/libane.a

Or link `-lane` in place of `/usr/lib/libane.a`.



## Python

### Installation

The [single-file backend](https://github.com/eiln/ane/blob/main/python/ane/__init__.py)
can be installed with:

	pip install ane

Or

	git clone https://github.com/eiln/ane.git
	cd ane
	make python


### Dependencies

`libane` on the C-side (duh).

More notably Numpy because it operates on numpy arrays;
shouldn't matter but mine is pip-installed and at 1.24.1.


### Conversion

	anecc --python /path/to/mymodel.hwx

Generates

	$name.anec.so


### Usage

The entrypoint is `class Model`;
pass the path to the dylib anecc compiled:

	import ane
	model = ane.Model("mymodel.anec.so")

All inputs must be numpy arrays, i.e.

	import numpy as np
	A = np.random.random((1, 2, 3, 4)).astype(np.float16)

The model is bound to a single method, which is

	pred = model.predict([A])

Input is a **list of numpy arrays** of length input_count.
Output is a **list of numpy arrays** of length output_count.
**Order matters**.
This isn't CoreML with dict keys, so 
ensure inputs (and each shape) correspond to what anecc says.
**Must be a list** even if it's single input/output.
Meaning, for a single-input/single-output model (as in most cases),
bracket the `[input]` and extract the output with `pred[0]`.
These are piped directly into C driver code;
I'm not fucking around with types.



## Jupyter Notebook

Same as normal Python
but you NEED to **RESTART THE KERNEL** (circle arrow button on Firefox)
**when finished** to trigger atexit or you WILL **RUN OUT OF RESOURCES**.

