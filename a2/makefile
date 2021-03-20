make:
	cc -std=gnu11 -Wall -Werror -o parser parser.c

clean:
ifeq (,$(wildcard ./parser))
	rm parser
endif

debug: parser.c
	cc -std=gnu11 -Wall -Werror -o parser parser.c -g

gdb: parser
	gdb parser

valgrind:
	valgrind --tool=memcheck --leak-check=full ./parser s.csv

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./parser s.csv
