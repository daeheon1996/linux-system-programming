myshell: main.o parser.o executor.o builtin.o
	gcc main.o parser.o executor.o builtin.o -o miniShell

main.o: main.c
	gcc -Wall -Wextra -c main.c

parser.o: parser.c parser.h
	gcc -Wall -Wextra -c parser.c 

executor.o: executor.c executor.h
	gcc -Wall -Wextra -c executor.c

builtin.o: builtin.c builtin.h
	gcc -Wall -Wextra -c builtin.c

clean:
	rm -f main.o parser.o executor.o builtin.o miniSell