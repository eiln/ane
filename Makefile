
.PHONY: libane clean

default: libane
libane:
	make -C libane
install:
	make -C libane install
	make -C python install

clean:
	rm -rf build
