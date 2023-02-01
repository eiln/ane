
CFLAGS = -I. -Wall -Werror
LIBS = -I/usr/include/libdrm \
	-I/home/eileen/ane/ane/src/include \
	-I/home/eileen/ane/anec \
	-I/home/eileen/ane/anelib

BUILD_DIR = /home/eileen/ane/build
OBJECTS = $(BUILD_DIR)/ane.o $(BUILD_DIR)/ane_drv.o

.PHONY: anelib clean

anelib: $(OBJECTS)
	ar cr $(BUILD_DIR)/anelib.a $(OBJECTS)
	ld -r $(OBJECTS) -o $(BUILD_DIR)/anelib.o

$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(BUILD_DIR)
	gcc $(CFLAGS) $(LIBS) -c $< -o $@


clean:
	rm -rf $(BUILD_DIR)/*
	