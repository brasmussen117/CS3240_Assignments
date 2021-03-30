testfile = s.csv

file = search

make:
	cc -std=gnu11 -Wall -Werror -o parser parser.c
	cc -std=gnu11 -Wall -Werror -o search search.c

cleanx:
	rm -f ./parser
	rm -f ./search

cleanb:
	rm -f ./bin/cards.bin
	rm -f ./bin/index.bin

debug: $(file).c
	cc -std=gnu11 -Wall -Werror -o $(file) $(file).c -g -no-pie

gdb: $(file)
	gdb $(file)

valgrind:
	valgrind --tool=memcheck --leak-check=full ./$(file) $(testfile)

valgrind_gdb:
	valgrind --tool=memcheck --leak-check=full --vgdb-error=0 ./$(file) $(testfile)
