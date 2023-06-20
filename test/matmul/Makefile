all:
	anecc matmul.hwx -w
	gcc -I. -I/usr/include/libane main.c -o main.out -lane

clean:
	rm -f *.out *.anec
