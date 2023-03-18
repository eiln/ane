
.PHONY: anelib clean

default: anelib

anelib:
	make -C anelib
install:
	make -C anelib install

clean:
	rm -rf build
