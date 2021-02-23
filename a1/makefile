clean: parser
	rm parser
	rm parser.o

debug: main.c
	cc -std=gnu11 -Wall -Werror -o parser main.c -g

# parser: parser.o
# 	cc -std=gnu11 -Wall -Werror -o parser main.c

parser.o: main.c
	cc -c -std=gnu11 -Wall -Werror -o parser.o main.c

valgrind:
	valgrind --tool=memcheck --leak-check=full ./parser
