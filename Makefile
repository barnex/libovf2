libovf2:
	gcc -c -std=c89 -Wall -Wextra -Werror libovf2.c
	gcc -c -std=c99 -Wall -Wextra -Werror libovf2.c
	g++ -c          -Wall -Wextra -Werror libovf2.c
	gcc    -std=c99 -Wall -Wextra -Werror libovf2.c test.c     -o test
	gcc    -std=c99 -Wall -Wextra -Werror libovf2.c ovf2tool.c -o ovf2tool

test: libovf2
	./test
