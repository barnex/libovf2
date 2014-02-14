all:
	gcc -c -std=c99 -Wall -Werror libovf2.c
	gcc -std=c99 -Wall -Werror *.c -o ovf2tool

test: all
	./ovf2tool testdata/*.ovf
