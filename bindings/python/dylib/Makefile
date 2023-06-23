
CC = gcc
CFLAGS = -I. -I./include -Wall -Werror -Wextra \
	-Wdeclaration-after-statement \
	-O3 -std=gnu99

LIBS = -I/usr/include/libane

.PHONY: libane_python install uninstall clean

libane_python:
	gcc $(CFLAGS) $(LIBS)\
		-shared -pthread -fPIC -fno-strict-aliasing \
		libane_python.c -o libane_python.so \
		-lane

install: libane_python
	install libane_python.so ${DESTDIR}/usr/lib

uninstall:
	rm -f ${DESTDIR}/usr/lib/libane_python.so

clean:
	rm -f *.so

