testfile = s.csv

file = parser

make:
	cc -std=gnu11 -Wall -Werror -o parser parser.c

clean:
	rm -f ./parser
	rm -f ./s_cardbin
	rm -f ./s_indexbin

debug: $(file).c
	cc -std=gnu11 -Wall -Werror -o $(file) $(file).c -g -no-pie

gdb: $(file)
	gdb $(file)

valgrind:
	valgrind --tool=memcheck --leak-check=full ./$(file) $(testfile)

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./$(file) $(testfile)
