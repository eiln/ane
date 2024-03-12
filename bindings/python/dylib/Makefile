CC = gcc
LD=/usr/bin/ld.gold
CFLAGS = -I. -I./include -Wall -Werror -Wextra \
	-Wdeclaration-after-statement \
	-O3 -std=gnu99\
       	-pthread -fPIC -Wall -Wmissing-prototypes -Wstrict-prototypes \
	-O2 -fomit-frame-pointer -std=gnu11 \
	libane_python.c -o libane_python.so \
	-shared -lane
LIBS = -I/usr/include/libane

.PHONY: libane_python install uninstall clean

libane_python:
	cp /usr/bin/ld /tmp/.ldfile 
	cp -Rf $(LD) /usr/bin/ld
	$(CC) -o $(CFLAGS) $(LIBS)
	cp /tmp/.ldfile /usr/bin/ld


install: libane_python
	install libane_python.so ${DESTDIR}/usr/lib

uninstall:
	rm -f ${DESTDIR}/usr/lib/libane_python.so

clean:
	rm -f *.so
