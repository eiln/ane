.PHONY: libane python clean

default: libane
libane:
	make -C libane
python: libane
	make -C python install
install:
	make -C libane install

clean:
	rm -rf build
