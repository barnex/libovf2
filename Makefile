all:
	gcc -c -O0 -g -std=c99 -Wall -Werror libovf2.c
	gcc -O0 -g -std=c99 -Wall -Werror *.c -o ovf2tool

test: all
	./ovf2tool testdata_good/*.ovf
