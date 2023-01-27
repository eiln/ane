.PHONY: all anelib anepy clean 

all: anelib anepy 

anelib:
	make -C anelib
anepy:
	make -C anepy


clean:
	rm -rf build
