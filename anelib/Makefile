
CC = gcc
CFLAGS = -I. -Wall -Werror
LIBS = -I/usr/include/libdrm \
	-I/home/eileen/ane/ane/src/include

BUILD_DIR = /home/eileen/ane/build
OBJECTS = $(BUILD_DIR)/ane.o $(BUILD_DIR)/ane_drv.o

.PHONY: anelib clean

anelib: $(OBJECTS)
	ar cr $(BUILD_DIR)/anelib.a $(OBJECTS)
	ld -r $(OBJECTS) -o $(BUILD_DIR)/anelib.o

$(BUILD_DIR)/%.o : %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
