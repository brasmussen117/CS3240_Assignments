testfile = s.csv

file = parser

make:
	cc -std=gnu11 -Wall -Werror -o parser parser.c

clean:
	rm -f ./parser
	rm -f ./search
	rm -f ./bin/cards
	rm -f ./bin/index

debug: $(file).c
	cc -std=gnu11 -Wall -Werror -o $(file) $(file).c -g -no-pie

gdb: $(file)
	gdb $(file)

valgrind:
	valgrind --tool=memcheck --leak-check=full ./$(file) $(testfile)

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./$(file) $(testfile)
