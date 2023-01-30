
.PHONY: all anelib clean

default: anelib
all: default

anelib:
	make -C anelib

clean:
	rm -rf build
