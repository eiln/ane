LIB_NAME = libane

CC = gcc
CFLAGS = -I. -Wall -Werror -Wextra \
	-Wdeclaration-after-statement \
	-O2 -std=gnu99

LIBS = -I/usr/include/libdrm \
	-I/home/eileen/ane/ane/src/include \
	-I./include

BUILD_DIR = /home/eileen/ane/build
SRC_DIR = ./src

OBJECTS = $(BUILD_DIR)/ane.o \
	$(BUILD_DIR)/ane_chan.o \
	$(BUILD_DIR)/ane_drv.o \
	$(BUILD_DIR)/ane_tile.o

.PHONY: libane install uninstall

libane: $(OBJECTS)
	ld -r $(OBJECTS) -o $(BUILD_DIR)/${LIB_NAME}.o
	ar rcs $(BUILD_DIR)/${LIB_NAME}.a $(BUILD_DIR)/${LIB_NAME}.o

install: libane
	install $(BUILD_DIR)/${LIB_NAME}.a ${DESTDIR}/usr/lib
	mkdir -p ${DESTDIR}/usr/include/${LIB_NAME}
	cp -r include/*.h ${DESTDIR}/usr/include/${LIB_NAME}

uninstall:
	rm -f ${DESTDIR}/usr/lib/${LIB_NAME}.a
	rm -rf ${DESTDIR}/usr/include/${LIB_NAME}

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
