libovf2:
	gcc -c -std=c89 -Wall -Wextra -Werror libovf2.c
	gcc -c -std=c99 -Wall -Wextra -Werror libovf2.c
	g++ -c          -Wall -Wextra -Werror libovf2.c

ovf2tool: libovf2
	gcc -std=c99 *.c -o ovf2tool

test: 
	./ovf2tool testdata_good/*.ovf
