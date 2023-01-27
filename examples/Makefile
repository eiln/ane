
CFLAGS = -I. -Wall -Werror
LIBS = -I/usr/include/libdrm \
	-I/home/eileen/ane/ane/src/include \
	-I/home/eileen/anec \
	-I/home/eileen/ane/anelib

BUILD_DIR = /home/eileen/ane/build

.PHONY: hello_ane clean


hello_ane:
	make -C /home/eileen/ane/anelib
	gcc $(CFLAGS) $(LIBS) -c hello_ane.c -o hello_ane.o
	gcc $(CFLAGS) hello_ane.o $(BUILD_DIR)/anelib.a -o hello_ane.out


clean:
	rm -f *.o *.out *.bin
