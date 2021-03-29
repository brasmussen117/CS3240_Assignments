make:
	cc -std=gnu11 -Wall -Werror -o parser parser.c

clean:
	rm -f ./parser
	rm -f ./s_cardbin
	rm -f ./s_indexbin

debug: parser.c
	cc -std=gnu11 -Wall -Werror -o parser parser.c -g -no-pie

gdb: parser
	gdb parser

valgrind:
	valgrind --tool=memcheck --leak-check=full ./parser s.csv

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./parser s.csv
