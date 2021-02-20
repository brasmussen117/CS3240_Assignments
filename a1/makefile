parser: parser.o
	cc -std=gnu11 -Wall -Werror -o parser main.c

parser.o: main.c
	cc -c -std=gnu11 -Wall -Werror -o parser main.c

clean: parser
	rm parser

valgrind:
	valgrind --tool=memcheck --leak-check=full ./parser
