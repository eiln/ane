
.PHONY: libane clean

default: libane
libane:
	make -C libane
install:
	make -C libane install

clean:
	rm -rf build
