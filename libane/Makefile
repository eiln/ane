LIB_NAME = libane

CC = gcc
CFLAGS = -I. -Wall -Werror -Wextra -Wpedantic -O2

LIBS = -I/usr/include/libdrm \
	-I/home/eileen/ane/ane/src/include \
	-I./include

BUILD_DIR = /home/eileen/ane/build
SRC_DIR = ./src

OBJECTS = $(BUILD_DIR)/ane.o \
	$(BUILD_DIR)/ane_chan.o \
	$(BUILD_DIR)/ane_drv.o

.PHONY: libane install

libane: $(OBJECTS)
	ar cr $(BUILD_DIR)/${LIB_NAME}.a $(OBJECTS)
	ld -r $(OBJECTS) -o $(BUILD_DIR)/${LIB_NAME}.o

install: libane
	install $(BUILD_DIR)/${LIB_NAME}.a ${DESTDIR}/usr/lib
	install $(BUILD_DIR)/${LIB_NAME}.o ${DESTDIR}/usr/lib
	mkdir -p ${DESTDIR}/usr/include/${LIB_NAME}
	cp -r include/*.h ${DESTDIR}/usr/include/${LIB_NAME}

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
