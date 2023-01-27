
CFLAGS = -I. -Wall -Werror
LIBS = -I/usr/include/libdrm \
	-I/home/eileen/ane/ane/src/include \
	-I/home/eileen/anec \
	-I/home/eileen/ane/anelib

BUILD_DIR = /home/eileen/ane/build

.PHONY: clean anepy 

anepy:
	make -C /home/eileen/ane/anelib
	gcc -shared -pthread -fPIC -fno-strict-aliasing \
		$(CFLAGS) $(LIBS) -I/usr/include/python3.10 \
		$(BUILD_DIR)/anelib.o anepy.c -o $(BUILD_DIR)/anepy.so

clean:
	rm -f *.o *.out *.so *.bin
