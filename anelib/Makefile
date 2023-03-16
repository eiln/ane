
CC = gcc
CFLAGS = -I. -Wall -Werror -Wextra -Wpedantic

LIBS = -I/usr/include/libdrm \
	-I/home/eileen/ane/ane/src/include \
	-I/home/eileen/ane/anelib/include

BUILD_DIR = /home/eileen/ane/build
SRC_DIR = /home/eileen/ane/anelib/src

OBJECTS = $(BUILD_DIR)/ane.o \
	$(BUILD_DIR)/ane_chan.o \
	$(BUILD_DIR)/ane_drv.o

.PHONY: anelib

anelib: $(OBJECTS)
	ar cr $(BUILD_DIR)/anelib.a $(OBJECTS)
	ld -r $(OBJECTS) -o $(BUILD_DIR)/anelib.o

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
