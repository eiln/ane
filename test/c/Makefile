
all: matmul rsqrt

matmul:
	anecc matmul.hwx
	gcc -I. -I/usr/include/libane matmul.c -o matmul.out /usr/lib/libane.a

rsqrt:
	anecc rsqrt.hwx
	gcc -I. -I/usr/include/libane rsqrt.c -o rsqrt.out -lane

clean:
	rm -f *.o *.out

